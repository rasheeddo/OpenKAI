#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_followClient_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_followClient_H_

#include "../../../Base/common.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_link.h"

namespace kai
{

#define APFOLLOW_OFF 0
#define APFOLLOW_ON 1

class _APcopter_followClient: public _AutopilotBase
{
public:
	_APcopter_followClient();
	~_APcopter_followClient();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);

	void onKey(int key);
	static void callbackKey(int key, void* pInst)
	{
		NULL_(pInst);
		_APcopter_followClient* pS = (_APcopter_followClient*)pInst;
		pS->onKey(key);
	}

private:
	void updateBB(void);
	void updateAlt(void);
	void updateHdg(void);
	void updateState(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_followClient *) This)->update();
		return NULL;
	}

public:
	_APcopter_link* m_pAL;
	float	m_diff;
	uint8_t	m_iSetState;

	float	m_bbSize;
	float	m_dBBsize;
	vFloat2 m_vBBsize;
	vFloat4 m_vBB;

	float	m_alt;
	float	m_dAlt;
	float	m_hdg;
	float	m_dHdg;

	INTERVAL_EVENT m_ieSend;
};

}
#endif
