#include "APcopter_avoid.h"

namespace kai
{

APcopter_avoid::APcopter_avoid()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;

	m_iClass = -1;
	m_bFound = false;
	m_apMode = -1;
	m_tO.init();

	m_vMyPos.init();
	m_vMyPos.x = 0.5;
	m_vMyPos.y = 0.5;
	m_vTargetPos.init();
	m_apMount.init();

	m_iTracker = 0;
	m_bUseTracker = false;
	m_pTracker[0] = NULL;
	m_pTracker[1] = NULL;
	m_pTnow = NULL;
	m_pTnew = NULL;

}

APcopter_avoid::~APcopter_avoid()
{
}

bool APcopter_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iClass);
	KISSm(pK,bUseTracker);
	KISSm(pK,apMode);

	Kiss* pG = pK->o("mount");
	if(!pG->empty())
	{
		double p=0, r=0, y=0;
		pG->v("pitch", &p);
		pG->v("roll", &r);
		pG->v("yaw", &y);

		m_apMount.m_control.input_a = p * 100;	//pitch
		m_apMount.m_control.input_b = r * 100;	//roll
		m_apMount.m_control.input_c = y * 100;	//yaw
		m_apMount.m_control.save_position = 0;

		pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
		pG->v("stabRoll", &m_apMount.m_config.stab_roll);
		pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
		pG->v("mountMode", &m_apMount.m_config.mount_mode);
	}

	pG = pK->o("targetPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vTargetPos.x);
		pG->v("y", &m_vTargetPos.y);
		pG->v("z", &m_vTargetPos.z);
		pG->v("w", &m_vTargetPos.w);
	}

	pG = pK->o("myPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vMyPos.x);
		pG->v("y", &m_vMyPos.y);
		pG->v("z", &m_vMyPos.z);
		pG->v("w", &m_vMyPos.w);
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrl", &iName);
	m_pPC = (APcopter_posCtrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	if(m_bUseTracker)
	{
		iName = "";
		pK->v("_TrackerBase1", &iName);
		m_pTracker[0] = (_TrackerBase*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pTracker[0], iName + ": not found");

		iName = "";
		pK->v("_TrackerBase2", &iName);
		m_pTracker[1] = (_TrackerBase*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pTracker[1], iName + ": not found");

		m_pTnow = m_pTracker[m_iTracker];
		m_pTnew = m_pTracker[1-m_iTracker];
	}

	return true;
}

int APcopter_avoid::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pPC,-1);
	NULL__(m_pDet,-1);
	_VisionBase* pV = m_pDet->m_pVision;
	NULL__(pV,-1);

	if(m_bUseTracker)
	{
		NULL__(m_pTracker[0],-1);
		NULL__(m_pTracker[1],-1);
	}

	return this->ActionBase::check();
}

void APcopter_avoid::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		m_vTargetPos = m_vMyPos;
		m_pPC->setPos(m_vMyPos, m_vMyPos);
		m_pDet->goSleep();
		if(m_bUseTracker)
		{
			m_pTracker[0]->stopTrack();
			m_pTracker[1]->stopTrack();
		}

		return;
	}

	if(m_bMissionChanged)
	{
		m_pDet->wakeUp();
	}

	m_pAP->setMount(m_apMount);

	if(!find())
	{
		m_bFound = false;
		m_vTargetPos = m_vMyPos;
		m_pPC->setPos(m_vMyPos, m_vMyPos);

		if(m_pAP->getApMode() == GUIDED && m_apMode>=0)
			m_pAP->setApMode(m_apMode);

		return;
	}

	//TODO: change to map based avoidance

	if(m_vTargetPos.x < 0.5)
		m_vTargetPos.x = 1.0;
	else
		m_vTargetPos.x = 0.0;
	m_vTargetPos.y = m_vMyPos.y;

	m_bFound = true;
	m_pPC->setPos(m_vMyPos, m_vTargetPos);

	//TODO: transmitter mode switch is disrupted by this line
	if(m_pAP->getApMode() == m_apMode)
		m_pAP->setApMode(GUIDED);
}

bool APcopter_avoid::find(void)
{
	IF__(check()<0, false);

	OBJECT* pO;
	OBJECT* tO = NULL;
	double topProb = 0.0;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_iClass);
		IF_CONT(pO->m_topProb < topProb);

		tO = pO;
		topProb = pO->m_topProb;
	}

	vFloat4 bb;
	if(m_bUseTracker)
	{
		if(tO)
		{
			if(m_pTnew->trackState() == track_stop)
			{
//				m_pTnew->startTrack(tO->m_bb);
			}
		}

		if(m_pTnew->trackState() == track_update)
		{
			m_iTracker = 1 - m_iTracker;
			m_pTnow = m_pTracker[m_iTracker];
			m_pTnew = m_pTracker[1-m_iTracker];
			m_pTnew->stopTrack();
		}

		if(m_pTnow->trackState() != track_update)
		{
			m_pPC->setON(false);
			m_pPC->releaseCtrl();
			return false;
		}

//		bb = *m_pTnow->getBB();
	}
	else
	{
		if(!tO)
		{
			m_pPC->setON(false);
			m_pPC->releaseCtrl();
			return false;
		}

		bb = tO->m_o.m_bb2.m_bb;
	}

	m_vTargetPos.x = bb.midX();
	m_vTargetPos.y = bb.midY();
	m_pPC->setON(true);

	return true;
}

bool APcopter_avoid::bFound(void)
{
	return m_bFound;
}

bool APcopter_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->tabNext();

	if(!bActive())
		pWin->addMsg("Inactive");

	pWin->addMsg("Avoid Target = (" + f2str(m_vTargetPos.x) + ", "
							   + f2str(m_vTargetPos.y) + ", "
					           + f2str(m_vTargetPos.z) + ", "
				           	   + f2str(m_vTargetPos.w) + ")");

	pWin->tabPrev();

	return true;
}

bool APcopter_avoid::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}

	C_MSG("Avoid Target = (" + f2str(m_vTargetPos.x) + ", "
				     	 + f2str(m_vTargetPos.y) + ", "
						 + f2str(m_vTargetPos.z) + ", "
						 + f2str(m_vTargetPos.w) + ")");

	return true;
}

}