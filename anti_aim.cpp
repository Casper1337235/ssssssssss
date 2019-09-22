#include "antiaim.h"
#include "Hooks.h"
#include "MathFunctions.h"
#include "RageBot.h"
#include "fakelag.h"
#include "MiscHacks.h"
anti_aim * c_antiaim = new anti_aim();
antiaim_helper * c_helper = new antiaim_helper();

static bool dir = false;
static bool back = false;
static bool up = false;

static bool desync_left = false;
static bool desync_right = false;

static bool jitter = false;
static bool jitter2 = false;
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

float anti_aim::get_feet_yaw()
{
	auto GetLocalPlayer = static_cast<IClientEntity*>(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));

	if (!GetLocalPlayer)
		return 0.f;

	auto state = GetLocalPlayer->GetBasePlayerAnimState();

	float current_feet_yaw = state->goal_feet_yaw;

	if (current_feet_yaw >= -360)
		current_feet_yaw = min(current_feet_yaw, 360.f);

	return current_feet_yaw;
}

float get_curtime(CUserCmd* ucmd)
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local_player)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}

void next_lby_update(CUserCmd* cmd)
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local_player)
		return;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto anim_state = local_player->get_animation_state();
	if (!anim_state)
		return;

	auto net_channel = interfaces::engine->GetNetChannelInfo();

	if (!net_channel || net_channel->m_nChokedPackets)
		return;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return;

	float next_lby_update;
	bool broke_this_tick = false;
	auto server_time = local_player->m_nTickBase() * interfaces::globals->interval_per_tick;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (anim_state->speed_2d)
	{
		/*anim_state->goal_feet_yaw = flApproachAngle(
		anim_state->eye_angles_y,
		anim_state->goal_feet_yaw,
		( ( anim_state->m_flUnknownFraction * 20.0f ) + 30.0f )
		* anim_state->last_client_side_animation_update_time );*/

		next_lby_update = server_time + 0.22f;
	}
	else if (anim_state->speed_2d < 0.5f) {
		/*anim_state->goal_feet_yaw = flApproachAngle(
		local_player->m_flLowerBodyYawTarget( ),
		anim_state->goal_feet_yaw,
		anim_state->last_client_side_animation_update_time * 100.0f );*/

		if (server_time > next_lby_update) {
			cmd->viewangles.y = c_beam->real + 125.f; //base_yaw + body_yaw( cmd );
			next_lby_update = server_time + 1.1f;
			broke_this_tick = true;
		}
	}

	if (broke_this_tick) {
		if (!pWeapon->IsMiscGAY())
			cmd->buttons &= ~IN_ATTACK;
	}
}
#include <chrono>
static bool shouldBreak = false;
static bool brokeThisTick = false;
static bool fake_walk = false;
static int chocked = 0;
static bool gaymode = false;
static bool doubleflick = false;
static bool has_broken = false;
static bool lbyupdate = false;
static bool nextlbyupdate = false;
static bool broken = false;
static bool wasMoving = true;
static bool preBreak = false;

