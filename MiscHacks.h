#pragma once

#include "Hacks.h"

Vector GetAutostrafeView();

class CMiscHacks : public CHack
{
public:
	void Init();
	void Draw();
	void zeusbot(CUserCmd * m_pcmd);
	void namespam();
	void buybot_primary();
	void buybot_secondary();
	void buybot_otr();

	void preset_cfg();
	
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	int GetFPS();
	void FakeWalk0(CUserCmd * pCmd, bool & bSendPacket);
	float get_gun(C_BaseCombatWeapon* weapon);
	bool _test = false;
	bool oof = false;
	bool _done = false;

	bool do_zeus;

	void MinimalWalk(CUserCmd * pCommand, float fMaxSpeed);
	void optimize();

	void LoadNamedSky(const char * sky_name);
	void buy_logs(IGameEvent * Event);
	void colour_modulation();
	void fake_crouch(CUserCmd * cmd, bool & packet, IClientEntity * local);
	bool anotherpcheck;
	void AutoJump(CUserCmd *pCmd);
private:
	
	void AutoPistol(CUserCmd * pCmd);
	void strafe_2(CUserCmd * cmd);
	void PostProcces();

	void strafer(CUserCmd * cmd);

	void viewmodel_x_y_z();

	void RankReveal(CUserCmd * cmd);

	void airduck(CUserCmd * pCmd);
	void RageStrafe(CUserCmd * userCMD, IClientEntity * g_LocalPlayer);

	bool paste = false;
	int CircleFactor = 0;
	static vec_t normalize_yaw(vec_t ang)
	{
		while (ang < -180.0f)
			ang += 360.0f;
		while (ang > 180.0f)
			ang -= 360.0f;
		return ang;
	}
	
}; extern CMiscHacks* c_misc;
