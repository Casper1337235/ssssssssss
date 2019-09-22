#include "EnginePrediction.h"
#include "NetVars.h"
#include "XorStr.hpp"
#include "MathFunctions.h"
#include "Hacks.h"
#include "EnginePrediction.h"
BYTE bMoveData[200];

/*
void CPredictionSystem::StartPrediction(CUserCmd* pCmd) 
{
	static int flTickBase;

	if (pCmd)
	{
		if (pCmd->hasbeenpredicted)
			flTickBase = hackManager.pLocal()->GetTickBase();
		else
			++flTickBase;
	}

	static bool bInit = false;
	if (!bInit) {
		m_pPredictionRandomSeed = *(int**)(Utilities::Memory::FindPatternV2("client_panorama.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		bInit = true;
	}

//	*m_pPredictionRandomSeed = MD5_PseudoRandom(Hacks.CurrentCmd->command_number) & 0x7FFFFFFF; 

	m_flOldCurtime = interfaces::globals->curtime;
	m_flOldFrametime = interfaces::globals->frametime;

	if (!interfaces::c_gamemove)
		return;

	interfaces::globals->curtime = flTickBase * interfaces::globals->interval_per_tick;
	interfaces::globals->frametime = interfaces::globals->interval_per_tick;

	interfaces::c_gamemove->StartTrackPredictionErrors(hackManager.pLocal());

	memset(&m_MoveData, 0, sizeof(m_MoveData));

	interfaces::movehelper->SetHost(hackManager.pLocal());
	interfaces::c_prediction->SetupMove(hackManager.pLocal(), pCmd, interfaces::movehelper, &m_MoveData);
	interfaces::c_gamemove->ProcessMovement(hackManager.pLocal(), &m_MoveData);
	interfaces::c_prediction->FinishMove(hackManager.pLocal(), pCmd, &m_MoveData);
}
void CPredictionSystem::EndPrediction(CUserCmd* pCmd) 
{
	if (!interfaces::c_gamemove)
		return;

	interfaces::c_gamemove->FinishTrackPredictionErrors(hackManager.pLocal());
	interfaces::movehelper->SetHost(0);

	*m_pPredictionRandomSeed = -1;

	interfaces::globals->curtime = m_flOldCurtime;
	interfaces::globals->frametime = m_flOldFrametime;
}

*/

/*
int CPredictionSystem::post_think(IClientEntity* player) const 
{
	
	using gay_t = bool(__thiscall*)(IClientEntity*);
	using gay_2 = void(__thiscall*)(IClientEntity*);

	static auto PostThinkVPhysics = (gay_t)game_utils::pattern_scan(
		GetModuleHandle("client_panorama.dll"), "55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 53 8B D9");
	static auto SimulatePlayerSimulatedEntities = (gay_2)game_utils::pattern_scan(
		GetModuleHandle("client_panorama.dll"), "56 8B F1 57 8B BE ?? ?? ?? ?? 83 EF 01");

	call_vfunc< void(__thiscall *)(void*) >(interfaces::model_cache, 33)(
		interfaces::model_cache);
	if (player->IsAlive()) {
		call_vfunc< void(__thiscall *)(void*) >(player, 334)(player);

		if (player->GetFlags() & FL_ONGROUND)
			*reinterpret_cast< uintptr_t * >(uintptr_t(player) + 0x3014) = 0;

		if (*reinterpret_cast< int * >(uintptr_t(player) + 0x28BC) == -1)
			call_vfunc< void(__thiscall *)(void*, int) >(player, 214)(player, 0);

		call_vfunc< void(__thiscall *)(void*) >(player, 215)(player);

		PostThinkVPhysics(player);
	}
	SimulatePlayerSimulatedEntities(player);

	return call_vfunc< int(__thiscall *)(void*) >(interfaces::model_cache, 34)(
		interfaces::model_cache);
		
}
*/

int *m_nPredictionRandomSeed;
int *m_pSetPredictionPlayer;

float flOldCurtime;
float flOldFrametime;
void CPredictionSystem::StartPrediction(CUserCmd* pCmd) noexcept
{
	static int flTickBase;

	auto player = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));

	if (!player)
		return;

	if (!pCmd || !interfaces::movehelper || !player->IsAlive())
		return;

	static CUserCmd* pLastCmd;
	
	if (pCmd)
	{
		if (pCmd->hasbeenpredicted)
			flTickBase = player->GetTickBase();
		else
			++flTickBase;
	}

	static bool bInit = false; // :b:
/*	if (!m_nPredictionRandomSeed )//|| !m_pSetPredictionPlayer)
	{
		m_nPredictionRandomSeed = *(int**)(game_utils::pattern_scan("client_panorama.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
	//	m_pSetPredictionPlayer = *reinterpret_cast<int**>(Utilities::Memory::FindPatternV2("client_panorama.dll", "89 35 ? ? ? ? F3 0F 10 46") + 2); who cares
		bInit = true;
	} */

	const auto getRandomSeed = [&]() {

		using MD5_PseudoRandomFn = unsigned long(__cdecl*)(std::uintptr_t);
		static auto offset = game_utils::FindPattern1("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 70 6A 58");
		static auto MD5_PseudoRandom = reinterpret_cast<MD5_PseudoRandomFn>(offset);
		return MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;
	};
	CMoveData data;
// 	memset(&data, 0, sizeof(CMoveData));
//	player->SetCurrentCommand(pCmd); // yo would this help? No legit, i am a bit clueless. I think it does, but if you know for sure, tag me at discord	

//	*m_nPredictionRandomSeed = pCmd->random_seed;
//	*m_pSetPredictionPlayer = uintptr_t(player);

//	*reinterpret_cast< uint32_t* >(reinterpret_cast< uint32_t >(player) + 0x3314) = reinterpret_cast< uint32_t >(pCmd);
//	*reinterpret_cast< uint32_t* >(reinterpret_cast< uint32_t >(player) + 0x326C) = reinterpret_cast< uint32_t >(pCmd);

	flOldCurtime = interfaces::globals->curtime;
	flOldFrametime = interfaces::globals->frametime;

	pCmd->random_seed = getRandomSeed();
	interfaces::globals->curtime = flTickBase * interfaces::globals->interval_per_tick;
	interfaces::globals->frametime = interfaces::globals->interval_per_tick;

	pLastCmd = pCmd;
//	flOldCurtime = interfaces::globals->curtime;
//	flOldFrametime = interfaces::globals->frametime;

//	interfaces::globals->tickcount = player->GetTickBase();

//	if (pCmd->impulse)
//		*reinterpret_cast< uint32_t * >(uint32_t(player) + 0x31FC) = pCmd->impulse;

	interfaces::c_gamemove->StartTrackPredictionErrors(player);

	memset(&move_data, 0, sizeof(move_data));
	interfaces::movehelper->SetHost(player);
	interfaces::c_prediction->SetupMove(player, pCmd, interfaces::movehelper, &move_data);
	interfaces::c_gamemove->ProcessMovement(player, &move_data);
	interfaces::c_prediction->FinishMove(player, pCmd, &move_data);

	
	/* --- everything below this is from my friend, some unfinished. it's actually G but i think it's a bit much for me --- */