int GetFPS()
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
}bool break_lby = false;
void anti_lby(CUserCmd* cmd, bool& bSendPacket)
{
	if (options::menu.misc.desync_type_stand.getindex() < 1)
		return;

	auto local_player = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!local_player)
		return;

	auto local_weapon = local_player->GetActiveWeaponHandle();

	if (!local_weapon)
		return;

	float b = rand() % 4;

	static float oldCurtime = interfaces::globals->curtime;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (bSendPacket)
	{
		brokeThisTick = false;
		chocked =  interfaces::client_state->chokedcommands;

		if (local_player->getvelocity().Length2D() >= 0.1f && (local_player->GetFlags() & FL_ONGROUND))
		{
			if (GetAsyncKeyState(VK_SHIFT))
			{
				wasMoving = false;
				oldCurtime = interfaces::globals->curtime;
				if (interfaces::globals->curtime - oldCurtime >= 1.1f)
				{
					lbyupdate = true;
					shouldBreak = true;
					nextlbyupdate = interfaces::globals->curtime;
				}
			}
			else
			{
				oldCurtime = interfaces::globals->curtime;
				wasMoving = true;
				has_broken = false;
			}
		}

		else
		{
			if (wasMoving && interfaces::globals->curtime - oldCurtime > 0.22f)
			{
				wasMoving = false;
				lbyupdate = true;
				shouldBreak = true;
				nextlbyupdate = interfaces::globals->curtime;
			}

			else if (interfaces::globals->curtime - oldCurtime > 1.1f)
			{
				shouldBreak = true;
				nextlbyupdate = interfaces::globals->curtime;
			}

			else if (interfaces::globals->curtime - oldCurtime > 1.1f - TICKS_TO_TIME(chocked) - TICKS_TO_TIME(2))
			{
				lbyupdate = true;
				preBreak = true;
			}

			else if (interfaces::globals->curtime - oldCurtime > 1.f - TICKS_TO_TIME(chocked + 12))
				doubleflick = true;

			else
			{
				lbyupdate = false;
			}
		}
	}
	else if (shouldBreak)
	{
		float r = rand() % 35;
		brokeThisTick = true;
		oldCurtime = interfaces::globals->curtime;
		break_lby = true;
		cmd->viewangles.y -= options::menu.misc.desync_type_stand.getindex() > 5 ? 100 + r: 120.f + r;
		break_lby = false;
		shouldBreak = false;

	}


}



#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)

//--------------------------------------------------------------------------------
void anti_aim::DoYaw(CUserCmd* pCmd, IClientEntity* pLocal, bool &bSendPacket)
{

	if (GetAsyncKeyState(options::menu.misc.manualleft.GetKey())) // right
	{
		dir = true;
		back = false;
		up = false;
		bigboi::indicator = 1;
	}

	if (GetAsyncKeyState(options::menu.misc.manualright.GetKey())) // left
	{
		dir = false;
		back = false;
		up = false;
		bigboi::indicator = 2;
	}

	if (GetAsyncKeyState(options::menu.misc.manualback.GetKey()))
	{
		dir = false;
		back = true;
		up = false;
		bigboi::indicator = 3;
	}

	if (GetAsyncKeyState(options::menu.misc.manualfront.GetKey()))
	{
		dir = false;
		back = false;
		up = true;
		bigboi::indicator = 4;
	}

	IClientEntity* local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!local)
		return;

	if ((pCmd->buttons & IN_ATTACK) && !options::menu.misc.desync_twist_onshot.getstate())
		return;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	bool standing = pLocal->getvelocity().Length2D() <= 0.1;
	bool on_ground = (pLocal->GetFlags() & FL_ONGROUND);

	if (standing)
	{

		if (options::menu.misc.desync_type_stand.getindex() < 6)
		{
			if (!options::menu.misc.desync_aa_stand.getstate())
			{
				if (!bSendPacket || bSendPacket)
					selection(pCmd, false, false);
			}
			else
			{
				if (!bSendPacket || (bSendPacket && !(pCmd->command_number % 3)))
				{
					selection(pCmd, false, false);
				}
				else
				{
					do_desync(pCmd, false, bSendPacket);
				}
			}
		}
		else
		{
			if (!options::menu.misc.desync_aa_stand.getstate())
			{
				if (!bSendPacket || bSendPacket)
					selection(pCmd, false, false);
			}
			else
			{
				if (!bSendPacket)
				{
					selection(pCmd, false, false);
				}
				else
				{
					do_desync(pCmd, false, bSendPacket);
				}
			}
		}

	}
	

	if (!standing)
	{

		if (options::menu.misc.desync_type_move.getindex() < 6)
		{
			if (!options::menu.misc.desync_aa_move.getstate())
			{
				if (!bSendPacket || bSendPacket)
					on_ground ? selection(pCmd, true, false) : selection(pCmd, true, true);
			}
			else
			{
				if (!bSendPacket)
				{
					on_ground ? selection(pCmd, true, false) : selection(pCmd, true, true);
				}
				else
				{
					do_desync(pCmd, true, bSendPacket);
				}
			}
		}
		else
		{
			if (!options::menu.misc.desync_aa_move.getstate())
			{
				if (!bSendPacket || bSendPacket)
					on_ground ? selection(pCmd, true, false) : selection(pCmd, true, true);
			}
			else
			{
				if (!bSendPacket)
				{
					selection(pCmd, true, true);
				}
				else
				{
					do_desync(pCmd, true, bSendPacket);
				}
			}
		}

	}

	// ez
}

