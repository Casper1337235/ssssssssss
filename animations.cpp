#include "animations.h"
#include "Hooks.h"

std::unique_ptr< c_animfix > animfix = std::make_unique< c_animfix >();

void c_animfix::re_work(ClientFrameStage_t stage) // i'm really out of motivation to try, so, pasted from a friend.
{
	auto local_player = hackManager.pLocal();
	if (!local_player || !local_player->get_animation_state())
		return;

	if (stage == ClientFrameStage_t::FRAME_RENDER_START)
		local_player->SetAbsAngles(Vector(0.f, c_beam->real, 0.f));


	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		IClientEntity * entity = (IClientEntity*)interfaces::ent_list->get_client_entity(i);

		if (!entity->isValidPlayer() || !entity->dormant())
			continue;

		static auto set_interpolation_flags = [](IClientEntity* e, int flag) {
			const auto var_map = (uintptr_t)e + 36;
			const auto sz_var_map = *(int*)(var_map + 20);

			for (auto index = 0; index < sz_var_map; index++)
				*(uintptr_t*)((*(uintptr_t*)var_map) + index * 12) = flag;
		};

		if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_END)
			set_interpolation_flags(entity, 0);

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

#define TICK_INTERVAL			(interfaces::globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

void c_animfix::fix_local_player_animations()
{
	auto local_player = hackManager.pLocal();
	if (!local_player || !local_player->IsAlive())
		return;

	static float sim_time;
	if (sim_time != local_player->get_simulation_time())
	{
		auto state = local_player->GetBasePlayerAnimState(); 
		if (!state) 
			return;

		const float curtime = interfaces::globals->curtime;
		const float frametime = interfaces::globals->frametime;
		const float realtime = interfaces::globals->realtime;
		const float absoluteframetime = interfaces::globals->absoluteframetime;
		const float absoluteframestarttimestddev = interfaces::globals->absoluteframestarttimestddev;
		const float interpolation_amount = interfaces::globals->interpolation_amount;
		const float framecount = interfaces::globals->framecount;
		const float tickcount = interfaces::globals->tickcount;
		auto old_fraction = local_player->GetBasePlayerAnimState()->m_flUnknownFraction = 0.f;
		static auto host_timescale = interfaces::cvar->FindVar(("host_timescale"));

		interfaces::globals->curtime = local_player->get_simulation_time();
		interfaces::globals->realtime = local_player->get_simulation_time();
		interfaces::globals->frametime = interfaces::globals->interval_per_tick * host_timescale->GetFloat();
		interfaces::globals->absoluteframetime = interfaces::globals->interval_per_tick * host_timescale->GetFloat();
		interfaces::globals->absoluteframestarttimestddev = local_player->get_simulation_time() - interfaces::globals->interval_per_tick * host_timescale->GetFloat();
		interfaces::globals->interpolation_amount = 0;
		interfaces::globals->framecount = TIME_TO_TICKS(local_player->get_simulation_time());
		interfaces::globals->tickcount = TIME_TO_TICKS(local_player->get_simulation_time());

		int backup_flags = local_player->GetFlags();
//		int backup_eflags = local_player->get_eflags();

		animationlayer backup_layers[15];
		std::memcpy(backup_layers, local_player->get_anim_overlays(), (sizeof(animationlayer) * 15));

		if (state->m_iLastClientSideAnimationUpdateFramecount == interfaces::globals->framecount)
			state->m_iLastClientSideAnimationUpdateFramecount = interfaces::globals->framecount - 1;

		local_player->client_side_animation() = true;
		local_player->update_clientside_animation();
		local_player->client_side_animation() = false;


		float lby_delta = local_player->get_lowerbody() - c_beam->real;
		lby_delta = std::remainderf(lby_delta, 360.f);
		lby_delta = clamp(lby_delta, -60.f, 60.f);

		float feet_yaw = std::remainderf(c_beam->real + lby_delta, 360.f);

		if (feet_yaw < 0.f) {
			feet_yaw += 360.f;
		}


		std::memcpy(local_player->get_anim_overlays(), backup_layers, (sizeof(animationlayer) * 15));

	//	animstate[local_player->GetIndex()] = local_player->get_animation_state();
		local_player->getFlags() = backup_flags;
	//	local_player->get_eflags() = backup_eflags;


		interfaces::globals->curtime = curtime;
		interfaces::globals->realtime = realtime;
		interfaces::globals->frametime = frametime;
		interfaces::globals->absoluteframetime = absoluteframetime;
		interfaces::globals->absoluteframestarttimestddev = absoluteframestarttimestddev;
		interfaces::globals->interpolation_amount = interpolation_amount;
		interfaces::globals->framecount = framecount;
		interfaces::globals->tickcount = tickcount;
		sim_time = local_player->get_simulation_time();

		local_player->set_abs_angles_v2(Vector(0.f, c_beam->real, 0.f));
		local_player->SetupBones(nullptr, -1, 0x7FF00, curtime);

		local_player->GetBasePlayerAnimState()->m_flUnknownFraction = old_fraction;

		local_player->SetAngle2(Vector(0.f, c_beam->real, 0.f));
	}
	local_player->invalidate_bone_cache();
	local_player->SetupBones(nullptr, -1, 0x7FF00, interfaces::globals->curtime); // local player getting BONED, GET IT? HAHA boned aaaaah i'm lonely
}
/*
void c_animfix::update_animations(IClientEntity* entity) 
{
	auto state = entity->get_animation_state(); 
	
	if (!state) 
		return;

	auto index = entity->GetIndex();
	static float sim_time[65];

	if (sim_time[index] != entity->get_simulation_time())
	{
		const float curtime = interfaces::globals->curtime;
		const float frametime = interfaces::globals->frametime;
		static auto host_timescale = interfaces::cvar->FindVar(("host_timescale"));

		interfaces::globals->frametime = interfaces::globals->interval_per_tick * host_timescale->get_float();
		interfaces::globals->curtime = entity->get_simulation_time() + interfaces::globals->interval_per_tick;

		Vector backup_velocity = entity->GetVelocity();

		int backup_flags = entity->GetFlags();

		animationlayer backup_layers[15];
		std::memcpy(backup_layers, entity->get_anim_overlays(), (sizeof(animationlayer) * 15));

		state->m_bOnGround ? entity->GetFlags() |= (1 << 0) : entity->GetFlags() &= ~(1 << 0);

		entity->get_abs_velocity() = entity->get_velocity();
		entity->get_abs_velocity2() = entity->get_velocity();
		entity->unkpasted() = 0x1;

		if (state->last_client_side_animation_update_framecount == interfaces::globals->frame_count)
			state->last_client_side_animation_update_framecount = interfaces::globals->frame_count - 1;

		entity->update_clientside_animation();

		float lby_delta = entity->get_lby() - entity->GetEyeAnglesXY()->y;
		lby_delta = std::remainderf(lby_delta, 360.f);
		lby_delta = std::clamp(lby_delta, -60.f, 60.f);

		float feet_yaw = std::remainderf(entity->GetEyeAnglesXY()->y + lby_delta, 360.f);

		if (feet_yaw < 0.f) {
			feet_yaw += 360.f;
		}

		static float pitch, yaw = 0.f;

		entity->get_anim_state()->goal_feet_yaw = entity->get_anim_state()->current_feet_yaw = feet_yaw;

		entity->get_anim_state()->eye_angles_y = entity->get_anim_state()->body_yaw = resolver->data[entity->get_index()].final_angle;

		std::memcpy(entity->get_anim_overlays(), backup_layers, (sizeof(animationlayer) * 15));

		entity->get_velocity() = backup_velocity;
		entity->get_flags() = backup_flags;

		interfaces::globals->curtime = curtime;
		interfaces::globals->frametime = frametime;
		sim_time[index] = entity->get_simulation_time();
	}

	entity->invalidate_bone_cache();
	entity->setup_bones(nullptr, -1, 0x7FF00, interfaces::globals->curtime);
}
*/