//	if (player->GetWeapon2())
//		player->GetWeapon2()->UpdateAccPenalty();

	/*
	//	*m_pPredictionRandomSeed = MD5_PseudoRandom(Hacks.CurrentCmd->command_number) & 0x7FFFFFFF;

	m_flOldCurtime = interfaces::globals->curtime;
	m_flOldFrametime = interfaces::globals->frametime;

	if (!interfaces::c_gamemove)
	return;

	interfaces::globals->curtime = flTickBase * interfaces::globals->interval_per_tick;
	interfaces::globals->frametime = interfaces::globals->interval_per_tick;

	interfaces::c_gamemove->StartTrackPredictionErrors(hackManager.pLocal());

	memset(&m_MoveData, 0, sizeof(m_MoveData));

	interfaces::movehelper->SetHost(hackManager.pLocal());
	interfaces::c_prediction->SetupMove(hackManager.pLocal(), pCmd, interfaces::movehelper, &m_MoveData);
	interfaces::c_gamemove->ProcessMovement(hackManager.pLocal(), &m_MoveData);
	interfaces::c_prediction->FinishMove(hackManager.pLocal(), pCmd, &m_MoveData)



	if (!m_movedata_)
		m_movedata_ = malloc(182);

	if (!m_prediction_player_ || !m_prediction_seed_) {
		m_prediction_seed_ = *reinterpret_cast< int** >(game_utils::pattern_scan(
			GetModuleHandleA("client_panorama.dll"), "A3 ? ? ? ? 66 0F 6E 86") + 1);
		m_prediction_player_ = *reinterpret_cast< int** >(game_utils::pattern_scan(
			GetModuleHandleA("client_panorama.dll"), "89 35 ? ? ? ? F3 0F 10 48 20") + 2);
	}

	//	CPrediction::StartCommand
	{
		*reinterpret_cast< int * >(m_prediction_seed_) = pCmd ? pCmd->random_seed : -1;
		*reinterpret_cast< int * >(m_prediction_player_) = reinterpret_cast< int >(player);

		*reinterpret_cast< CUserCmd ** >(uint32_t(player) + 0x3334) = pCmd; // m_pCurrentCommand
		*reinterpret_cast< CUserCmd ** >(uint32_t(player) + 0x3288) = pCmd; // unk01
	}

	//	backup player variables
	m_old_player_.flags = player->GetFlags();
	m_old_player_.velocity = player->GetVelocity();

	//	backup globals
	m_old_globals_.curtime = interfaces::globals->curtime;
	m_old_globals_.frametime = interfaces::globals->frametime;
	m_old_globals_.tickcount = interfaces::globals->tickcount;

	//	backup tick base
	const int old_tickbase = player->m_nTickBase();

	//	backup prediction variables
	const bool old_in_prediction = interfaces::c_prediction->m_bInPrediction;
	const bool old_first_prediction = interfaces::c_prediction->m_bIsFirstTimePredicted;

	//	set globals correctly
	interfaces::globals->curtime = player->m_nTickBase() * interfaces::globals->interval_per_tick;
	interfaces::globals->frametime = interfaces::c_prediction->m_bEnginePaused ? 0 : interfaces::globals->interval_per_tick;
	interfaces::globals->tickcount = player->m_nTickBase();

	//	setup prediction
	interfaces::c_prediction->m_bIsFirstTimePredicted = false;
	interfaces::c_prediction->m_bInPrediction = true;

	interfaces::c_gamemove->StartTrackPredictionErrors(player);

	////	weapon selection, credits gavreel: https://www.unknowncheats.me/forum/garry-s-mod/267731-engine-prediction.html
	//{
	//    if ( cmd->weaponselect != 0 ) {
	//        auto weapon = reinterpret_cast< c_base_combat_weapon * >(interfaces::client_entity_list->get_client_entity(
	//            cmd->weaponselect ));
	//        if ( weapon ) {
	//            player->select_item( weapon->get_weapon_info( )->weapon_name, cmd->weaponsubtype );
	//        }
	//    }
	//}

	if (pCmd->impulse)
		*reinterpret_cast< uint32_t * >(uint32_t(player) + 0x31FC) = pCmd->impulse;

	//	CBasePlayer::UpdateButtonState
	{
		pCmd->buttons |= *reinterpret_cast< uint32_t * >(uint32_t(player) + 0x3330);

		const int v16 = pCmd->buttons;
		int* unk02 = reinterpret_cast< int * >(uint32_t(player) + 0x31F8);
		const int v17 = v16 ^ *unk02;

		*reinterpret_cast< int * >(uint32_t(player) + 0x31EC) = *unk02;
		*reinterpret_cast< int * >(uint32_t(player) + 0x31F8) = v16;
		*reinterpret_cast< int * >(uint32_t(player) + 0x31F0) = v16 & v17;
		*reinterpret_cast< int * >(uint32_t(player) + 0x31F4) = v17 & ~v16;
	}

	//	check if player is standing on moving ground
	interfaces::c_prediction->CheckMovingGround(player, interfaces::globals->framecount);

	//	copy from command to player
	player->set_local_view_angles(pCmd->viewangles);

	//	CPrediction::RunPreThink
	{
		//	THINK_FIRE_ALL_FUNCTIONS
		if (player->physics_run_think(0)) {
			player->pre_think();
		}
	}

	//	CPrediction::RunThink
	 /
	 +
		const auto next_think = reinterpret_cast< int * >(uint32_t(player) + 0xFC);
		if (*next_think > 0 && *next_think <= player->GetTickBase()) {
			//	TICK_NEVER_THINK
			*next_think = -1;

			player->think();
		}
	}

	//	set host
	interfaces::movehelper->SetHost(player);

	//	setup input
	interfaces::c_prediction->SetupMove(player, pCmd, interfaces::movehelper, m_movedata_);

	//	run movement
	interfaces::gamemovement->ProcessMovement(player, m_movedata_);

	//	finish prediction
	interfaces::c_prediction->FinishMove(player, pCmd, m_movedata_);

	//	invoke impact functions
	interfaces::movehelper->process_impacts();

	//	CPrediction::RunPostThink
	{
	//	post_think(player);
	}

	//	restore tickbase
	player->m_nTickBase() = old_tickbase;

	//	restore prediction
	interfaces::c_prediction->m_bIsFirstTimePredicted = old_first_prediction;
	interfaces::c_prediction->m_bInPrediction = old_in_prediction;
	*/
}
void CPredictionSystem::EndPrediction(CUserCmd* pCmd) noexcept 
{
	auto player = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));

	if (!player)
		return;

	interfaces::c_gamemove->FinishTrackPredictionErrors(player);
	interfaces::movehelper->SetHost( nullptr );

//	if (m_nPredictionRandomSeed )
//	{
//		*m_nPredictionRandomSeed = -1;
//	}

	interfaces::globals->curtime = flOldCurtime;
	interfaces::globals->frametime = flOldFrametime;

	interfaces::c_gamemove->Reset();
}


