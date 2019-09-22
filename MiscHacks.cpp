#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "IClientMode.h"
#include <chrono>
#include <algorithm>
#include <time.h>
#include "Hooks.h"
#include "position_adjust.h"
#include "RageBot.h"
#include "Autowall.h"
#include "Resolver.h"
CMiscHacks* c_misc = new CMiscHacks;
template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}
inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}
Vector AutoStrafeView;
void CMiscHacks::Init()
{
}
void CMiscHacks::Draw()
{
	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

}

void set_name(const char* name)
{
	ConVar* nameConvar = interfaces::cvar->FindVar(("name"));
	*(int*)((DWORD)&nameConvar->fnChangeCallback + 0xC) = NULL;
	nameConvar->SetValueChar(name);
}

inline float FastSqrt(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}
#define square( x ) ( x * x )

void angleVectors(const Vector& angles, Vector& forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void CMiscHacks::zeusbot(CUserCmd *m_pcmd)
{
	auto m_local = hackManager.pLocal();

	if (m_local->GetFlags() & FL_FROZEN)
		return;
	if (!m_local->IsAlive())
		return;
	if (!m_local->GetWeapon2())
		return;

	do_zeus = false;

	for (int y = 0; y <= 360; y += 360.f / 6.f) {
		for (int x = -89; x <= 89; x += 179.f / 6.f) {
			Vector ang = Vector(x, y, 0);
			Vector dir;
			angleVectors(ang, dir);
			trace_t trace;
			UTIL_TraceLine(m_local->GetEyePosition(), m_local->GetEyePosition() + (dir * 530), MASK_SHOT, m_local, 0, &trace);

			if (trace.m_pEnt == nullptr)
				continue;
			if (trace.m_pEnt == m_local)
				continue;
			if (!trace.m_pEnt->IsAlive())
				continue;
		
			if (trace.m_pEnt->team() == m_local->team())
				continue;
			if (trace.m_pEnt->dormant())
				continue;

			player_info_t info;
			if (!(interfaces::engine->GetPlayerInfo(trace.m_pEnt->GetIndex(), &info)))
				continue;

			do_zeus = true;

		//	m_pcmd->viewangles = Vector(x, y, 0);
		//	m_pcmd->buttons |= IN_ATTACK;

		//	do_zeus = true;

			do_zeus = false;

			return;
		}
	}
}

void CMiscHacks::namespam()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < .5)
		return;
	const char* result;
	std::vector <std::string> names;
	if (interfaces::engine->IsInGame() && interfaces::engine->IsConnected()) {
		for (int i = 1; i < interfaces::globals->max_clients; i++)
		{
			IClientEntity *entity = interfaces::ent_list->get_client_entity(i);

			player_info_t pInfo;

			if (entity && hackManager.pLocal()->team() == entity->team())
			{
				ClientClass* cClass = (ClientClass*)entity->GetClientClass();

				if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
				{
					if (interfaces::engine->GetPlayerInfo(i, &pInfo))
					{
						if (!strstr(pInfo.name, "GOTV"))
							names.push_back(pInfo.name);
					}
				}
			}
		}
	}

	set_name("\n\xAD\xAD\xAD");
	int randomIndex = rand() % names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", names[randomIndex].c_str());
	result = buffer;



	set_name(result);
	start_t = clock();

}
/*
template<class T>
static T* FindHudElement(const char* name)
{

static auto pThis = *reinterpret_cast<DWORD**>(Utilities::Memory::FindPatternV2("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 8D 58") + 1);

static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39"));

if (find_hud_element != nullptr)
{
return (T*)find_hud_element(pThis, name);
}

}
void preservefeed(IGameEvent* Event)
{
if (hackManager.pLocal()->IsAlive())
{
static DWORD* _death_notice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");

if (_death_notice == nullptr)
return;

static void(__thiscall *_clear_notices)(DWORD) = (void(__thiscall*)(DWORD))Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

if (round_change)
{

_death_notice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
if (_death_notice - 20)
_clear_notices(((DWORD)_death_notice - 20));
round_change = false;
}

if (_death_notice)
*(float*)((DWORD)_death_notice + 0x50) = Options::Menu.VisualsTab.killfeed.GetState() ? 100 : 1;
}
}*/
void CMiscHacks::buybot_primary()
{
	bool is_ct = hackManager.pLocal()->team() == TEAM_CS_CT;
	switch (options::menu.misc.buybot_primary.getindex())
	{
	case 1: is_ct ? (interfaces::engine->ExecuteClientCmd("buy scar20;")) : (interfaces::engine->ExecuteClientCmd("buy g3sg1;"));
		break;
	case 2: interfaces::engine->ExecuteClientCmd("buy ssg08;");
		break;
	case 3: interfaces::engine->ExecuteClientCmd("buy awp;");
		break;
	case 4: is_ct ? (interfaces::engine->ExecuteClientCmd("buy m4a1; buy m4a1_silencer")) : (interfaces::engine->ExecuteClientCmd("buy ak47;"));
		break;
	case 5: is_ct ? (interfaces::engine->ExecuteClientCmd("buy aug;")) : (interfaces::engine->ExecuteClientCmd("buy sg556"));
		break;
	case 6: is_ct ? (interfaces::engine->ExecuteClientCmd("buy mp9")) : (interfaces::engine->ExecuteClientCmd("buy mac-10;"));
		break;
	}

}