void anti_aim::selection(CUserCmd * pcmd, bool moving, bool air) // if (is_oxygen)
{
	if (!moving)
	{
		switch (options::menu.misc.AntiAimYaw.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;

		case 7:
		{
			freaky(pcmd, moving);
		}
		break;
		}

	}

	if (moving && !air)
	{
		switch (options::menu.misc.AntiAimYawrun.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;

		case 7:
		{
			freaky(pcmd, moving);
		}
		break;
		}
	}

	if (moving && air)
	{
		switch (options::menu.misc.AntiAimYaw3.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;

		case 7:
		{
			freaky(pcmd, moving);
		}
		break;
		}
	}
}

void anti_aim::DoPitch(CUserCmd * pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (pCmd->buttons & IN_ATTACK && !(game_utils::IsPistol(pWeapon) && game_utils::AutoSniper(pWeapon)) && options::menu.misc.OtherSafeMode.getindex() < 3)
		return;

	bool untrusted = options::menu.misc.OtherSafeMode.getindex() > 2;
	switch (options::menu.misc.AntiAimPitch.getindex())
	{
	case 0:
		break;
	case 1:
		untrusted ? fakedown(pCmd) : pitchdown(pCmd);
		break;
	case 2:
		untrusted ? fakeup(pCmd) : pitchup(pCmd);
		break;
	case 3:
	{
		untrusted ? pitch_fakejitter(pCmd) : pitchjitter(pCmd);
	}
	break;
	case 4:
	{
		untrusted ? pitch_fakerandom(pCmd) : pitchrandom(pCmd);
	}
	break;
	case 5:
	{
		untrusted ? pCmd->viewangles.x = -180540.f : zero(pCmd);
	}


	}
}

void anti_aim::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	if (pLocal->movetype() == MOVETYPE_LADDER || pLocal->movetype() == MOVETYPE_NOCLIP)
		return;

	if (pCmd->buttons & IN_USE)
		return;

	if (game_utils::IsGrenade(pWeapon) && pWeapon->GetThrowTime() > 0)
		return;

	if (options::menu.misc.disable_on_dormant.getstate())
	{
		if (c_helper->closest() == -1)
			return;
	}

	if (interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_LEFT) || (pWeapon->IsKnife() && interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_RIGHT)))
		return;

	if (options::menu.misc.desync_twist_onshot.getstate())
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			if (options::menu.aimbot.resolver.getindex() == 2 ? bSendPacket : (bSendPacket &&  pCmd->command_number % 3))
				options::menu.aimbot.resolver.getindex() == 2 ? pCmd->viewangles.y += 100.f : (pCmd->viewangles.y -= 29.f,  *hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() -= 110.f);
		}
	}

	if (pWeapon->isZeus27() && c_misc->do_zeus == true)
		return;


	DoPitch(pCmd);

	if (options::menu.misc.antilby.getstate())
	{
		anti_lby(pCmd, bSendPacket);
	}

	DoYaw(pCmd, hackManager.pLocal(), bSendPacket);
}

void anti_aim::backwards(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	jitter2 = !jitter2;
	pcmd->viewangles.y += 180 + (jitter2 ? c : -c);
}

void anti_aim::jitter_side(CUserCmd * pCmd)
{
	jitter2 = !jitter2;
	pCmd->viewangles.y = jitter2 ? 90 : -90;
}

