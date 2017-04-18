#include "../APcopter/APcopter_guided.h"

namespace kai
{

APcopter_guided::APcopter_guided()
{
	m_pSF40 = NULL;
	m_pAP = NULL;
	m_lastFlightMode = 0;
}

APcopter_guided::~APcopter_guided()
{
}

bool APcopter_guided::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_guided::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	//Add GPS

	return true;
}

void APcopter_guided::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	IF_(!isActive());

	updateAttitude();

}

void APcopter_guided::updateAttitude(void)
{
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pSF40);

	uint32_t fMode = m_pAP->m_flightMode;
	if(fMode != m_lastFlightMode)
	{
		//set new holding position in mode change (Any -> Guided_NoGPS)
		m_lastFlightMode = fMode;
	}


//	APMcopter_PID* pPID;
//	APMcopter_CTRL* pCtrl;
//	double v;
//
//	//roll
//	pPID = &m_pAPM->m_pidRoll;
//	pCtrl = &m_pAPM->m_ctrlRoll;
//	v = m_pSF40->m_pX->v();
//
//	pCtrl->m_errOld = pCtrl->m_err;
//	pCtrl->m_predPos = v + (v-pCtrl->m_pos)*pPID->m_dT;
//	pCtrl->m_pos = v;
//	pCtrl->m_err = pCtrl->m_targetPos - pCtrl->m_predPos;
//	pCtrl->m_errInteg += pCtrl->m_err;
//	pCtrl->m_v = pPID->m_P * pCtrl->m_err
//						+ pPID->m_D * (pCtrl->m_err - pCtrl->m_errOld)
//						+ constrain(pPID->m_I * pCtrl->m_errInteg, pPID->m_Imax, -pPID->m_Imax);
//
//	//pitch
//	pPID = &m_pAPM->m_pidPitch;
//	pCtrl = &m_pAPM->m_ctrlPitch;
//	v = m_pSF40->m_pY->v();
//
//	pCtrl->m_errOld = pCtrl->m_err;
//	pCtrl->m_predPos = v + (v-pCtrl->m_pos)*pPID->m_dT;
//	pCtrl->m_pos = v;
//	pCtrl->m_err = pCtrl->m_targetPos - pCtrl->m_predPos;
//	pCtrl->m_errInteg += pCtrl->m_err;
//	pCtrl->m_v = pPID->m_P * pCtrl->m_err
//						+ pPID->m_D * (pCtrl->m_err - pCtrl->m_errOld)
//						+ constrain(pPID->m_I * pCtrl->m_errInteg, pPID->m_Imax, -pPID->m_Imax);

	//TODO:
	//yaw
	//throttle

	//Send Mavlink command
//	m_pAPM->updateAttitude();

}

bool APcopter_guided::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	if(m_pAP)
	{
		string msg = *this->getName()+": Att. Target: Roll="
				+ f2str(m_pAP->m_ctrlRoll.m_v)
				+", Pitch="
				+ f2str(m_pAP->m_ctrlPitch.m_v);
		pWin->addMsg(&msg);
	}

	return true;
}


}