void CMiscHacks::buybot_secondary()
{
	switch (options::menu.misc.buybot_secondary.getindex())
	{
	case 1: interfaces::engine->ExecuteClientCmd("buy elite;");
		break;
	case 2: interfaces::engine->ExecuteClientCmd("buy deagle;");
		break;
	case 3: interfaces::engine->ExecuteClientCmd("buy fn57");
		break;
	}

}

void CMiscHacks::buybot_otr()
{
	std::vector<dropdownboxitem> otr_list = options::menu.misc.buybot_otr.items;

	if (otr_list[0].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy vest; buy vesthelm;");
	}

	if (otr_list[1].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy hegrenade;");
	}

	if (otr_list[2].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy flashbang;");
	}

	if (otr_list[3].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy smokegrenade;");
	}

	if (otr_list[4].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy molotov;");
	}

	if (otr_list[5].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy taser;");
	}

	if (otr_list[6].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy defuser;");
	}

}

void remove_fakelag_limit() { // call this only once!

	auto clMoveChokeClamp = game_utils::FindPattern1("engine.dll", "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC") + 1;

	unsigned long protect = 0;

	VirtualProtect((void*)clMoveChokeClamp, 4, PAGE_EXECUTE_READWRITE, &protect);
	*(std::uint32_t*)clMoveChokeClamp = 17;
	VirtualProtect((void*)clMoveChokeClamp, 4, protect, &protect);
}

void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	IGameEvent* Event;

	if (!hackManager.pLocal()->IsAlive())
	{
		_done = false;	
		return;
	}
	

	// ------- Oi thundercunt, this is needed for the weapon configs ------- //

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	PostProcces();
//	preset_cfg();
	if (pWeapon != nullptr)
	{
		if (GetAsyncKeyState(options::menu.misc.minimal_walk.GetKey()))
		{
			MinimalWalk(pCmd, get_gun(pWeapon));
		}

		if (options::menu.aimbot.AimbotEnable.getstate())
			zeusbot(pCmd);

		if (game_utils::AutoSniper(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_auto.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_auto.getvalue());

		}

		if (game_utils::IsPistol(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_pistol.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_pistol.getvalue());

		}

		if (pWeapon->is_scout())
		{

			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_scout.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_scout.getvalue());

		}

		if (pWeapon->is_awp())
		{
			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_awp.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_awp.getvalue());

		}

		if (game_utils::IsRifle(pWeapon) || game_utils::IsShotgun(pWeapon) || game_utils::IsMachinegun(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_otr.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_otr.getvalue());

		}

		if (game_utils::IsMP(pWeapon))
		{
			options::menu.aimbot.AccuracyHitchance.setvalue((float)options::menu.aimbot.hc_smg.getvalue());
			options::menu.aimbot.AccuracyMinimumDamage.setvalue((float)options::menu.aimbot.md_smg.getvalue());
		}

		if (game_utils::IsZeus(pWeapon))
		{
			options::menu.aimbot.AccuracyHitchance.setvalue(10);
			options::menu.aimbot.AccuracyMinimumDamage.setvalue(30);
		} 
	}

	if (options::menu.misc.infinite_duck.getstate())
	{
		pCmd->buttons |= IN_BULLRUSH;
	}

	if (options::menu.visuals.override_viewmodel.getstate())
		viewmodel_x_y_z();

	//	if (Options::Menu.RageBotTab.AimbotEnable.GetState())
	//		AutoPistol(pCmd);