/*
void anti_aim::backwards_jitter(CUserCmd * pcmd, bool moving)
{
jitter2 = !jitter2;
pcmd->viewangles.y += moving ? (jitter2 ? 130 : -130) : (jitter2 ? 145 + rand() % 15 : -145 - rand() % 15);
}
*/

void anti_aim::lowerbody(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	jitter2 = !jitter2;

	pcmd->viewangles.y = (hackManager.pLocal()->get_lowerbody() + options::menu.misc.lby1.getvalue()) + (jitter2 ? c : -c);
}

void anti_aim::rand_lowerbody(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	jitter2 = !jitter2;

	pcmd->viewangles.y = hackManager.pLocal()->get_lowerbody() + RandomFloat(options::menu.misc.randlbyr.getvalue(), -options::menu.misc.randlbyr.getvalue() + (jitter2 ? c : -c));
}

void anti_aim::jitter_180(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	jitter2 = !jitter2;
	pcmd->viewangles.y += jitter2 ? 180 : 0  + RandomFloat(c, -c);
}

void anti_aim::manual(CUserCmd * pCmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	jitter2 = !jitter2;
	if (dir && !back && !up)
		pCmd->viewangles.y -= 90.f + (jitter2 ? c : -c);
	else if (!dir && !back && !up)
		pCmd->viewangles.y += 90.f + (jitter2 ? c : -c);
	else if (!dir && back && !up)
		pCmd->viewangles.y -= 180.f + (jitter2 ? c : -c);
	else if (!dir && !back && up)
		pCmd->viewangles.y += (jitter2 ? c : -c);
}

void anti_aim::pitchdown(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 89.f;
}

void anti_aim::pitchup(CUserCmd * pcmd)
{
	pcmd->viewangles.x = -89.f;
}

void anti_aim::zero(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f;
}

void anti_aim::pitchjitter(CUserCmd * pcmd)
{
	if (jitter)
		pcmd->viewangles.x = 89.f;
	else
		pcmd->viewangles.x = -89.f;
	jitter = !jitter;
}

void anti_aim::pitch_fakejitter(CUserCmd * pcmd)
{
	if (jitter)
		pcmd->viewangles.x = 540.f;
	else
		pcmd->viewangles.x = -540.f;
	jitter = !jitter;
}

void anti_aim::pitchrandom(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f + RandomFloat(-89.f, 89.f);
}

void anti_aim::pitch_fakerandom(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f + RandomFloat(-540.f, 540.f);
}

void anti_aim::fakedown(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 540.f;
}

void anti_aim::fakeup(CUserCmd * pcmd)
{
	pcmd->viewangles.x = -540;
}


#define RandomInt(min, max) (rand() % (max - min + 1) + min)
#define	MASK_ALL				(0xFFFFFFFF)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 			/**< everything that is normally solid */
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 	/**< everything that blocks player movement */
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) /**< blocks npc movement */
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME) 							/**< water physics in these contents */
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE) 							/**< everything that blocks line of sight for AI, lighting, etc */
#define MASK_OPAQUE_AND_NPCS	(MASK_OPAQUE|CONTENTS_MONSTER)										/**< everything that blocks line of sight for AI, lighting, etc, but with monsters added. */
#define	MASK_VISIBLE			(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE) 								/**< everything that blocks line of sight for players */
#define MASK_VISIBLE_AND_NPCS	(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE) 							/**< everything that blocks line of sight for players, but with monsters added. */
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX) 	/**< bullets see these as solid */
#define MASK_SHOT_HULL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE) 	/**< non-raycasted weapons see this as solid (includes grates) */
#define MASK_SHOT_PORTAL		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW) 							/**< hits solids (not grates) and passes through everything else */
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS) // non-raycasted weapons see this as solid (includes grates)
#define MASK_SOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE) 					/**< everything normally solid, except monsters (world+brush only) */
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for player movement, except monsters (world+brush only) */
#define MASK_NPCSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for npc movement, except monsters (world+brush only) */
#define MASK_NPCWORLDSTATIC		(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 					/**< just the world, used for route rebuilding */
#define MASK_SPLITAREAPORTAL	(CONTENTS_WATER|CONTENTS_SLIME) 		

