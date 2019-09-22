#include "Entities.h"
#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"
 float intervalPerTick;
#define TICK_INTERVAL			(  )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / intervalPerTick ) )


C_BaseCombatWeapon* IClientEntity::GetWeapon2()
{
	static int iOffset = GET_NETVAR("DT_BaseCombatCharacter", "m_hActiveWeapon");
	ULONG pWeepEhandle = *(PULONG)((DWORD)this + iOffset);
	return (C_BaseCombatWeapon*)(interfaces::ent_list->GetClientEntityFromHandle((HANDLE)pWeepEhandle));
}
const Vector& IClientEntity::world_space_center() {
	Vector vecOrigin = GetVecOrigin_2();

	Vector min = this->GetCollideable()->OBBMins() + vecOrigin;
	Vector max = this->GetCollideable()->OBBMaxs() + vecOrigin;

	Vector size = max - min;
	size /= 2.f;
	size += min;

	return size;
}
int IClientEntity::get_choked_ticks()
{
	if (!this || !this->IsAlive()) 
		return 0;

	float flSimulationTime = this->GetSimulationTime();
	float flOldSimulationTime = this->m_flOldSimulationTime();
	float flSimDiff = flSimulationTime - flOldSimulationTime;

	return TIME_TO_TICKS(max(0, flSimDiff /*- latency*/));
}

void IClientEntity::invalidate_bone_cache(void) 
{
	static auto invalidate_bone_bache_fn = game_utils::FindPattern1("client_panorama.dll", "80 3D ?? ?? ?? ?? ?? 74 16 A1 ?? ?? ?? ?? 48 C7 81");

	*(uintptr_t*)((uintptr_t)this + 0x2924) = 0xFF7FFFFF;
	*(uintptr_t*)((uintptr_t)this + 0x2690) = **(uintptr_t**)((uintptr_t)invalidate_bone_bache_fn + 10) - 1;
}

void IClientEntity::update_clientside_animation(void) {
	if (!this)
		return;

	static auto update_clientside_animation_fn = game_utils::FindPattern1("client_panorama.dll", "55 8B EC 51 56 8B F1 80 BE ? ? ? ? 00 74 ? 8B 06 FF");

	((void(__thiscall*)(void*)) update_clientside_animation_fn)(this);
}

bool IClientEntity::IsKnifeorNade()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetWeapon2();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_knife")
		return true;
	else if (WeaponName == "weapon_incgrenade")
		return true;
	else if (WeaponName == "weapon_decoy")
		return true;
	else if (WeaponName == "weapon_flashbang")
		return true;
	else if (WeaponName == "weapon_hegrenade")
		return true;
	else if (WeaponName == "weapon_smokegrenade")
		return true;
	else if (WeaponName == "weapon_molotov")
		return true;

	return false;
}

inline Vector ExtrapolateTick(Vector p0, Vector v0)
{
	return p0 + (v0 *intervalPerTick);
}
Vector IClientEntity::GetPredicted(Vector p0)
{
	return ExtrapolateTick(p0, this->getvelocity());
}
void IClientEntity::CopyPoseParameters(float* dest)
{
	float* flPose = (float*)((DWORD)this + 0x2764);
	memcpy(dest, flPose, sizeof(float) * 24);
}

void IClientEntity::CopyAnimLayers(CAnimationLayer* dest)
{
	int count = 15;
	for (int i = 0; i < count; i++)
	{
		dest[i] = GetAnimOverlayX(i);
	}
}

bool IClientEntity::has_gungame_immunity()
{
	static int m_bGunGameImmunity = GET_NETVAR("DT_CSPlayer", "m_bGunGameImmunity");
	return *(bool*)((DWORD)this + m_bGunGameImmunity);
}
void IClientEntity::pre_think() {
	typedef void(__thiscall *o_pregay)(void*);
	call_vfunc< o_pregay >(this, 312)(this);
}
bool IClientEntity::physics_run_think(int smthn)
{
	static auto impl_PhysicsRunThink = reinterpret_cast< bool(__thiscall *)(void*, int) >(
		game_utils::pattern_scan(GetModuleHandle("client_panorama.dll"),
			"55 8B EC 83 EC 10 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 16")
		);

	return impl_PhysicsRunThink(this, smthn);
}
void IClientEntity::think() {
	typedef void(__thiscall *o_pregay)(void*);
	call_vfunc< o_pregay >(this, 138)(this);
}
CBaseAnimState * IClientEntity::get_animation_state() 
{
	//	return *reinterpret_cast< CBaseAnimState ** >(reinterpret_cast< void * >(uintptr_t(this) + 0x3900));
	static auto animstate_offset = *(uintptr_t*)((uintptr_t)game_utils::FindPattern1("client_panorama.dll", "8B 8E ? ? ? ? F3 0F 10 48 04 E8 ? ? ? ? E9") + 0x2);
	return *(CBaseAnimState**)((uintptr_t)this + animstate_offset);
}