//	RankReveal(pCmd);

	if (pLocal->movetype() == MOVETYPE_LADDER || pLocal->movetype() == MOVETYPE_NOCLIP)
		return;

//	if (options::menu.misc.OtherAutoJump.getstate())
//		AutoJump(pCmd);

	if (options::menu.misc.airduck_type.getindex() != 0)
	{
		airduck(pCmd);
	}
	interfaces::engine->get_viewangles(AutoStrafeView);
	if (options::menu.misc.OtherAutoStrafe.getstate())
	{
		strafer(pCmd);
	}

	if (GetAsyncKeyState(options::menu.misc.fw.GetKey()) && !options::menu.m_bIsOpen)
	{
		FakeWalk0(pCmd, bSendPacket);
	}


	if (!_done && hackManager.pLocal()->IsAlive())
	{
		if (options::menu.misc.buybot_primary.getindex() != 0)
			buybot_primary();

		if (options::menu.misc.buybot_secondary.getindex() != 0)
			buybot_secondary();

		buybot_otr();

		_done = true;
	}

	if (options::menu.misc.NameChanger.getstate())
	{
		namespam();
	}

	bool unlocked = false;
	if (options::menu.misc.fakelag_unlock.getstate())
	{
		remove_fakelag_limit();
		unlocked = true, options::menu.misc.fakelag_unlock.SetState(false);

	}

	if (unlocked)
	{
		if (options::menu.misc.FakelagStand.getvalue() > 14)
			options::menu.misc.FakelagStand.setvalue(14);

		if (options::menu.misc.FakelagMove.getvalue() > 14)
			options::menu.misc.FakelagMove.setvalue(14);

		if (options::menu.misc.Fakelagjump.getvalue() > 14)
			options::menu.misc.Fakelagjump.setvalue(14);

		if (options::menu.misc.fakelag_fd.getvalue() > 14)
			options::menu.misc.fakelag_fd.setvalue(14);


		if (options::menu.misc.fakelag_fd.getvalue() > 15.500 && options::menu.misc.fakelag_fd.getvalue() < 16)
			options::menu.misc.fakelag_fd.setvalue(16);

		if (options::menu.misc.fakelag_fd.getvalue() > 14.500 && options::menu.misc.fakelag_fd.getvalue() <= 15.500)
			options::menu.misc.fakelag_fd.setvalue(15);


	}

	if (options::menu.misc.fakelag_fd.getvalue() > 13.500 && options::menu.misc.fakelag_fd.getvalue() <= 14.500)
		options::menu.misc.fakelag_fd.setvalue(14);

	if (options::menu.misc.fakelag_fd.getvalue() > 12.500 && options::menu.misc.fakelag_fd.getvalue() <= 13.500)
		options::menu.misc.fakelag_fd.setvalue(13);
}
int CMiscHacks::GetFPS()
{
	static int fps = 0;
	static int count = 0;
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	static auto last = high_resolution_clock::now();
	count++;
	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}
	return fps;
}
float curtime_fixedx(CUserCmd* ucmd) {
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}

void VectorAnglesXXX(Vector forward, Vector &angles)
{
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);

		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector CalcAngleFakewalk(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesXXX(dst - src, ret);
	return ret;
}

void rotate_movement(float yaw, CUserCmd* cmd)
{
	Vector viewangles;
	QAngle yamom;
	interfaces::engine->get_viewangles(viewangles);
	float rotation = DEG2RAD(viewangles.y - yaw);
	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);
	float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
	float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);
	cmd->forwardmove = new_forwardmove;
	cmd->sidemove = new_sidemove;
}