void anti_aim::freestanding_jitter(CUserCmd* pCmd, bool moving)
{

	IClientEntity* GetLocalPlayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	float range = options::menu.misc.freerange.getvalue() / 2;
	static int Ticks = 0;

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	static float hold = 0.f;
	Vector besthead;
	float opposite = 0.f;

	auto leyepos = hackManager.pLocal()->GetOrigin_likeajew() + hackManager.pLocal()->GetViewOffset();
	auto headpos = hitbox_location(GetLocalPlayer, 0);
	auto origin = hackManager.pLocal()->GetOrigin_likeajew();

	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->GetOrigin_likeajew() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, hackManager.pLocal());

		trace_t trace1, trace2;
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS | CONTENTS_GRATE */, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS | CONTENTS_GRATE */, &filter, &trace2);

	//	UTIL_TraceLine(data.src, End_Point, 0x4600400B, local, 0, &data.enter_trace);
		 
		if (trace2.DidHit())
			endpos2 = trace2.endpos;

		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;

	};

	int index = c_helper->closest();
	static IClientEntity* entity;

	if (index != -1)
		entity = interfaces::ent_list->get_client_entity(index); // maybe?

	if (!entity->isValidPlayer())
	{
		pCmd->viewangles.y -= 180.f;
		return;
	}

	float radius = Vector(headpos - origin).Length2D();

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float besthead = 0; besthead < 7; besthead += 0.1)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				opposite = besthead - 180;
				bestrotation = besthead;
			}
		}
	}
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	if (no_active)
	{
		pCmd->viewangles.y -= 180.f + RandomFloat(-c, c);
	}

	else
	{
		jitter = !jitter;
		pCmd->viewangles.y = jitter ? RAD2DEG(bestrotation) + c : RAD2DEG(bestrotation) - c;
	}
}

void anti_aim::crooked(CUserCmd * pcmd, bool moving) //by faxzee
{
	jitter2 = !jitter2;
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();
	float flCrookedoffset = 120.f;

	if (jitter2) {
		pcmd->viewangles.y = hackManager.pLocal()->get_lowerbody() + flCrookedoffset;
		pcmd->viewangles.y += c;
	}
	else {
		pcmd->viewangles.y = hackManager.pLocal()->get_lowerbody() + flCrookedoffset;
		pcmd->viewangles.y -= c;
	}
}

float normalize_yaw180(float yaw)
{
	if (yaw > 180)
		yaw -= (round(yaw / 360) * 360.f);
	else if (yaw < -180)
		yaw += (round(yaw / 360) * -360.f);

	return yaw;
}

