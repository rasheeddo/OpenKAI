/*
 * MissionBase.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "MissionBase.h"

namespace kai
{

MissionBase::MissionBase()
{
	m_nextMission = "";
	m_tStart = 0;
	m_tStamp = 0;
	m_tTimeout = 0;
	m_type = mission_base;
}

MissionBase::~MissionBase()
{
}

bool MissionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v<string>("nextMission", &m_nextMission);

	if(pK->v("tTimeout",&m_tTimeout))
	{
		m_tTimeout *= USEC_1SEC;
	}

	return true;
}

bool MissionBase::update(void)
{
	m_tStamp = getTimeUsec();
	if(m_tStart <= 0)
		m_tStart = m_tStamp;

	return false;
}

void MissionBase::reset(void)
{
	m_tStart = 0;
}

MISSION_TYPE MissionBase::type(void)
{
	return m_type;
}

void MissionBase::draw(void)
{
	this->BASE::draw();
}

}