float fakewalk_curtime(CUserCmd* ucmd)
{
	auto local_player = hackManager.pLocal();

	if (!local_player)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted)
	{
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}
void CMiscHacks::FakeWalk0(CUserCmd* pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	globalsh.fakewalk = true;
	static int iChoked = -1;
	iChoked++;
	if (pCmd->forwardmove > 0)
	{
		pCmd->buttons |= IN_BACK;
		pCmd->buttons &= ~IN_FORWARD;
	}
	if (pCmd->forwardmove < 0)
	{
		pCmd->buttons |= IN_FORWARD;
		pCmd->buttons &= ~IN_BACK;
	}
	if (pCmd->sidemove < 0)
	{
		pCmd->buttons |= IN_MOVERIGHT;
		pCmd->buttons &= ~IN_MOVELEFT;
	}
	if (pCmd->sidemove > 0)
	{
		pCmd->buttons |= IN_MOVELEFT;
		pCmd->buttons &= ~IN_MOVERIGHT;
	}
	static int choked = 0;
	choked = choked > 14 ? 0 : choked + 1;

	float nani = options::menu.misc.FakeWalkSpeed.getvalue() / 14;

	pCmd->forwardmove = choked < nani || choked > 14 ? 0 : pCmd->forwardmove;
	pCmd->sidemove = choked < nani || choked > 14 ? 0 : pCmd->sidemove; //100:6 are about 16,6, quick maths
	bSendPacket = choked < 1;
}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF8h
		sub     esp, 44h
		push    ebx
		push    esi
		push    edi
		mov     edi, cvar
		mov     esi, value
		jmp     pfn
	}
}

void CMiscHacks::AutoPistol(CUserCmd* pCmd)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	static bool WasFiring = false;
	if (game_utils::IsPistol(pWeapon) && !game_utils::IsBomb(pWeapon))
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			if (WasFiring)
			{
				pCmd->buttons &= ~IN_ATTACK;
				ragebot->was_firing = true;
			}
		}
		else
			ragebot->was_firing = false;

		WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
	}
	else
		return;
}
void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	auto g_LocalPlayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto userCMD = pCmd;
	if (options::menu.misc.autojump_type.getindex() < 1)
	{
		if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER) return;
		if (userCMD->buttons & IN_JUMP && !(g_LocalPlayer->GetFlags() & FL_ONGROUND))
		{
			userCMD->buttons &= ~IN_JUMP;
		}
	}
	if (options::menu.misc.autojump_type.getindex() > 0)
	{
		if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
			return;
		userCMD->buttons |= IN_JUMP;
	}
}
void CMiscHacks::airduck(CUserCmd *pCmd) // quack
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (options::menu.misc.airduck_type.getindex() == 1)
	{
		if (!(local->GetFlags() & FL_ONGROUND))
		{
			pCmd->buttons |= IN_DUCK;
		}
	}
	if (options::menu.misc.airduck_type.getindex() == 2)
	{
		if (!(local->GetFlags() & FL_ONGROUND))
		{
			static bool counter = false;
			static int counters = 0;
			if (counters == 9)
			{
				counters = 0;
				counter = !counter;
			}
			counters++;
			if (counter)
			{
				pCmd->buttons |= IN_DUCK;
			}
			else
				pCmd->buttons &= ~IN_DUCK;
		}
	}
}
template<class T, class U>
inline T clampangle(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

#define nn(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
bool bHasGroundSurface(IClientEntity* pLocalBaseEntity, const Vector& vPosition)
{
	trace_t pTrace;
	Vector vMins, vMaxs; pLocalBaseEntity->GetRenderBounds(vMins, vMaxs);

	UTIL_TraceLine(vPosition, { vPosition.x, vPosition.y, vPosition.z - 32.f }, MASK_PLAYERSOLID_BRUSHONLY, pLocalBaseEntity, 0, &pTrace);

	return pTrace.fraction != 1.f;
}

void CMiscHacks::strafe_2(CUserCmd * cmd)
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (local->GetMoveType() == MOVETYPE_NOCLIP || local->GetMoveType() == MOVETYPE_LADDER  || !local || !local->IsAlive())
		return;

	if (interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_A) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_D) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_S) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_W) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_LSHIFT))
		return;

	if (!(local->GetFlags() & FL_ONGROUND)) {
		if (cmd->mousedx > 1 || cmd->mousedx < -1) {
			cmd->sidemove = clamp(cmd->mousedx < 0.f ? -450.0f : 450.0f, -450.0f, 450.0f);
		}
		else {
			cmd->forwardmove = 10000.f / local->getvelocity().Length();
			cmd->sidemove = (cmd->command_number % 2) == 0 ? -450.0f : 450.0f;
			if (cmd->forwardmove > 450.0f)
				cmd->forwardmove = 450.0f;
		}
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}