float next_update = 0;
void anti_aim::do_desync(CUserCmd * cmd, bool moving, bool packet)
{
	if (!cmd)
		return;

	auto state = hackManager.pLocal()->GetBasePlayerAnimState();

	if (!state)
		return;
	IClientEntity* local = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	// hey, Divinity guy, ya know you don't have to define the same god damn thing twice just for simple stuff, or anything at all, right?
	// but hey hey i no buli :>

	if (GetAsyncKeyState(options::menu.misc.manual_desync_left.GetKey())) // right
	{
		bigboi::desync_manual = 1;
	}

	if (GetAsyncKeyState(options::menu.misc.manual_desync_right.GetKey())) // left
	{

		bigboi::desync_manual = 2;
	}

	/*
	yo yo, can you believe that i've been running the MOVING desync code inside the standing desync? Fuck my life ahaha
	*/
	if (!moving)
	{
		switch (options::menu.misc.desync_type_stand.getindex())
		{
		case 0: {
			if (packet && cmd->command_number % 3)
			{
				*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 58.f;
				cmd->viewangles.y = c_beam->real - 58.f;
			}

			break;
		case 1:
			desync_jitter(cmd, false, packet);
			break;

		case 2:
			desync_stretch_override(cmd, false, packet);
			break;

		case 3:
			desync_laurie_experimental(cmd, false, packet);
			break;
		case 4:
		{
			cmd->viewangles.y = c_beam->real - RandomFloat(50, -50);
			*local->GetBasePlayerAnimState()->feetyaw() = c_beam->real - RandomFloat(50, -50);
		}
		break;
		case 5:
		{

			if (packet)
			{
				float real_lby = c_beam->real - c_beam->lby;
				if (bigboi::desync_manual == 1)
				{
					if (real_lby < -119.f || real_lby > 119.f)
					{
						cmd->viewangles.y = c_beam->real + moving ? 50.f : 90.f;
						*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 75.f;
					}
					else
					{
						cmd->viewangles.y = c_beam->real + moving ? 29.f : 50.f;
						*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 42.f;
					}
				}
				else if (bigboi::desync_manual == 2)
				{
					if (real_lby < -119.f || real_lby > 119.f)
					{
						cmd->viewangles.y = c_beam->real - moving ? 50.f : 90.f;
						*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 75.f;
					}
					else
					{
						cmd->viewangles.y = c_beam->real - moving ? 29.f : 50.f;
						*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 42.f;
					}
				}
			}
		}
		break;
		case 6:
		{
			if (packet)
			{
				cmd->viewangles.y = (c_beam->real - 180) + RandomFloat(100, -100);
			}

		}
		break;
		}

		}
	}
	if (moving)
	{
		switch (options::menu.misc.desync_type_move.getindex())
		{
		case 0: {
			if (packet)
			{
				cmd->viewangles.y = c_beam->real - 29.f;
				if (cmd->command_number % 3)
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 40.f;
	
				//			hackManager.pLocal()->SetAbsAngles(Vector(89.f, c_beam->real - 58.f, 0));
			}
			break;
		case 1:
			desync_jitter(cmd, true, packet);
			break;

		case 2:
			desync_stretch_override(cmd, true, packet);
			break;

		case 3:
			desync_laurie_experimental(cmd, true, packet);
			break;
		case 4:
		{

			cmd->viewangles.y = c_beam->real - RandomFloat(29, -29);
			*local->GetBasePlayerAnimState()->feetyaw() = c_beam->real - RandomFloat(29, -29);
		}
		break;
		case 5:
		{
			float real_lby = c_beam->real - c_beam->lby;
			if (bigboi::desync_manual == 1)
			{
				if (real_lby < -119.f || real_lby > 119.f)
				{
					cmd->viewangles.y = c_beam->real + moving ? 50.f : 90.f;
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 75.f;
				}
				else
				{
					cmd->viewangles.y = c_beam->real + moving ? 29.f : 50.f;
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 42.f;
				}
			}
			else if (bigboi::desync_manual == 2)
			{
				if (real_lby < -119.f || real_lby > 119.f)
				{
					cmd->viewangles.y = c_beam->real - moving ? 50.f : 90.f;
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 75.f;
				}
				else
				{
					cmd->viewangles.y = c_beam->real - moving ? 29.f : 50.f;
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 42.f;
				}
			}

		}
		break;
		case 6:
		{
			if (packet)
			{
				cmd->viewangles.y = (c_beam->real - 180) + RandomFloat(150, -150);
				if (cmd->command_number % 3)
				{
					cmd->viewangles.y = (c_beam->real + 29.f);
				}
			}
		}
		break;
		}

		}

	}


}


void anti_aim::update_lowerbody_breaker() { // from HappyHack by "Incriminating" (unknowncheats)
	float server_time = hackManager.pLocal()->GetTickBase() * interfaces::globals->interval_per_tick, speed = hackManager.pLocal()->getvelocity().Length2DSqr(), oldCurtime = interfaces::globals->curtime;

	if (speed > 0.1) {
		next_update = server_time + 0.22;
	}

	break_lby = false;

	if (next_update <= server_time) {
		next_update = server_time + 1.1;
		break_lby = true;
	}


	if (!(hackManager.pLocal()->GetFlags() & FL_ONGROUND)) {
		break_lby = false;
	}
}