void CMiscHacks::PostProcces()
{
	ConVar* Meme = interfaces::cvar->FindVar("mat_postprocess_enable");
	SpoofedConvar* meme_spoofed = new SpoofedConvar(Meme);
	meme_spoofed->SetString("mat_postprocess_enable 0");
}

void CMiscHacks::MinimalWalk(CUserCmd* cmd, float speed)
{
	if (speed <= 0.f)
		return;

	float fSpeed = (float)(FastSqrt(square(cmd->forwardmove) + square(cmd->sidemove) + square(cmd->upmove)));

	if (fSpeed <= 0.f)
		return;

	if (cmd->buttons & IN_DUCK)
		speed *= 2.94117647f;

	if (fSpeed <= speed)
		return;

	float fRatio = speed / fSpeed;

	cmd->forwardmove *= fRatio;
	cmd->sidemove *= fRatio;
	cmd->upmove *= fRatio;

	interfaces::globals->frametime *= (hackManager.pLocal()->getvelocity().Length2D()) / 1.25;
}
void CMiscHacks::fake_crouch(CUserCmd * cmd, bool &packet, IClientEntity * local)  // appears pasted
{
	static bool counter = false;


	if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()))
	{
	
		if (options::menu.misc.fake_crouch.getstate())
		{
			unsigned int chokegoal = options::menu.misc.fakelag_fd.getvalue() / 2;
			auto choke = *(int*)(uintptr_t(interfaces::client_state) + 0x4D28);
			bool mexican_tryhard = choke >= chokegoal;

			if (local->GetFlags() & FL_ONGROUND)
			{
				if (mexican_tryhard  || interfaces::client_state->m_flNextCmdTime <= 0.1f  )
					cmd->buttons |= IN_DUCK;
				else
					cmd->buttons &= ~IN_DUCK;
			}
		}
	}
}
float CMiscHacks::get_gun(C_BaseCombatWeapon* weapon)
{

	if (!weapon)
		return 0.f;

	if (weapon->isAuto())
		return 39.f;

	else if (weapon->is_scout())
		return 70.f;

	else if (weapon->is_awp())
		return 30.f;

	else
		return 34.f;
}

void CMiscHacks::strafer(CUserCmd* cmd) {

	if (!GetAsyncKeyState(VK_SPACE) || hackManager.pLocal()->getvelocity().Length2D() < 0.5)
		return;

	if (!(hackManager.pLocal()->GetFlags() & FL_ONGROUND))
	{
		static float cl_sidespeed = interfaces::cvar->FindVar("cl_sidespeed")->GetFloat();
		if (fabsf(cmd->mousedx > 2)) {
			cmd->sidemove = (cmd->mousedx < 0.f) ? -cl_sidespeed : cl_sidespeed;
			return;
		}

		/*
		if (GetAsyncKeyState('S')) {
			cmd->viewangles.y -= 180;
		}
		else if (GetAsyncKeyState('D')) {
			cmd->viewangles.y += 90;
		}
		else if (GetAsyncKeyState('A')) {
			cmd->viewangles.y -= 90;
		}
		*/

		if (!hackManager.pLocal()->getvelocity().Length2D() > 0.5 || hackManager.pLocal()->getvelocity().Length2D() == NAN || hackManager.pLocal()->getvelocity().Length2D() == INFINITE)
		{
			cmd->forwardmove = 400;
			return;
		}

		cmd->forwardmove = clamp(5850.f / hackManager.pLocal()->getvelocity().Length2D(), -400, 400);
		if ((cmd->forwardmove < -400 || cmd->forwardmove > 400))
			cmd->forwardmove = 0;

		const auto vel = hackManager.pLocal()->getvelocity();
		const float y_vel = RAD2DEG(atan2(vel.y, vel.x));
		const float diff_ang = normalize_yaw(cmd->viewangles.y - y_vel);

		cmd->sidemove = (diff_ang > 0.0) ? -cl_sidespeed : cl_sidespeed;
		cmd->viewangles.y = normalize_yaw(cmd->viewangles.y - diff_ang);

	}
}

void CMiscHacks::viewmodel_x_y_z()
{
	static int vx, vy, vz, b1g;
	static ConVar* view_x = interfaces::cvar->FindVar("viewmodel_offset_x");
	static ConVar* view_y = interfaces::cvar->FindVar("viewmodel_offset_y");
	static ConVar* view_z = interfaces::cvar->FindVar("viewmodel_offset_z");

	static ConVar* bob = interfaces::cvar->FindVar("cl_bobcycle"); // sv_competitive_minspec 0

	ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
	SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
	sv_cheats_spoofed->SetInt(1);

	ConVar* sv_minspec = interfaces::cvar->FindVar("sv_competitive_minspec");
	SpoofedConvar* sv_minspec_spoofed = new SpoofedConvar(sv_minspec);
	sv_minspec_spoofed->SetInt(0);


	view_x->nFlags &= ~FCVAR_CHEAT;
	view_y->nFlags &= ~FCVAR_CHEAT;
	view_z->nFlags &= ~FCVAR_CHEAT;
	bob->nFlags &= ~FCVAR_CHEAT;

	vx = options::menu.visuals.offset_x.getvalue();
	vy = options::menu.visuals.offset_y.getvalue();
	vz = options::menu.visuals.offset_z.getvalue();
	b1g = 0.98f;

	view_x->SetValue(vx);

	view_y->SetValue(vy);

	view_z->SetValue(vz);

	if (!paste)
	{
		interfaces::engine->ExecuteClientCmd("cl_bobcycle 0.98"); //  // rate 196608
		interfaces::engine->ExecuteClientCmd("rate 196608");
		paste = true;
	}
}

void CMiscHacks::optimize()
{
	static bool done = false;

	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
	{
		done = false;
		return;
	}

	angle_correction ac;
	if (!done)
	{
		ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
		SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
		sv_cheats_spoofed->SetInt(1);

		auto da2 = interfaces::cvar->FindVar("cl_disable_ragdolls"); 
		da2->SetValue(1);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] cl_disable_ragdolls was set to 1.     \n");

		auto da3 = interfaces::cvar->FindVar("dsp_slow_cpu"); 
		da3->SetValue(2);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] dsp_slow_cpu was set to 2.     \n");

		auto da5 = interfaces::cvar->FindVar("mat_disable_bloom");
		da5->SetValue(1);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] mat_disable_bloom was set to 1.     \n");

//		auto da6 = interfaces::cvar->FindVar("r_drawparticles"); 
//		da6->SetValue(0);
		auto da7 = interfaces::cvar->FindVar("func_break_max_pieces");
		da7->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] func_break_max_pieces was set to 0.     \n");

		auto da8 = interfaces::cvar->FindVar("muzzleflash_light");
		da8->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] muzzleflash_light was set to 0.     \n");

		auto da9 = interfaces::cvar->FindVar("r_eyemove"); 
		da9->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] r_eyemove was set to 0.     \n");

		auto da10 = interfaces::cvar->FindVar("r_eyegloss");
		da10->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] r_eyegloss was set to 0.     \n");

		auto da11 = interfaces::cvar->FindVar("mat_queue_mode"); 
		da11->SetValue(2);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] mat_queue_mode was set to 2.     \n");

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(10, 200, 250, 255), " [info] game graphics have been optimized.     \n");

//		auto xd80 = interfaces::cvar->FindVar("r_showenvcubemap"); 
//		xd80->SetValue(0);
//		auto xd81 = interfaces::cvar->FindVar("r_drawtranslucentrenderables");// <----
//		xd81->SetValue(0);
		done = true;
	}
	//	Interfaces::Engine->ExecuteClientCmd("r_drawtranslucentrenderables 0");
}