void anti_aim::desync_jitter(CUserCmd * cmd, bool moving, bool packet)
{
	auto feetdelta = hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw - cmd->viewangles.y;

	float desync = moving ? 29.f : 50.f;
	float lby_delta = 180.0f - desync + 10.0f;
	float desync_length = 180.0f - lby_delta - 10.f;
	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}
		cmd->viewangles.y -= 120.0f;
	}

	if (cmd->command_number % 3) 
	{
		if (feetdelta < desync)
		{
			cmd->viewangles.y = c_beam->real + 180.f - desync_length;
			*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 10.f - desync_length;
		}
		else {
			cmd->viewangles.y = c_beam->real + desync_length;
			*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 10 + desync_length;
		}
	}

}

/*
if (dir && !back && !up)
pCmd->viewangles.y -= 90.f + (jitter2 ? c : -c);
else if (!dir && !back && !up)
pCmd->viewangles.y += 90.f + (jitter2 ? c : -c);
*/

void anti_aim::desync_stretch_override(CUserCmd * cmd, bool moving, bool packet)
{
	auto feetdelta = hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw - cmd->viewangles.y;
	float desync = moving ? 29.f : 50.f;
	float lby_delta = 180.0f - desync + 10.0f;
	float desync_length = 180.0f - lby_delta - 10.f;
	float soviet_union = rand() % 21;
	bool it_sucks = soviet_union <= 10;

	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}	
		cmd->viewangles.y +- 120.f;		
	}

	if (packet && cmd->command_number % 3)
	{		
		cmd->viewangles.y = c_beam->real + desync;
		hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw += desync;		
	}

	else
	{
		if (dir && !back && !up)
			cmd->viewangles.y += it_sucks ? 20.f : 170.f ;
		else if (!dir && !back && !up)
			cmd->viewangles.y -= it_sucks ? 20.f : 170.f;
		else if (!dir && back && !up)
			cmd->viewangles.y -= it_sucks ? 120.f : -120.f;
		else if (!dir && !back && up)
			cmd->viewangles.y += it_sucks ? 25.f : -25.f;
	}
}

void anti_aim::desync_laurie_experimental(CUserCmd * cmd, bool moving, bool packet)
{
	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}
		cmd->viewangles.y = c_beam->real - 120.f;
	}

	if (packet && cmd->command_number % 3)
	{
		*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = 90.f;
		cmd->viewangles.y = c_beam->real - moving ? 40.f : 90.f;
		
	}
}

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
float  anti_aim::get_max_desync_delta(IClientEntity* player, CBaseAnimState* anim_state)
{
	if (!player || !anim_state || !player->get_animation_state())
		return 0.f;

	auto ducking_speed = anim_state->speed_2d / (player->max_speed() * 0.340f);

	auto speed_fraction = max(0.0f, min(anim_state->m_flFeetSpeedForwardsOrSideWays, 1.0f));

	auto fl_yaw_modifier = ((anim_state->m_flStopToFullRunningFraction * -0.3f) - 0.2f) * speed_fraction + 1.0f;

	if (anim_state->m_fDuckAmount > 0)
	{
		auto fl_ducking_speed = clamp(ducking_speed, 0.0f, 1.0f);
		fl_yaw_modifier = fl_yaw_modifier + ((anim_state->m_fDuckAmount * fl_ducking_speed) * (0.5f -
			fl_yaw_modifier));
	}

	auto delta = *(float*)((uintptr_t)anim_state + 0x334) * fl_yaw_modifier;

	return delta;
}

void anti_aim::freaky(CUserCmd * cmd, bool moving)
{
	IClientEntity* GetLocalPlayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	
	static int Ticks = 0;

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	static float hold = 0.f;
	Vector besthead;
	float opposite = 0.f;

	auto leyepos = hackManager.pLocal()->GetOrigin_likeajew() + hackManager.pLocal()->GetViewOffset();
	auto headpos = hitbox_location(GetLocalPlayer, 0);
	auto origin = hackManager.pLocal()->GetOrigin_likeajew();

	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->GetOrigin_likeajew() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, hackManager.pLocal());

		trace_t trace1, trace2;
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS | CONTENTS_GRATE */, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS | CONTENTS_GRATE */, &filter, &trace2);

		//	UTIL_TraceLine(data.src, End_Point, 0x4600400B, local, 0, &data.enter_trace);

		if (trace2.DidHit())
			endpos2 = trace2.endpos;

		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;

	};

	int index = c_helper->closest();
	static IClientEntity* entity;

	if (index != -1)
		entity = interfaces::ent_list->get_client_entity(index); // maybe?

	if (!entity->isValidPlayer())
	{
		cmd->viewangles.y -= 180.f;
		return;
	}

	float radius = Vector(headpos - origin).Length2D();

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float besthead = 0; besthead < 7; besthead += 0.1)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				opposite = besthead - 180;
				bestrotation = besthead;
			}
		}
	}
	float c = moving ? options::menu.misc.move_jitter.getvalue() : options::menu.misc.stand_jitter.getvalue();

	if (no_active)
	{
		cmd->viewangles.y -= 180.f + RandomFloat(-c, c);
	}

	else
	{
		jitter = !jitter;
		if ((RAD2DEG(bestrotation) < -45.f && RAD2DEG(bestrotation) > -150.f) || (RAD2DEG(bestrotation) > 210.f && RAD2DEG(bestrotation) < 315.f)) // your mom is a hoe
		{
			cmd->viewangles.y -= 130.f;
		}

		else if (RAD2DEG(bestrotation) < 150.f && RAD2DEG(bestrotation) > 45.f)
		{
			cmd->viewangles.y += 130.f;
		}

		else if (RAD2DEG(bestrotation) >= 150.f && RAD2DEG(bestrotation) <= 210.f)
		{
			float d = rand() % 100;
			if (d <= 15)
			{
				cmd->viewangles.y -= jitter ? 0 : 150.f + RandomFloat(c, -c);
			}
			else if (d >= 85)
			{
				cmd->viewangles.y += jitter ? 0 : 150.f + RandomFloat(c, -c); //
			}
			else
			{
				cmd->viewangles.y -= 180.f + RandomFloat(c, -c);
			}

			/*
				if (d <= 33)
			{
				cmd->viewangles.y -= jitter ? 0 : 150.f + RandomFloat(c, -c);
			}
			else if (d > 33 && d < 66)
			{
				cmd->viewangles.y += jitter ? 0 : 150.f + RandomFloat(c, -c); 
			}
			else
			{
				cmd->viewangles.y -= 180.f + RandomFloat(c, -c);
			}
			*/
		}

		else
		{
			cmd->viewangles.y = RAD2DEG(bestrotation);
		}
		
	}
}


float anti_aim::at_target() {
	auto cur_tar = -1;
	auto last_dist = FLT_MAX;

	auto local = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(
		interfaces::engine->GetLocalPlayer()));

	if (!local || !local->IsAlive())
		return 0.f;

	for (auto i = 0; i < interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<  IClientEntity* >(interfaces::ent_list->get_client_entity(i));

		if (!entity || entity == local || entity->is_dormant() || entity->team() == local->team())
			continue;

		auto cur_dist = (entity->m_VecORIGIN() - local->m_VecORIGIN()).Length();

		if (!cur_tar || cur_dist < last_dist) {
			cur_tar = i;
			last_dist = cur_dist;
		}
	}

	if (cur_tar) {
		auto entity = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(cur_tar));
		if (!entity) {
			return 180.f;
		}

		auto target_angle = CalcAngleA(local->m_VecORIGIN(), entity->m_VecORIGIN());
		return target_angle.y;
	}

	return 180.f;
}