void CMiscHacks::LoadNamedSky(const char *sky_name)
{
	static auto fnLoadNamedSkys = (void(__fastcall*)(const char*))game_utils::FindPattern1("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45");
	fnLoadNamedSkys(sky_name);
	anotherpcheck = false;
}
void CMiscHacks::colour_modulation()
{
	static bool freakware = false;

	if (options::menu.visuals.colmodupdate.getstate())
	{

		ConVar* staticdrop = interfaces::cvar->FindVar("r_DrawSpecificStaticProp");
		SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
		staticdrop_spoofed->SetInt(0);
		ConVar* NightSkybox1 = interfaces::cvar->FindVar("sv_skyname");
		*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
		for (MaterialHandle_t i = interfaces::materialsystem->FirstMaterial(); i != interfaces::materialsystem->InvalidMaterial(); i = interfaces::materialsystem->NextMaterial(i))
		{
			IMaterial *pMaterial = interfaces::materialsystem->GetMaterial(i);

			interfaces::engine->ExecuteClientCmd("mat_queue_mode 2"); // rate 196608 dsp_slow_cpu

			if (!pMaterial)
				continue;
			if (!pMaterial || pMaterial->IsErrorMaterial())
				continue;

			float sky_r = options::menu.visuals.sky_r.getvalue() / 10;
			float sky_g = options::menu.visuals.sky_g.getvalue() / 10;
			float sky_b = options::menu.visuals.sky_b.getvalue() / 10;

			float test = options::menu.visuals.asusamount.getvalue() / 100;
			float amountr = options::menu.visuals.colmod.getvalue() / 100;

			switch (options::menu.visuals.customskies.getindex())
			{
			case 1:
			{
				NightSkybox1->SetValue("sky_csgo_night02b");
			}
			break;

			case 2:
			{
				NightSkybox1->SetValue("sky_l4d_rural02_ldr");
			}
			break;

			case 3:
			{
				LoadNamedSky("sky_descent");
			}
			break;
			}


			if (options::menu.visuals.ModulateSkyBox.getstate() && strstr(pMaterial->GetTextureGroupName(), ("SkyBox")))
			{
				pMaterial->ColorModulate(sky_r, sky_g, sky_b);
			}

			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures") && options::menu.ColorsTab.asus_type.getindex() < 1)  // walls	
			{
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures") && options::menu.ColorsTab.asus_type.getindex() > 0)  // walls	
			{
				pMaterial->AlphaModulate(test);
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
			{
				pMaterial->AlphaModulate(test);
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}

		}
		options::menu.visuals.colmodupdate.SetState(false);
	}
}

typedef void(*RevealAllFn)(int);
RevealAllFn fnReveal;
void CMiscHacks::RankReveal(CUserCmd * cmd)
{
	if (!options::menu.visuals.CompRank.getstate())
		return;

	using ServerRankRevealAll = char(__cdecl*)(int*); static uintptr_t RankRevealFnc = Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 8B 0D ? ? ? ? 85 C9 75 ? A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 ? 85 C0 74 ? 8B C8 E8 ? ? ? ? 8B C8 EB ? 33 C9 89 0D ? ? ? ? 8B 45 ? FF 70 ? E8 ? ? ? ? B0 ? 5D"); printf("RankReveal: 0x%X\n", RankRevealFnc); // Print client_panorama.dll+0x38CF60 int v[3] = { 0, 0, 0 }; reinterpret_cast< ServerRankRevealAll >(RankRevealFnc)(v);
}

typedef void(__cdecl* MsgFnX)(const char* msg, va_list);

void MsgXD(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFnX fn = (MsgFnX)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}

void CMiscHacks::buy_logs(IGameEvent* Event)
{
	auto userid = Event->GetInt("userid");
	if (!userid)
		return;

	auto engine_userid = interfaces::engine->GetPlayerForUserID(userid);
	if (!engine_userid)
		return;

	player_info_t info;
	interfaces::engine->GetPlayerInfo(engine_userid, &info);

	auto local_player = reinterpret_cast<IClientEntity*>(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));
	auto entity = reinterpret_cast<IClientEntity*>(interfaces::ent_list->get_client_entity(engine_userid));

	if (!local_player || !entity)
		return;

	if (entity->team() == local_player->team())
		return;

//	std::transform(player_name.begin(), player_name.end(), player_name.begin(), ::tolower);
//	std::string
	std::string player = info.name;
	std::string bought = "has purchased a";
	std::string weapon = Event->GetString("weapon");
	std::string newline = ".     \n";
	std::string uremam = player + bought + weapon + newline;

	MsgXD(uremam.c_str());
	
}