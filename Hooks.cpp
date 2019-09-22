 #include "global_count.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "circlestrafer.h"
#include "CBulletListener.h"
#include "Interfaces.h"
#include "Skinchanger.h"
#include "autodefuse.h"
#include "RenderManager.h"
#include "lodepng.h"
#include "knifebot.h"
#include "Visuals.h"
#include <d3d9.h>
#include "EnginePrediction.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "hitmarker.h"
#include "laggycompensation.h"
#include <intrin.h>
#include "DamageIndicator.h"
#include "RageBot.h"
#include "LagCompensation2.h"
#include "position_adjust.h"
#include "EnginePrediction.h"
#include "lin_extp.h"
#include "radar.h"
#include "fakelag.h"
#include "experimental.h"
#include "killsay.h"
#include "backdrop.h"

#include "animations.h"
std::vector<impact_info> impacts;
std::vector<hitmarker_info> Xhitmarkers;
static CPredictionSystem* Prediction = new CPredictionSystem();
beam * c_beam = new beam();
backup_visuals * c_visuals = new backup_visuals();
CLagcompensation lagcompensation;
HANDLE worldmodel_handle;
C_BaseCombatWeapon* worldmodel;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif
#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif
Vector Globals::aim_point;
int bigboi::indicator;
int bigboi::desync_manual;
bool bigboi::freestand;
bool round_change;
int bigboi::freestandval;
std::vector<trace_info> trace_logs;
int currentfov;
Vector LastAngleAA;
extern Vector LastAngleAA2;
Vector LastAngleAAFake;
Vector last_fake;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
bool Resolver::hitbaim;
bool Globals::Up2date;
int Globals::fired[65];
int Globals::hit[65];
extern float lineLBY;
extern float lineLBY2;

extern float current_desync;
extern float lineRealAngle;
//extern float lineFakeAngle;
extern float last_real;
extern float lspeed;
extern float pitchmeme;
extern float lby2;
extern float inaccuracy;

static bool fuckingcheck;

Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;

float bigboi::current_yaw;
#define STUDIO_RENDER					0x00000001
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots[65];
static int missedLogHits[65];
float fakeangle;
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef long(__stdcall *EndScene_t)(IDirect3DDevice9*);
typedef int(__thiscall* DoPostScreenEffects_t)(IClientModeShared*, int);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef long(__stdcall *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);

//... new hooks ...//
class CBoneBitList;
typedef void(__thiscall* ExtraBoneProcess_t) (void*, void*, void*, void*, void*, CBoneBitList&, void*);
typedef void(__thiscall* TraceRayFn)(void *thisptr, const Ray_t &ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t *pTrace);

typedef bool(__thiscall* SetupBonesFn)(void*, matrix3x4_t*, int, int, float);

using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);

typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pEcx);
typedef void(__thiscall* ExtraBoneProcessing)(void*);

EndScene_t o_EndScene;
SceneEnd_t pSceneEnd;
Reset_t o_Reset;
DoPostScreenEffects_t o_DoPostScreenEffects;
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;
ExtraBoneProcessing oDoExtraBonesProcessing;
SetupBonesFn original_setup_bones;

TraceRayFn original_trace_ray;

void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice);
int __stdcall Hooked_DoPostScreenEffects(int a1);
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);

typedef void(__thiscall* LockCursor)(void*);

LockCursor oLockCursor;

/*
void __fastcall DoExtraBonesProcessing(void * ECX, void * EDX, void * unkn1, void * unkn2, void * unkn3, void * unkn4, CBoneBitList & unkn5, void * unkn6)
{
	IClientEntity* pPlayerEntity = (IClientEntity*)ECX;

	if (!pPlayerEntity || pPlayerEntity == nullptr)
		return;

	if (!pPlayerEntity->IsAlive() || pPlayerEntity->IsDormant() || pPlayerEntity->team() == hackManager.pLocal()->team())
		return;

	if (!pPlayerEntity->GetBasePlayerAnimState())
		return;

	auto oDoExtraBonesProcessing = Hooks::VMTRenderView.GetMethod<ExtraBoneProcess_t>(192);

	// Hooks::VMTRenderView.GetMethod<SceneEnd_t>(9)(pEcx);

	float Backup = pPlayerEntity->GetBasePlayerAnimState()->m_flUnknownFraction;
	pPlayerEntity->GetBasePlayerAnimState()->m_flUnknownFraction = 0;

	oDoExtraBonesProcessing(ECX, unkn1, unkn2, unkn3, unkn4, unkn5, unkn6);

	pPlayerEntity->GetBasePlayerAnimState()->m_flUnknownFraction = Backup;
}


void __fastcall HookedTraceRay(void *thisptr, void*, const Ray_t &ray, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace)
{
	original_trace_ray(thisptr, ray, fMask, pTraceFilter, pTrace);
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
		pTrace->surface.flags |= SURF_SKY;
}

bool in_setup_bones[64] = { false };
bool __fastcall setup_bones_hk(void* ecx, void* edx, matrix3x4_t* a1, int a2, int a3, float a4) {

	auto entity = reinterpret_cast< IClientEntity* >((std::uintptr_t)ecx - 0x4);

	if (!entity)
		return false;

	if (!in_setup_bones[entity->GetIndex()])
		return false;

	return false;//  ( ( bool( __thiscall* )( void*, matrix3x4_t*, int, int, float ) ) o_setup_bones )( null );ret;
}

*/


void	__stdcall Hooked_LockCursor()
{
	bool xd = options::menu.m_bIsOpen;
	if (xd) {
		interfaces::surface->unlockcursor();
		return;
	}
	oLockCursor(interfaces::surface);
}
namespace GlobalBREAK
{
	bool bVisualAimbotting = false;
	QAngle vecVisualAimbotAngs = QAngle(0.f, 0.f, 0.f);
	int ChokeAmount = 0;
	float flFakewalked = 0.f;
	bool NewRound = false;
	bool WeaponFire = false;
	QAngle fakeangleslocal;
	bool bRainbowCross = true;
	bool dohitmarker;
	float LastTimeWeFired = 0;
	int ShotsFiredLocally = 0;
	int ShotsHitPerEntity[65];
	bool HeadShottedEntity[65] = { false };
	float curFov = 0;
	bool bUsingFakeAngles[65];
	float HitMarkerAlpha = 0.f;
	int TicksOnGround = 0;
	int ticks_while_unducked = 0;
	char* breakmode;
	int AnimationPitchFix = 0;
	float hitchance;
	int NextPredictedLBYUpdate = 0;
	int breakangle;
	int prevChoked = 0;
	bool AAFlip = false;
	bool LEFT;
	bool RIGHT;
	bool BACK;
	char my_documents_folder[MAX_PATH];
	float smt = 0.f;
	QAngle visualAngles = QAngle(0.f, 0.f, 0.f);
	bool bSendPacket = false;
	bool bAimbotting = false;
	CUserCmd* userCMD = nullptr;
	char* szLastFunction = "<No function was called>";
	HMODULE hmDll = nullptr;
	bool bFakewalking = false;
	Vector vecUnpredictedVel = Vector(0, 0, 0);
	float flFakeLatencyAmount = 0.f;
	float flEstFakeLatencyOnServer = 0.f;
	matrix3x4_t traceHitboxbones[128];
	std::array<std::string, 64> resolverModes;
	bool send_packet;
}
int ground_tick;
Vector OldOrigin;
namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel;
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender;
	Utilities::Memory::VMTManager VMTPrediction;
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
	Utilities::Memory::VMTManager VMTDIRECTX;
	Utilities::Memory::VMTManager VMTSurface;
	Utilities::Memory::VMTManager VMTSetupBones;
	RecvVarProxyFn g_fnSequenceProxyFn = NULL;

};
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
	VMTDIRECTX.RestoreOriginal();
	VMTEventManager.RestoreOriginal();
	VMTSurface.RestoreOriginal();
	VMTRenderView.RestoreOriginal();
	VMTClient.RestoreOriginal();
}

bool __fastcall HookedSetupBones(void* ECX, void* EDX, matrix3x4_t* matrix, int num_bones, int bone_mask, float current_time)
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto animstate = local_player->GetBasePlayerAnimState();
	if (!ECX || !matrix || !local_player || local_player->GetHealth() <= 0 || !animstate)
		return original_setup_bones(ECX, matrix, num_bones, bone_mask, current_time);

	static auto last_networked_origin = Vector(0, 0, 0);
	if (GlobalBREAK::bSendPacket)
		last_networked_origin = local_player->GetVecOrigin_2();

//	local_player->SetAbsOriginal(last_networked_origin);
	return original_setup_bones(ECX, matrix, num_bones, bone_mask, current_time);
}

void setup_bones()
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local_player || local_player->GetHealth() <= 0) return;

	auto renderable = reinterpret_cast<void*>(uintptr_t(local_player) + 0x4);
	if (!renderable) return;

	static bool is_init = false;
	if (!is_init)
	{
		is_init = true;
		Hooks::VMTSetupBones.Init(renderable);
		original_setup_bones = Hooks::VMTSetupBones.HookFunction<SetupBonesFn>(13, HookedSetupBones);

		interfaces::cvar->ConsoleColorPrintf(Color(20, 200, 250, 255), " [debug] setupbones has been hooked     \n");
	}
	if (!Hooks::VMTSetupBones.IsHooked(13))
		original_setup_bones = Hooks::VMTSetupBones.HookFunction<SetupBonesFn>(13, HookedSetupBones);
}

void Hooks::Initialise()
{
	interfaces::engine->ExecuteClientCmd("clear");
	//--------------- D3D ---------------//
	VMTDIRECTX.Initialise((DWORD*)interfaces::g_pD3DDevice9);
	o_EndScene = (EndScene_t)VMTDIRECTX.HookMethod((DWORD)&EndScene_hooked, 42);
	VMTDIRECTX.Initialise((DWORD*)interfaces::g_pD3DDevice9);
	o_Reset = (Reset_t)VMTDIRECTX.HookMethod((DWORD)&Reset_hooked, 16);
	//--------------- NORMAL HOOKS ---------------//
	VMTClientMode.Initialise((DWORD*)interfaces::ClientMode);
	o_DoPostScreenEffects = (DoPostScreenEffects_t)VMTClientMode.HookMethod((DWORD)Hooked_DoPostScreenEffects, 44);
	VMTPanel.Initialise((DWORD*)interfaces::panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	VMTPrediction.Initialise((DWORD*)interfaces::prediction_dword);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	VMTModelRender.Initialise((DWORD*)interfaces::model_render);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	VMTClientMode.Initialise((DWORD*)interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);
	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);
	VMTClient.Initialise((DWORD*)interfaces::client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 37);
	VMTEventManager.Initialise((DWORD*)interfaces::event_manager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);
	VMTRenderView.Initialise((DWORD*)interfaces::render_view);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);
	VMTSurface.Initialise((DWORD*)interfaces::surface);
	oLockCursor = (LockCursor)VMTSurface.HookMethod((DWORD)Hooked_LockCursor, 67);

//	original_setup_bones = (SetupBonesFn)VMTSetupBones.HookMethod((DWORD)&HookedSetupBones, 13);

//	oDoExtraBonesProcessing = (ExtraBoneProcessing)VMTClientMode.HookMethod((DWORD)DoExtraBonesProcessing, 192);

//	original_trace_ray = (TraceRayFn)VMTSurface.HookMethod((DWORD)HookedTraceRay, 5);

//	o_setup_bones = (SetupBones)VMTClientMode.HookMethod((DWORD)setup_bones_hk, 13);

	for (ClientClass* pClass = interfaces::client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
	{
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) {
			RecvTable* pClassTable = pClass->m_pRecvTable;
			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) {
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];
				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;
				// Store the original proxy function.
				Hooks::g_fnSequenceProxyFn = (RecvVarProxyFn)pProp->m_ProxyFn;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = Hooks::SetViewModelSequence;

				break;
			}

			break;
		}
	}
	ConVar* nameVar = interfaces::cvar->FindVar("name");
	//--------------- NAME CVAR ---------------//
	if (nameVar)
	{
		*(int*)((DWORD)&nameVar->fnChangeCallback + 0xC) = 0;
	}
	//--------------- EVENT LOG ---------------//
	static auto y = interfaces::cvar->FindVar("sv_showanimstate"); //this probably isn't avaible in modern source
	y->SetValue(1);
	static auto developer = interfaces::cvar->FindVar("developer");
	developer->SetValue(1);
//	static auto con_filter_text_out = interfaces::cvar->FindVar("con_filter_text_out");
	static auto con_filter_enable = interfaces::cvar->FindVar("con_filter_enable");
	static auto con_filter_text = interfaces::cvar->FindVar("con_filter_text");
	static auto dogstfu = interfaces::cvar->FindVar("con_notifytime");
	dogstfu->SetValue(3);
	con_filter_text->SetValue(".     ");
//	con_filter_text_out->SetValue("");
	con_filter_enable->SetValue(2);

}


AnimatedClanTag *animatedClanTag = new AnimatedClanTag();

int __stdcall Hooked_DoPostScreenEffects(int a1)
{
	auto m_local = hackManager.pLocal();

	for (auto i = 0; i < interfaces::glow_manager->size; i++)
	{
		auto glow_object = &interfaces::glow_manager->m_GlowObjectDefinitions[i];
		IClientEntity *m_entity = glow_object->m_pEntity;

		if (!glow_object->m_pEntity || glow_object->IsUnused() || !m_local)
			continue;
		if (strstr(m_entity->GetClientClass()->m_pNetworkName, "Weapon"))
		{
			if (options::menu.visuals.OtherEntityGlow.getstate())
			{
				float m_flRed = options::menu.ColorsTab.GlowOtherEnt.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowOtherEnt.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowOtherEnt.GetValue()[2];
				
				glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
				glow_object->m_flGlowAlpha = 1.f;
				glow_object->m_bRenderWhenOccluded = true;
				glow_object->m_bRenderWhenUnoccluded = false;

				c_beam->glow = true;
			}
		}

		if (m_entity->isValidPlayer() && m_entity->cs_player())
		{
			if (m_entity == m_local && options::menu.visuals.Glowz_lcl.getvalue() > 0)
			{
				if (m_local->IsAlive() && options::menu.visuals.localmaterial.getindex() < 6)
				{
					float m_flRed = options::menu.ColorsTab.GlowLocal.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowLocal.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowLocal.GetValue()[2];
					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.Glowz_lcl.getvalue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;

				}

			}

			if (options::menu.visuals.GlowZ.getvalue() > 0 && m_entity->team() != m_local->team())
			{	
					float m_flRed = options::menu.ColorsTab.GlowEnemy.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowEnemy.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowEnemy.GetValue()[2];

					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.GlowZ.getvalue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
					//	glow_object->m_bPulsatingChams = 2;		
			}
			if (options::menu.visuals.team_glow.getvalue() > 0)
			{
				if (m_entity->team() == m_local->team() && m_entity != m_local)
				{
					float m_flRed = options::menu.ColorsTab.GlowTeam.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowTeam.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowTeam.GetValue()[2];

					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.team_glow.getvalue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
					//	glow_object->m_bPulsatingChams = 1;
				}
			}
		}
	}
	return o_DoPostScreenEffects(interfaces::ClientMode, a1);
}
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	auto hr = o_Reset(pDevice, pPresentationParameters);
	if (hr >= 0)
	{
		bool gey;
		gey = true;
	}
	return hr;
}
struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};
#define M_PI 3.14159265358979323846
void CircleFilledRainbowColor(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color::FromHSB(hue, 1.f, 1.f);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
#define M_PI 3.14159265358979323846


void greyone(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color(15, 15, 15);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
void colorboy69(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = (Color)options::menu.ColorsTab.spreadcrosscol.GetValue();
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice)
{
	//this will probably get drawn even over the console and other CSGO hud elements, but whatever
	//this will also draw over the menu so we should disable it if the menu is open
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto g_LocalPlayer = pLocal;
	if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_LocalPlayer->GetWeapon2()) {
		int w, h;
		interfaces::engine->GetScreenSize(w, h); w /= 2; h /= 2;
		if (interfaces::engine->IsInGame() && interfaces::engine->IsConnected())
		{
			int w, h;
			static float rot = 0.f;
			interfaces::engine->GetScreenSize(w, h); w /= 2; h /= 2;
			C_BaseCombatWeapon* pWeapon = g_LocalPlayer->GetWeapon2();
			if (pWeapon)
			{
				short Index = (int)pWeapon->GetItemDefinitionIndex();
				
				if (g_LocalPlayer && Index != 42 && Index != 59 && Index != 500)
				{
					if (options::menu.visuals.SpreadCrosshair.getindex() != 0 && !options::menu.m_bIsOpen)
					{
						auto accuracy = pWeapon->GetInaccuracy() * (90 * 6.5);

						switch (options::menu.visuals.SpreadCrosshair.getindex())
						{
						case 1:
						{
							greyone(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 2:
						{
							colorboy69(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 3:
						{
							CircleFilledRainbowColor(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 4:
						{
							CircleFilledRainbowColor(w, h, accuracy, rot, 1, 50, pDevice);
						}
						break;
						}

						rot += 1.f;
						if (rot > 360.f)
							rot = 0.f;
					}
				}
			}
		}
	}


	return o_EndScene(pDevice);
}
void MovementCorrection(CUserCmd* userCMD, IClientEntity * local)
{
	if (!local)
		return;

	if (userCMD->forwardmove) {
		userCMD->buttons &= ~(userCMD->forwardmove < 0 ? IN_FORWARD : IN_BACK);
		userCMD->buttons |= (userCMD->forwardmove > 0 ? IN_FORWARD : IN_BACK);
	}
	if (userCMD->sidemove) {
		userCMD->buttons &= ~(userCMD->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
		userCMD->buttons |= (userCMD->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
	}

}
float clip(float n, float lower, float upper)
{
	return (std::max)(lower, (std::min)(n, upper));
}
int kek = 0;
int autism = 0;
int speed = 0;
static float testtimeToTick;
static float testServerTick;
static float testTickCount64 = 1;

float NormalizeYaw(float value)
{
	while (value > 180)
		value -= 360.f;
	while (value < -180)
		value += 360.f;
	return value;
}
float random_float(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(("vstdlib.dll")), ("RandomFloat"));
	return m_RandomFloat(min, max);
}
LinearExtrapolations linear_extraps;
std::string Tag = " mirror";
std::string Tag2 = "";
void set_clan_tag(const char* tag, const char* clan_name)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(Utilities::Memory::FindPatternV2(XorStr("engine.dll"), XorStr("53 56 57 8B DA 8B F9 FF 15")));
	pSetClanTag(tag, clan_name);
}
void clan_changer()
{
	auto m_local = hackManager.pLocal();
	bool OOF = false;
	if (options::menu.ColorsTab.ClanTag.getstate())
	{
		if (!m_local || !m_local->IsAlive() || !interfaces::engine->IsInGame() || !interfaces::engine->connected())
		{
			if (!OOF)
			{
				Tag2 += Tag.at(0);
				Tag2.erase(0, 1);
				set_clan_tag(Tag2.c_str(), "mirror v6");
			}
			else
			{
				OOF = true;
			}
		}
		static size_t lastTime = 0;

		if (GetTickCount() > lastTime)
		{
			OOF = false;
			Tag += Tag.at(0);
			Tag.erase(0, 1);
			set_clan_tag(Tag.c_str(), "mirror v6");
			lastTime = GetTickCount() + 650;
		}
	}
}
struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};
std::deque<CIncomingSequence> sequences;
int32_t lastincomingsequencenumber;


bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	uintptr_t* FPointer; __asm { MOV FPointer, EBP }
	byte* SendPacket = (byte*)(*FPointer - 0x1C);
	GlobalBREAK::bSendPacket = *SendPacket;
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
	//	static bool boost_fps = false;
	//	static bool abc = false;
		
		c_misc->colour_modulation();

		pCmd->sidemove = pCmd->sidemove;
		pCmd->upmove = pCmd->upmove;
		pCmd->forwardmove = pCmd->forwardmove;

		GlobalBREAK::bSendPacket = *SendPacket;
		if (GetAsyncKeyState(options::menu.misc.manualleft.GetKey()))
		{
			bigboi::indicator = 1;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualright.GetKey()))
		{
			bigboi::indicator = 2;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualback.GetKey()))
		{
			bigboi::indicator = 3;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualfront.GetKey()))
		{
			bigboi::indicator = 4;
		}

		if (GetAsyncKeyState(options::menu.misc.manual_desync_left.GetKey()))
		{
			bigboi::desync_manual = 1;
		}
		else if(GetAsyncKeyState(options::menu.misc.manual_desync_right.GetKey()))
		{
			bigboi::desync_manual = 2;
		}

		defususmaximus(pCmd);

		GlobalBREAK::smt = frametime;
		GlobalBREAK::userCMD = pCmd;
		GlobalBREAK::vecUnpredictedVel = pLocal->getvelocity();

		clan_changer();

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;
		uintptr_t* framePtr;
		__asm mov framePtr, ebp;
		GlobalBREAK::bSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);

		if (pLocal->GetFlags() & FL_ONGROUND)
			GlobalBREAK::TicksOnGround++;
		else
			GlobalBREAK::TicksOnGround = 0;

		if (pLocal->GetFlags() & FL_DUCKING)
			GlobalBREAK::ticks_while_unducked = 0;
		else
			GlobalBREAK::ticks_while_unducked++;

		if (GlobalBREAK::bSendPacket)
			GlobalBREAK::prevChoked = interfaces::client_state->chokedcommands;
		if (!GlobalBREAK::bSendPacket)
			GlobalBREAK::visualAngles = QAngle(pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z);
		if (GlobalBREAK::TicksOnGround == 1)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 1 && pLocal->getFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 0 && pLocal->GetFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;

		globalsh.bSendPaket = true;

		if (options::menu.misc.FakeLagChoke.getvalue() > 0 || options::menu.misc.FakeLagChoke2.getvalue() > 0)
			globalsh.bSendPaket = false;
		if (interfaces::client_state->chokedcommands > 14 || (interfaces::client_state->chokedcommands == globalsh.ChokeAmount &&
			(options::menu.misc.FakeLagChoke.getvalue() > 0 || options::menu.misc.FakeLagChoke2.getvalue() > 0)))
			globalsh.bSendPaket = false;		globalsh.bSendPaket = (bool*)(*(DWORD*)pebp - 0x1C);
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		if (globalsh.bSendPaket)
			globalsh.prevChoked = interfaces::client_state->chokedcommands;

		IClientEntity* pEntity;
	//	Vector ClientAngles; interfaces::engine->get_viewangles(ClientAngles);

		if (options::menu.misc.SniperCrosshair.getstate() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pLocal->GetWeapon2() && pLocal->GetWeapon2()->m_bIsSniper()) {
				ConVar* cross = interfaces::cvar->FindVar("weapon_debug_spread_show");
				cross->nFlags &= ~FCVAR_CHEAT;
				cross->SetValue(3);
			}
		}
		else {
			ConVar* cross = interfaces::cvar->FindVar("weapon_debug_spread_show");
			cross->nFlags &= ~FCVAR_CHEAT;
			cross->SetValue(0);
		}
		if (pLocal) // isconnected and is ingame original check location
		{

			if (pLocal->GetFlags() & FL_ONGROUND && !(CMBacktracking::Get().current_record->m_nFlags & FL_ONGROUND)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = true;
			}
			if (pLocal->GetFlags() & FL_ONGROUND && interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			if (!pLocal->GetFlags() & FL_ONGROUND && !interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			if (interfaces::m_iInputSys->IsButtonDown(MOUSE_LEFT)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			for (int i = 1; i < 2; i++)
			{
				if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][i].flags)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}

			if (options::menu.misc.OtherAutoJump.getstate())
			{
				c_misc->AutoJump(pCmd);
			}

			setup_bones();

			linear_extraps.run();

			Prediction->StartPrediction(pCmd);
			{
				animfix->fix_local_player_animations();

				Hacks::MoveHacks(pCmd, bSendPacket);

			}
			Prediction->EndPrediction(pCmd);

			c_misc->fake_crouch(pCmd, bSendPacket, pLocal);

			if (pCmd->forwardmove)
			{
				pCmd->buttons &= ~(pCmd->forwardmove < 0 ? IN_FORWARD : IN_BACK);
				pCmd->buttons |= (pCmd->forwardmove > 0 ? IN_FORWARD : IN_BACK);
			}
			if (pCmd->sidemove) {
				pCmd->buttons &= ~(pCmd->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
				pCmd->buttons |= (pCmd->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
			}
		}

		IClientEntity* LocalPlayer = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

		float flServerTime = (float)(LocalPlayer->GetTickBase()  * interfaces::globals->interval_per_tick);
		static float next_time = 0;
		MovementCorrection(pCmd, LocalPlayer);
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (pLocal->IsAlive())
		{
			if (forward > 450) forward = 450;
			if (right > 450) right = 450;
			if (up > 450) up = 450;
			if (forward < -450) forward = -450;
			if (right < -450) right = -450;
			if (up < -450) up = -450;
			pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
			pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
			pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);
		}

		if (options::menu.misc.OtherSafeMode.getindex() < 3)
		{
			game_utils::NormaliseViewAngle(pCmd->viewangles);
			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}
			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log(" Re-calculating angles");
				game_utils::NormaliseViewAngle(pCmd->viewangles);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}
			if (pCmd->viewangles.x > 90)
			{
				pCmd->forwardmove = -pCmd->forwardmove;
			}
			if (pCmd->viewangles.x < -90)
			{
				pCmd->forwardmove = -pCmd->forwardmove;
			}
	
		lineLBY = pLocal->get_lowerbody();
		lineLBY2 = LastAngleAAReal.y - pLocal->get_lowerbody();
	
		switch (bSendPacket)
		{
			case true:
			{
				c_beam->cham_origin = pLocal->GetAbsOrigin();
				c_beam->fake = pCmd->viewangles;

				if (pCmd->command_number % 3)
					LastAngleAAFake = pCmd->viewangles;
			}
			break;

			case false:
			{
				LastAngleAAReal = pCmd->viewangles;
				c_beam->real = pCmd->viewangles.y;
			}
			break;
		}

		switch (options::menu.visuals.optimize.getstate())
		{
			case true:
				c_misc->optimize();
				break;
		}

		if (pLocal && pLocal->IsAlive() && pWeapon != nullptr && !game_utils::IsGrenade(pWeapon) && !(pWeapon->isZeus() || pWeapon->IsC4()))
		{
			inaccuracy = pWeapon->GetInaccuracy() * 1000;
			lspeed = pLocal->getvelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}

	
		Vector fl = pLocal->GetAbsAngles();
		if (hackManager.pLocal()->GetBasePlayerAnimState())
		{
			fl.y = LastAngleAAFake.y;
		}
		fl.z = 0.f;

		c_beam->cham_angle = fl;
	}
	return false;
}
/*
static void drawThiccLine(int x1, int y1, int x2, int y2, int type, Color color) {
	if (type > 1) {
		Render::Line(x1, y1 - 1, x2, y2 - 1, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1, y1 + 1, x2, y2 + 1, color);
		Render::Line(x1, y1 - 2, x2, y2 - 2, color);
		Render::Line(x1, y1 + 2, x2, y2 + 2, color);
	}
	else {
		Render::Line(x1 - 1, y1, x2 - 1, y2, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1 + 1, y1, x2 + 1, y2, color);
		Render::Line(x1 - 2, y1, x2 - 2, y2, color);
		Render::Line(x1 + 2, y1, x2 + 2, y2, color);
	}
}
*/
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y | %d | %X", &tstruct);
	return buf;
}
Color urmamasuckmylargegenetalia(int speed, int offset)
{
	float hue = (float)((GetCurrentTime() + offset) % speed);
	hue /= speed;
	return Color::FromHSB(hue, 1.0F, 1.0F);
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
static DWORD* deathNotice;
static void(__thiscall *ClearDeathNotices)(DWORD);


void things(IGameEvent* pEvent)
{
	if (!strcmp(pEvent->GetName(), "round_prestart") && interfaces::engine->IsInGame() && interfaces::engine->IsConnected() && hackManager.pLocal())
	{
		deathNotice = nullptr;
		fuckingcheck = true;
		c_misc->anotherpcheck = true;
	
	}
}
template<class T>
static T* FindHudElementX(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(game_utils::FindPattern1(("client_panorama.dll"), ("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(game_utils::FindPattern1(("client_panorama.dll"), ("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));
	return (T*)find_hud_element(pThis, name);
}


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (options::menu.visuals.Active.getstate() && options::menu.visuals.OtherNoScope.getstate() && strcmp("HudZoom", interfaces::panels->GetName(vguiPanel)) == 0)
		return;
	int w, h;
	int centerW, centerh, topH;
	interfaces::engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)interfaces::panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
		interfaces::m_iInputSys->EnableInput(!options::menu.m_bIsOpen);

		if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame() && pLocal->isValidPlayer())
		{

			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)interfaces::pinput + 0xEC);
			CUserCmd* pCmd = cmdlist;
			RECT scrn = Render::GetViewport();
			if (options::menu.misc.Radar.getstate())
				DrawRadar();
			if (globalsh.bSendPaket) {
				globalsh.prevChoked = interfaces::client_state->chokedcommands;
			}
			if (options::menu.visuals.LCIndicator.getindex() != 0)
			{
				if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
				{
					RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Lag Comp Status:");
					bool breaklagcomp = false;
					auto last_origin = pLocal->GetAbsOrigin2();
					if (pLocal->GetAbsOrigin2() != last_origin) 
					{
						if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->GetAbsOrigin2().Length2DSqr() > 4096) {
							breaklagcomp = true;
							last_origin = pLocal->GetAbsOrigin2();
						}
					}

					else if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->getvelocity().Length2D() > 125 && GlobalBREAK::bSendPacket && GlobalBREAK::ChokeAmount == c_fakelag->break_lagcomp_mm_2() || GlobalBREAK::ChokeAmount == 5) {
						breaklagcomp = true;

					}
					else {
						breaklagcomp = false;
					}
					//	Render::Text(6, scrn.bottom - 88, breaklagcomp ? Color(0, 255, 30, 255) : Color(255, 0, 30, 255), Render::Fonts::LBY, "LC");

					switch (options::menu.visuals.LCIndicator.getindex())
					{
						case 1:
						{
							Render::Text(9, scrn.bottom - 88, breaklagcomp ? Color(0, 255, 30, 255) : Color(255, 0, 30, 255), Render::Fonts::LBY, "LC");
						}
						break;

						case 2:
						{
							if (breaklagcomp)
							{
								Render::Text(9, centerh + 115, Color(255, 255, 255, 255), Render::Fonts::xd, " Lag Comp Status:");
								Render::Text(TextSize_2.left + 11, centerh + 115, Color(0, 90, 250, 255), Render::Fonts::xd, "Active");
							}

							if (!breaklagcomp)
							{
								Render::Text(9, centerh + 115, Color(255, 255, 255, 255), Render::Fonts::xd, " Lag Comp Status:");
								Render::Text(TextSize_2.left + 11, centerh + 115, Color(255, 0, 80, 255), Render::Fonts::xd, "Normal");
							}
						}
						break;
					}
					

				}
			}
			
			if (options::menu.visuals.killfeed.getstate())
			{
				if (hackManager.pLocal()) {
					if (!deathNotice) deathNotice = FindHudElementX<DWORD>("CCSGO_HudDeathNotice");
					if (deathNotice) {
						float* localDeathNotice = (float*)((DWORD)deathNotice + 0x50);
						if (localDeathNotice) *localDeathNotice = options::menu.visuals.killfeed.getstate() ? FLT_MAX : 1.5f;
						if (fuckingcheck && deathNotice - 20) {
							if (!ClearDeathNotices)
								ClearDeathNotices = (void(__thiscall*)(DWORD))game_utils::FindPattern1("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");
							if (ClearDeathNotices)
							{
								ClearDeathNotices(((DWORD)deathNotice - 20)); 
								fuckingcheck = false;
							}

						}
					}
				}
			}


			if (options::menu.visuals.FakeDuckIndicator.getindex() != 0)
			{
				switch (options::menu.visuals.FakeDuckIndicator.getindex())
				{
				case 1:
				{
					if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !c_fakelag->shot)
					{
						Render::Text(9, scrn.bottom - 54, Color(0, 250, 30, 255), Render::Fonts::LBY, "FD");
					}
					else
						Render::Text(9, scrn.bottom - 54, Color(255, 0, 30, 255), Render::Fonts::LBY, "FD");
				}
				break;

				case 2:
				{
					RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Fake Duck Status:");
					if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !c_fakelag->shot)
					{
						Render::Text(9, centerh + 130, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Duck Status:");
						Render::Text(TextSize_2.left + 11, centerh + 130, Color(0, 90, 250, 255), Render::Fonts::xd, "Active");
					}
					else
					{
						Render::Text(9, centerh + 130, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Duck Status:");
						Render::Text(TextSize_2.left + 11, centerh + 130, Color(255, 0, 80, 255), Render::Fonts::xd, "Inactive");
					}
				}
				break;
				}
			}
			if (options::menu.visuals.LBYIndicator.getindex() != 0)
			{

				RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "LBY");
				RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " LBY Status:");

				bool invalid_lby = (LastAngleAAReal.y - pLocal->get_lowerbody() >= -35 && LastAngleAAReal.y - pLocal->get_lowerbody() <= 35) || pLocal->IsMoving();
				switch (options::menu.visuals.LBYIndicator.getindex())
				{
				case 1:
				{
					if (invalid_lby)
					{
						Render::Text(9, scrn.bottom - 71, Color(255, 0, 30, 255), Render::Fonts::LBY, "LBY");
					}
					else
					{
						Render::Text(9, scrn.bottom - 71, Color(0, 250, 30, 255), Render::Fonts::LBY, "LBY");
					}
				}
				break;

				case 2:
				{
					if (invalid_lby)
					{
						Render::Text(9, centerh + 100, Color(255, 255, 255, 255), Render::Fonts::xd, " Lby Status:");
						Render::Text(TextSize_2.left + 11, centerh + 100, Color(255, 0, 80, 255), Render::Fonts::xd, "Normal");
					}
					else
					{
						Render::Text(9, centerh + 100, Color(255, 255, 255, 255), Render::Fonts::xd, " Lby Status:");
						Render::Text(TextSize_2.left + 11, centerh + 100, Color(0, 90, 250, 255), Render::Fonts::xd, "Broken");
					}
				}
				break;
				}
			}

			if (options::menu.visuals.fake_indicator.getindex() != 0)
			{
				float yaw_difference = LastAngleAAReal.y - LastAngleAAFake.y;
				bool fake_green = yaw_difference >= 35.f;
				bool fake_orange = yaw_difference < 35.f && yaw_difference > 20.f;

				switch (options::menu.visuals.fake_indicator.getindex())
				{
					case 1:
					{
						RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "FAKE");

						if (fake_green)
						{
							Render::Text(9, scrn.bottom - 105, Color(10, 255, 30, 255), Render::Fonts::LBY, "FAKE");
						}

						else if (fake_orange)
						{
							Render::Text(9, scrn.bottom - 105, Color(255, 150, 10, 255), Render::Fonts::LBY, "FAKE");
						}

						else
						{
							Render::Text(9, scrn.bottom - 105, Color(255, 0, 30, 255), Render::Fonts::LBY, "FAKE");
						}
					}
					break;

					case 2:
					{
						RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Desync Status:");
						switch (fake_green)
						{
							case true:
							{
								Render::Text(9, centerh + 145, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Status:");
								Render::Text(TextSize_2.left + 11, centerh + 145, Color(0, 90, 250, 255), Render::Fonts::xd, yaw_difference > 58 ? "Stretched" : "Optimal");
							}
							break;

							case false:
							{
								Render::Text(9, centerh + 145, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Status:");
								Render::Text(TextSize_2.left + 11, centerh + 145, Color(255, 0, 80, 255), Render::Fonts::xd, "Minimal");
							}
							break;
						}
					}
					break;
				}
			}

			if (options::menu.visuals.manualaa_type.getindex() > 0)
			{
				switch (options::menu.visuals.manualaa_type.getindex())
				{
					case 1:
					{
						c_visuals->single_arrow();
					}
					break;

					case 2:
					{
						c_visuals->all_arrows();
					}
					break;
				}
			}

			if (options::menu.visuals.cheatinfo.getstate())
			{
				char jew[64];
				float blob = interfaces::client_state->chokedcommands;
				float hc = options::menu.aimbot.AccuracyHitchance.getvalue();
				float md = options::menu.aimbot.AccuracyMinimumDamage.getvalue();
				
				auto choke = *(int*)(uintptr_t(interfaces::client_state) + 0x4D28); // because it refused to work the normal way

				sprintf_s(jew, " Pitch: %.1f", pitchmeme);
				Render::Text(9, (centerh - 20), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Yaw: %.1f", c_beam->real);
				Render::Text(9, (centerh), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Lby: %.1f", lineLBY);
				Render::Text(9, (centerh + 20), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Real / Lby Delta: %.1f", NormalizeYaw(lineLBY2));
				Render::Text(9, (centerh + 40), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Speed: %.1f", lspeed);
				Render::Text(9, (centerh + 60), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

		//		sprintf_s(jew, " Choke: %.1f", choke); // choke on a dick, feggit
		//		Render::Text(9, (centerh + 60), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

			}

		}

		if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
		{
			Hacks::DrawHacks();

			switch (options::menu.visuals.filter_antiaim.getstate())
			{
				case true: c_visuals->antiaim_arrow(); // because fuck you that's why
						break;
			}

			if (options::menu.visuals.OtherHitmarker.getstate())
				hitmarker::singleton()->on_paint();
		}
		skinchanger.update_settings();

		if (options::menu.m_bIsOpen && options::menu.ColorsTab.menu_backdrop.getstate())
		{
			Drop::DrawBackDrop();
		}

		options::DoUIFrame();


	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}
bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = interfaces::prediction_dword;
	result = origFunc(ecxVal);
	if (options::menu.visuals.OtherNoVisualRecoil.getstate() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;
		float* m_LocalViewAngles = NULL;
		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}
		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();
		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}
	return result;
}

player_info_t GetInfo(int Index) {
	player_info_t Info;
	interfaces::engine->GetPlayerInfo(Index, &Info);
	return Info;
}
typedef void(__cdecl* MsgFn)(const char* msg, va_list);

bool warmup = false;

auto HitgroupToString = [](int hitgroup) -> std::string
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "HEAD";
	case HITGROUP_CHEST:
		return "CHEST";
	case HITGROUP_STOMACH:
		return "STOMACH";
	case HITGROUP_LEFTARM:
		return "LEFT ARM";
	case HITGROUP_RIGHTARM:
		return "RIGHT ARM";
	case HITGROUP_LEFTLEG:
		return "LEFT LEG";
	case HITGROUP_RIGHTLEG:
		return "RIGHT LEG";
	default:
		return "BODY";
	}
};
void Msg(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}

void hitsound()
{
	const char* _custom = "csgo\\sound\\mirror_custom.wav";
	switch (options::menu.misc.hitmarker_sound.getindex())
	{
	case 1: PlaySoundA(cod, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 2: interfaces::engine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
		break;
	case 3: PlaySoundA(bubble, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 4: PlaySoundA(bameware_sound, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 5: PlaySoundA(anime, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 6: PlaySoundA(hitler_wav, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 7: interfaces::engine->ExecuteClientCmd("play training\\bell_impact"); // buttons\light_power_on_switch_01.wav
		break;
	case 8: PlaySoundA(_custom, NULL, SND_ASYNC);
		break;
	}
}


void ConColorMsg(Color const &color, const char* buf, ...)
{
	using ConColFn = void(__stdcall*)(Color const &, const char*, ...);
	auto ConCol = reinterpret_cast<ConColFn>((GetProcAddress(GetModuleHandle("tier0.dll"), "?ConColorMsg@@YAXABVColor@@PBDZZ")));
	ConCol(color, buf);
}


struct bullet_impact_log
{
	bullet_impact_log(int userid, Vector fire_pos, Vector impact_pos, float impact_time) {
		this->uid = userid;
		this->fire_posit = fire_pos;
		this->impact_posit = impact_pos;
		this->impac_time = impact_time;
	}

	int uid;
	Vector fire_posit;
	Vector impact_posit;
	float impac_time;
};

std::vector< bullet_impact_log > bullet_logs;

void DrawBeam(Vector src, Vector end, Color Color) {
	int r, g, b, a;
	Color.GetColor(r, g, b, a);
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;

	beamInfo.m_pszModelName = "sprites/blueglow1.vmt";

	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 3.0;
	beamInfo.m_flWidth = 1.f;
	beamInfo.m_flEndWidth = 1.f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 1.f;
	beamInfo.m_flBrightness = a;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = r;
	beamInfo.m_flGreen = g;
	beamInfo.m_flBlue = b;
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;

	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;

	Beam_t* myBeam = interfaces::render_beams->CreateBeamPoints(beamInfo);

	if (myBeam)
		interfaces::render_beams->DrawBeam(myBeam);
}


void draw_hitboxes(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		interfaces::DebugOverlay->DrawPill(vMin, vMax, hitbox->m_flRadius, r, g, b, a, duration);
	}
}

bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{
	CBulletListener::singleton()->OnStudioRender();
	IClientEntity* localplayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		std::vector<dropdownboxitem> spike = options::menu.misc.killsay.items;
		things(Event);
		std::string event_name = Event->GetName();

		angle_correction ac;

		if (!strcmp(Event->GetName(), "round_prestart"))
		{
			skinchanger.set_viewmodel();
			fuckingcheck = false;
		}
		
		if (!strcmp(Event->GetName(), "round_start"))
		{
			skinchanger.set_viewmodel();
		}

		if (!strcmp(Event->GetName(), "player_death"))
		{
			skinchanger.apply_killcon(Event);	

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");

			if (spike[1].GetSelected && interfaces::engine->GetPlayerForUserID(deadfag) == interfaces::engine->GetLocalPlayer() && interfaces::engine->GetPlayerForUserID(attackingfag) != interfaces::engine->GetLocalPlayer())
			{
				if (!deathmsg.empty()) {
					std::string msg = deathmsg[rand() % deathmsg.size()];
					std::string str;
					str.append("say ");
					str.append(msg);
					interfaces::engine->ClientCmd_Unrestricted(str.c_str());
				}
			}
			if (spike[0].GetSelected && interfaces::engine->GetPlayerForUserID(deadfag) != interfaces::engine->GetLocalPlayer() && interfaces::engine->GetPlayerForUserID(attackingfag) == interfaces::engine->GetLocalPlayer())
			{
				if (!killmsg.empty()) 
				{
					std::string msg = killmsg[rand() % killmsg.size()];
					std::string str;
					str.append("say ");
					str.append(msg);
					interfaces::engine->ClientCmd_Unrestricted(str.c_str());
				}
			}
		}
	
		if (!strcmp(Event->GetName(), "game_newmap"))
		{
			skinchanger.set_viewmodel();			

			ac.mirror_aesthetic_console();
			interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] changing map.     \n");

			if (options::menu.visuals.colmod.getvalue() < 100.f)
			{
				options::menu.visuals.colmodupdate.SetState(true);
				ac.mirror_aesthetic_console();
				interfaces::cvar->ConsoleColorPrintf(Color(250, 0, 200, 255), " [info] refreshed world modulation.     \n");
			}

		}

		if (strstr(Event->GetName(), "item_purchase")) {
			
			c_misc->buy_logs(Event);
		}
		if (!strcmp(Event->GetName(), "player_hurt"))
		{
			auto entity = interfaces::ent_list->get_client_entity(interfaces::engine->GetPlayerForUserID(Event->GetInt("userid")));
			int attackerid = Event->GetInt("attacker");
			int entityid = interfaces::engine->GetPlayerForUserID(attackerid);
			if (entityid == interfaces::engine->GetLocalPlayer())
			{
				hitsound();
				int nUserID = Event->GetInt("attacker");
				int nDead = Event->GetInt("userid");
				int gaylol = Event->GetInt("hitgroup");

				if ((nUserID || nDead) && nUserID != nDead)
				{
					Globals::hit[entity->GetIndex()] += 1;
					player_info_t killed_info = GetInfo(interfaces::engine->GetPlayerForUserID(nDead));
					player_info_t killer_info = GetInfo(interfaces::engine->GetPlayerForUserID(nUserID));
					IClientEntity* hurt = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetPlayerForUserID(Event->GetInt("userid")));

					if (options::menu.ColorsTab.DebugLagComp.getstate())
					{
						studiohdr_t* studio_hdr = interfaces::model_info->GetStudiomodel(hurt->GetModel());
						mstudiohitboxset_t* set = studio_hdr->GetHitboxSet(0);
						for (int i = 0; i < set->numhitboxes; i++)
						{
							mstudiobbox_t* hitbox = set->GetHitbox(i);
							if (!hitbox)
								continue;

							draw_hitboxes(hurt, 220, 220, 220, 255, 1, hitbox->m_flRadius);
						}
					}
					auto remaining_health = Event->GetString("health");
					int remainaing = Event->GetInt("health");
					auto dmg_to_health = Event->GetString("dmg_health");
					std::string szHitgroup = HitgroupToString(gaylol);
					interfaces::cvar->ConsoleColorPrintf(Color(220, 5, 250, 255), "Mirror: ");
					std::string One = "-";
					std::string Two = dmg_to_health;
					std::string Three = " in the ";
					std::string Four = szHitgroup;
					std::string gey = " of ";
					std::string yes = killed_info.name;
					std::string yyes = " [";
					std::string yyyes = " hp: ";
					std::string yyyyes = remaining_health;
					std::string yyyyyes = " ]";
					std::string newline = ".     \n";//no,i'm not retarded, i tried with stringstream but it didn't work.
					std::string uremam = One + Two + Three + Four + gey + yes + yyes + yyyes + yyyyes + yyyyyes + newline;

						
					switch (options::menu.visuals.logs.getindex())
					{
						case 1: Msg(uremam.c_str());
							break;
						case 2: ConColorMsg(Color(options::menu.ColorsTab.console_colour.GetValue()) ,uremam.c_str());
							break;
					}

					/*
					if (options::menu.visuals.bulletbeam.getstate())
					{
						auto shooter = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(
							interfaces::engine->GetPlayerForUserID(Event->GetInt("userid"))));

						int uid = Event->GetInt("userid");
						int x = Event->GetInt("x");
						int y = Event->GetInt("y");
						int z = Event->GetInt("z");

						if (interfaces::engine->GetPlayerForUserID(uid) == interfaces::engine->GetLocalPlayer())
						{
							Vector position(x, y, z);

							Vector CorrectedPos = Vector(shooter->GetEyePosition().x, shooter->GetEyePosition().y,
								shooter->GetEyePosition().z - 0.75);

							bullet_logs.push_back(bullet_impact_log(Event->GetInt("userid"), CorrectedPos, position,
								interfaces::globals->curtime));
						}
					}
					*/
				}
			}
		}

		/*
		if (options::menu.visuals.bulletbeam.getstate())
		{
			for (unsigned int i = 0; i < bullet_logs.size(); i++) {
				auto shooter = interfaces::ent_list->get_client_entity(
					interfaces::engine->GetPlayerForUserID(bullet_logs[i].uid));

				if (shooter && on_hit == true) {
					DrawBeam(bullet_logs[i].fire_posit, bullet_logs[i].impact_posit,
						Color(options::menu.ColorsTab.bullet_tracer.GetValue()));
				}

				bullet_logs.erase(bullet_logs.begin() + i);
			}
		}
		*/

		if (options::menu.aimbot.resolver.getindex() > 0)
		{
			auto entity = interfaces::ent_list->get_client_entity(interfaces::engine->GetPlayerForUserID(Event->GetInt("userid")));

			IClientEntity* pLocal = hackManager.pLocal();

			if (!strcmp(Event->GetName(), "weapon_fire")) 
			{
				auto userID = Event->GetInt("userid");
				auto attacker = interfaces::engine->GetPlayerForUserID(userID);

				if (attacker) 
				{
					if (attacker == interfaces::engine->GetLocalPlayer() && entity)
					{
						ac.mirror_aesthetic_console();
						interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] weapon fired.     \n");
						Globals::fired[entity->GetIndex()] += 1;
					}
				}
			}

			if (entity)
			{
				Globals::missedshots[entity->GetIndex()] = Globals::fired[entity->GetIndex()] - Globals::hit[entity->GetIndex()];

				if (Globals::missedshots[entity->GetIndex()] > 7)
				{
					Globals::hit[entity->GetIndex()] = 0;
					Globals::fired[entity->GetIndex()] = 0;
					Globals::missedshots[entity->GetIndex()] = 0;
					ac.mirror_aesthetic_console();
					interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] reset bullet count.     \n");
		
				}
			}
		}
	}
	return oFireEventClientSide(ECX, Event);
}
#define TEXTURE_GROUP_LIGHTMAP                      "Lightmaps"
#define TEXTURE_GROUP_WORLD                         "World textures"
#define TEXTURE_GROUP_MODEL                         "Model textures"
#define TEXTURE_GROUP_VGUI                          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE                      "Particle textures"
#define TEXTURE_GROUP_DECAL                         "Decal textures"
#define TEXTURE_GROUP_SKYBOX                        "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS                "ClientEffect textures"
#define TEXTURE_GROUP_OTHER                         "Other textures"
#define TEXTURE_GROUP_PRECACHED                     "Precached"
#define TEXTURE_GROUP_CUBE_MAP                      "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET                 "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED                   "Unaccounted textures"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER           "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP     "Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR    "Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD    "World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS   "Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER    "Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER          "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER         "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER                  "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL                    "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS                 "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS                "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE         "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS                 "Morph Targets"
void draw_hitbox_bt(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);

	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		interfaces::DebugOverlay->DrawPill(vMin, vMax, diameter, r, g, b, a, duration);
	}
}
void __fastcall  hkSceneEnd(void *pEcx, void *pEdx)
{
	Hooks::VMTRenderView.GetMethod<SceneEnd_t>(9)(pEcx);

	IClientEntity* local_player = hackManager.pLocal();

	if ( !interfaces::engine->IsInGame() || !interfaces::engine->IsConnected())
		return pSceneEnd(pEcx);

	pSceneEnd(pEcx);

	float blend_vis = options::menu.visuals.enemy_blend.getvalue() / 100;
	float blend_invis = options::menu.visuals.enemy_blend_invis.getvalue() / 100;

	for (int i = 1; i <= interfaces::globals->max_clients; i++)
	{
		auto ent = interfaces::ent_list->get_client_entity(i);

		if (ent)
		{	
			if (options::menu.visuals.fakelag_ghost.getstate())
			{
				float color[4] = { 0.6f, 0.01f, 1.f, 0.2f };
				IMaterial * estrogen = interfaces::materialsystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_MODEL, true);
				if (!estrogen || estrogen->IsErrorMaterial() || !local_player->isValidPlayer())
					return;

				Vector OrigAngle = local_player->GetAbsAngles_2();
				Vector OrigOrigin = local_player->GetAbsOrigin();

				local_player->SetAbsOriginal(c_beam->cham_origin);
				local_player->SetAbsAngles(c_beam->cham_angle);

				estrogen->ColorModulate(0.4, 0.01, 0.8);
			
				interfaces::render_view->SetBlend(0.3f);

				interfaces::model_render->ForcedMaterialOverride(estrogen);
				interfaces::render_view->SetColorModulation(color);
			//	estrogen->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				local_player->draw_model(0x1, 255);
				interfaces::model_render->ForcedMaterialOverride(nullptr);


				interfaces::render_view->SetBlend(1.f);

				estrogen->AlphaModulate(color[4]);
				local_player->SetAbsAngles(OrigAngle);
				local_player->SetAbsOriginal(OrigOrigin);

				
			}


			if (options::menu.ColorsTab.BackTrackBones2.getstate())
			{
				if (ent->getvelocity().Length2D() > 25 && ent->cs_player() && !ent->dormant() && local_player->IsAlive() && ent->team() != local_player->team())
					draw_hitbox_bt(ent, options::menu.ColorsTab.misc_lagcompBones.GetValue()[0], options::menu.ColorsTab.misc_lagcompBones.GetValue()[1], options::menu.ColorsTab.misc_lagcompBones.GetValue()[2], 255, 0.2, 0);
			}

		}

	}
}

void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

	Color color;
	float flColor[3] = { 0.f };
	bool DontDraw = false;
	static IMaterial* mat = CreateMaterialLit();

	int HandsStyle = options::menu.visuals.HandCHAMS.getindex();
	int gunstyle = options::menu.visuals.GunCHAMS.getindex();

	const char* ModelName = interfaces::model_info->GetModelName((model_t*)pInfo.pModel);
//	std::string ModelName_test = interfaces::model_info->GetModelName(pInfo.pModel);

	IClientEntity* pentity = (IClientEntity*)interfaces::ent_list->get_client_entity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

//	Color color_invis = Color(options::menu.ColorsTab.ChamsEnemyNotVis.GetValue());
//	Color color_vis = Color(options::menu.ColorsTab.ChamsEnemyVis.GetValue());

	float blend_vis = options::menu.visuals.enemy_blend.getvalue() / 100;
	float blend_invis = options::menu.visuals.enemy_blend_invis.getvalue() / 100;

	float fl_color_x[3] = { 0.f };
	float fl_color1[4] = { 0.f };
/*
	fl_color_x[0] = color_invis[0] / 255.f;
	fl_color_x[1] = color_invis[1] / 255.f;
	fl_color_x[2] = color_invis[2] / 255.f;
	fl_color_x[3] = blend_invis;

	fl_color1[0] = color_vis[0] / 255.f;
	fl_color1[1] = color_vis[1] / 255.f;
	fl_color1[2] = color_vis[2] / 255.f;
	fl_color1[3] = blend_vis;
*/

	int v = options::menu.visuals.visible_chams_type.getindex();
	int iv = options::menu.visuals.invisible_chams_type.getindex();

	float blend = options::menu.visuals.transparency.getvalue() / 100;
	float hand_blend = options::menu.visuals.hand_transparency.getvalue() / 100;
	float gun_blend = options::menu.visuals.gun_transparency.getvalue() / 100;
	float sleeve_blend = options::menu.visuals.sleeve_transparency.getvalue() / 100;
	float blend_local = options::menu.visuals.blend_local.getvalue() / 100;

	static IMaterial* covered = CreateMaterial(true, false);
	static IMaterial* wire = CreateMaterial(true, false, true);
	static IMaterial * glass = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
	static IMaterial * crystal = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
	static IMaterial * usual = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
	static IMaterial * gold = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
	static IMaterial * dogtag = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
	static IMaterial * devglow = interfaces::materialsystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_OTHER, true);
	static IMaterial * fbi = interfaces::materialsystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass", TEXTURE_GROUP_OTHER, true);

	static IMaterial * fish = interfaces::materialsystem->FindMaterial("models/props_shacks/fishing_net01", TEXTURE_GROUP_OTHER, true);
	static IMaterial * hydra = interfaces::materialsystem->FindMaterial("models/inventory_items/hydra_crtysal/hydra_crystal_detail", TEXTURE_GROUP_OTHER, true); //hhhhhhhhhydra

	int ChamsStyle = options::menu.visuals.OptionsChams.getindex();
	int sleeves = options::menu.visuals.SleeveChams.getindex();
//	IMaterial *covered = CoveredFlat;

	float fl_color[3] = { 0.f };

	IMaterial * visible; IMaterial * invisible;

	switch (v)
	{
		case 0: visible = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
			break;
		case 1: visible = interfaces::materialsystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true);
			break;
		case 2: visible = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
			break;
		case 3: visible = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
			break;
		case 4:	visible = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
			break;
		case 5: visible = interfaces::materialsystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_OTHER, true);
			break;
		case 6: visible = interfaces::materialsystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass", TEXTURE_GROUP_OTHER, true);
			break;
		case 7: visible = interfaces::materialsystem->FindMaterial("models/props_shacks/fishing_net01", TEXTURE_GROUP_OTHER, true);
			break;
		case 8: visible = interfaces::materialsystem->FindMaterial("models/inventory_items/hydra_crtysal/hydra_crystal_detail", TEXTURE_GROUP_OTHER, true);
			break;
	}

	switch (iv)
	{
		case 0: invisible = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
			break;
		case 1: invisible = interfaces::materialsystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true);
			break;
		case 2: invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
			break;
		case 3: invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
			break;
		case 4:	invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
			break;
		case 5: invisible = interfaces::materialsystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_OTHER, true);
			break;
		case 6: invisible = interfaces::materialsystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass", TEXTURE_GROUP_OTHER, true);
			break;
		case 7: invisible = interfaces::materialsystem->FindMaterial("models/props_shacks/fishing_net01", TEXTURE_GROUP_OTHER, true);
			break;
		case 8: invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/hydra_crtysal/hydra_crystal_detail", TEXTURE_GROUP_OTHER, true);
			break;
	}

	auto b_shadow_depth = (pInfo.flags & 0x40000000) != 0;

	if (b_shadow_depth) //so hooking dme will remove shadows you dont want that
	{
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		return;
	}

	if (options::menu.misc.lag_model.getstate()) // because the stickrpg effect is soooooooooo good and totally didn't exist in bitsys 2 years ago
	{
		pLocal->SetAbsOriginal(c_beam->cham_origin);
		pLocal->SetAbsOriginal(pLocal->GetAbsOrigin());
	}

	bool do_team = options::menu.visuals.ChamsTeamVis.getindex() > 0;
	bool do_local = options::menu.visuals.ChamsLocal.getstate();
	if (pentity->isValidPlayer() && strstr(ModelName, "models/player"))
	{
		if (options::menu.visuals.ChamsEnemy.getindex() != 0)
		{
			if (pentity->team() != pLocal->team() && !pentity->dormant() && pLocal)
			{
				switch (options::menu.visuals.ChamsEnemy.getindex())
				{
				case 1:
				{
					if (!visible || visible->IsErrorMaterial())
						return;

					fl_color_x[0] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[0] / 255.f;
					fl_color_x[1] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[1] / 255.f;
					fl_color_x[2] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[2] / 255.f;
					fl_color_x[3] = 1;
					interfaces::render_view->SetColorModulation(fl_color_x);
				//	interfaces::render_view->SetBlend(1.f);
					interfaces::model_render->ForcedMaterialOverride(visible);
				}
				break;

				case 2:
				{
					if (!invisible || invisible->IsErrorMaterial())
						return;

					fl_color_x[0] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[0] / 255.f;
					fl_color_x[1] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[1] / 255.f;
					fl_color_x[2] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[2] / 255.f;
					pLocal->IsAlive() ?	interfaces::render_view->SetColorModulation(fl_color_x) : invisible->ColorModulate(fl_color_x[0], fl_color_x[1], fl_color_x[2]);
					interfaces::render_view->SetBlend(1.f);
					invisible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					interfaces::model_render->ForcedMaterialOverride(invisible);

					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

					if (!visible || visible->IsErrorMaterial())
						return;

					visible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					fl_color1[0] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[0] / 255.f;
					fl_color1[1] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[1] / 255.f;
					fl_color1[2] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[2] / 255.f;
					interfaces::render_view->SetColorModulation(fl_color1);
					interfaces::render_view->SetBlend(1.f);
					interfaces::model_render->ForcedMaterialOverride(visible);
				}
				break;
				}
			}
		}

		if ((do_team && pentity->team() == pLocal->team() && pentity != pLocal && !do_local)
			|| (do_local && !do_team && pentity == pLocal && !pLocal->IsScoped()) || (do_team && do_local && pentity->team() == pLocal->team() && !pLocal->IsScoped()))
		{

			switch (options::menu.visuals.ChamsTeamVis.getindex())
			{
			case 1:
			{
				if (!visible || visible->IsErrorMaterial())
					return;

				fl_color_x[0] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[0] / 255.f;
				fl_color_x[1] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[1] / 255.f;
				fl_color_x[2] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color_x);
				interfaces::render_view->SetBlend(1.f);
				interfaces::model_render->ForcedMaterialOverride(visible);
			}
			break;

			case 2:
			{
				if (!invisible || invisible->IsErrorMaterial())
					return;

				fl_color_x[0] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[0] / 255.f;
				fl_color_x[1] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[1] / 255.f;
				fl_color_x[2] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color_x);
				interfaces::render_view->SetBlend(1.f);
				invisible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
				interfaces::model_render->ForcedMaterialOverride(invisible);

				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

				if (!visible || visible->IsErrorMaterial())
					return;

				
				fl_color1[0] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[0] / 255.f;
				fl_color1[1] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[1] / 255.f;
				fl_color1[2] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color1);
				interfaces::render_view->SetBlend(1.f);
				visible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(visible);
			}
			break;
			}


		}

		if (options::menu.visuals.localmaterial.getindex() != 0 && pLocal->IsScoped() && pentity == pLocal)
		{

			switch (options::menu.visuals.localmaterial.getindex())
			{

			case 1:
			{
				interfaces::render_view->SetBlend(blend);
			}
			break;

			case 2:
			{

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;

				interfaces::render_view->SetBlend(blend);
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::model_render->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 3:
			{

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 4:
			{

				flColor[0] = rand() % 250 / 255.f;
				flColor[1] = rand() % 250 / 255.f;
				flColor[2] = rand() % 250 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 5:
			{

				flColor[0] = 5 / 255.f;
				flColor[1] = 5 / 255.f;
				flColor[2] = 5 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 6:
			{
			
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 7:
			{
				if (!devglow || devglow->IsErrorMaterial())
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				devglow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(devglow);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 8:
			{
				if (!fbi || fbi->IsErrorMaterial())
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				fbi->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(fbi);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 9:
			{
				if (!fish || fish->IsErrorMaterial()) // i want fish. Fish is good. It does "glug glug" and swims n shit. You don't fuck with fish rofl
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				fish->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(fish);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 10:
			{
				if (!hydra || hydra->IsErrorMaterial()) // hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhydra (nn)
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				hydra->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(hydra);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;
			}

		}
	}
	if (gunstyle != 0)
	{
		if (strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
		{

			switch (gunstyle) // this shit was done in "else if"s with " == " instead of A > B or switch statements which is fucking HECKIN trash for optimization
			{
			case 1:
			{
				if (!usual || usual->IsErrorMaterial())
					return;
				usual->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(usual);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 2:
			{
				//	static IMaterial* mat_T = interfaces::materialsystem->FindMaterial("metal", TEXTURE_GROUP_MODEL);

				if (!mat || mat->IsErrorMaterial())
					return;
				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
			//	mat->AlphaModulate(1.0f);
			//	mat->ColorModulate(col[0], col[1], col[2]);
				interfaces::render_view->SetColorModulation(col);
			//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, 1);

				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 3:
			{
				IMaterial *material = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
				interfaces::render_view->SetBlend(gun_blend);
				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				//		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				//		material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
				interfaces::model_render->ForcedMaterialOverride(material);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 4:
			{
				interfaces::render_view->SetBlend(gun_blend);
				glass->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 5:
			{
				flColor[0] = 200 / 255.f;
				flColor[1] = 200 / 255.f;
				flColor[2] = 200 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(0.2f);
				interfaces::model_render->ForcedMaterialOverride(crystal);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 6:
			{
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;

				interfaces::render_view->SetBlend(gun_blend);
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 7:
			{
				if (!devglow || devglow->IsErrorMaterial())
					return;
				devglow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(devglow);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 8:
			{
				if (!fbi || fbi->IsErrorMaterial())
					return;
				fbi->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(fbi);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 9:
			{
				if (!fish || fish->IsErrorMaterial()) // BRO there's fish everywhere. Like, FUCK
					return;
				fish->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(fish);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 10:
			{
				if (!hydra || hydra->IsErrorMaterial())
					return;
				hydra->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(hydra);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;
			}
		}
	}


	if (sleeves != 0 && (strstr(ModelName, "v_sleeve")))
	{
	
		switch (sleeves)
		{
			case 1:
			{
				if (!usual || usual->IsErrorMaterial())
					return;

				usual->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;

				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(usual);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 2:
			{
				if (!mat || mat->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 3:
			{
				IMaterial *material = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
				
				interfaces::render_view->SetBlend(sleeve_blend);

				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(material);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 4:
			{
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			
			}
			break;

			case 5:
			{
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(crystal);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 6:
			{
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 7:
			{
				if (!usual || usual->IsErrorMaterial())
					return;

				usual->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				interfaces::model_render->ForcedMaterialOverride(usual);
			}
			break;

			case 8:
			{
				if (!devglow || devglow->IsErrorMaterial())
					return;
				devglow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(devglow);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 9:
			{
				if (!fbi || fbi->IsErrorMaterial())
					return;
				fbi->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(fbi);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 10:
			{
				if (!fish || fish->IsErrorMaterial())
					return;
				fish->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(fish);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 11:
			{
				if (!hydra || hydra->IsErrorMaterial())
					return;
				hydra->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(hydra);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

		} 

	}

	if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")) && pLocal && pLocal->IsAlive() && !strstr(ModelName, "v_sleeve"))
	{
		switch (HandsStyle)
		{
		case 1:
		{
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
		
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			interfaces::model_render->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 2:
		{
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
		//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::model_render->ForcedMaterialOverride(wire);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 3:
		{
			interfaces::render_view->SetBlend(1.f);		
			gold->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, true);
			interfaces::model_render->ForcedMaterialOverride(gold);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 4:
		{
			interfaces::render_view->SetBlend(1.f);
			glass->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			interfaces::model_render->ForcedMaterialOverride(glass);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 5:
		{
			flColor[0] = 5 / 255.f;
			flColor[1] = 5 / 255.f;
			flColor[2] = 5 / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(0.2f);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 6:
		{
			if (!dogtag || dogtag->IsErrorMaterial())
				return;

			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
		
			interfaces::render_view->SetBlend(hand_blend);
			dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::model_render->ForcedMaterialOverride(dogtag);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 7:
		{
			if (!devglow || devglow->IsErrorMaterial())
				return;
			devglow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::model_render->ForcedMaterialOverride(devglow);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 8:
		{
			if (!fbi || fbi->IsErrorMaterial())
				return;
			fbi->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::model_render->ForcedMaterialOverride(fbi);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 9:
		{
			if (!fish || fish->IsErrorMaterial())
				return;
			fish->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::model_render->ForcedMaterialOverride(fish);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 10:
		{
			if (!hydra || hydra->IsErrorMaterial())
				return;
			fish->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::model_render->ForcedMaterialOverride(hydra);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;
		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	interfaces::model_render->ForcedMaterialOverride(NULL);
}

std::vector<const char*> vistasmoke_mats =
{
	"particle/beam_smoke_01",
	"particle/particle_smokegrenade",
	"particle/particle_smokegrenade1",
	"particle/particle_smokegrenade2",
	"particle/particle_smokegrenade3",
	"particle/particle_smokegrenade_sc",
	"particle/smoke1/smoke1",
	"particle/smoke1/smoke1_ash",
	"particle/smoke1/smoke1_nearcull",
	"particle/smoke1/smoke1_nearcull2",
	"particle/smoke1/smoke1_snow",
	"particle/smokesprites_0001",
	"particle/smokestack",
	"particle/vistasmokev1/vistasmokev1",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_nearcull",
	"particle/vistasmokev1/vistasmokev1_nearcull_fog",
	"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev4_emods_nocull",
	"particle/vistasmokev1/vistasmokev4_nearcull",
	"particle/vistasmokev1/vistasmokev4_nocull"
};

class CBaseAnimating
{
public:
	std::array<float, 24>* m_flPoseParameter()
	{
		static int offset = 0;
		if (!offset)
			offset = 0x2764;
		return (std::array<float, 24>*)((uintptr_t)this + offset);
	}
	model_t* GetModel()
	{
		void* pRenderable = reinterpret_cast<void*>(uintptr_t(this) + 0x4);
		typedef model_t* (__thiscall* fnGetModel)(void*);
		return call_vfunc<fnGetModel>(pRenderable, 8)(pRenderable);
	}

	void SetBoneMatrix(matrix3x4_t* boneMatrix)
	{
		//Offset found in C_BaseAnimating::GetBoneTransform, string search ankle_L and a function below is the right one
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = interfaces::model_info->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(matrix + i, boneMatrix + i, sizeof(matrix3x4_t));
		}
	}
	void GetDirectBoneMatrix(matrix3x4_t* boneMatrix)
	{
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = interfaces::model_info->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(boneMatrix + i, matrix + i, sizeof(matrix3x4_t));
		}
	}
};

void UpdateIncomingSequences()
{
	auto clientState = interfaces::client_state; //DONT HARDCODE OFFESTS

	if (!clientState)
		return;

	auto intnetchan = clientState->m_NetChannel; //Can optimise, already done in CM hook, make a global

	INetChannel* netchan = reinterpret_cast<INetChannel*>(intnetchan);
	if (netchan)
	{
		if (netchan->m_nInSequenceNr > lastincomingsequencenumber)
		{
			//sequences.push_front(netchan->m_nInSequenceNr);
			lastincomingsequencenumber = netchan->m_nInSequenceNr;

			sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, interfaces::globals->realtime));
		}
		if (sequences.size() > 2048)
			sequences.pop_back();
	}
}


void AnimFix_ghetto()
{
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
		auto animations = local_player->GetBasePlayerAnimState();

		if (!animations)
			return;
		if (!local_player->IsAlive())
			return;
		if (!local_player)
			return;

		local_player->client_side_animation() = true;

		auto old_curtime = interfaces::globals->curtime;
		auto old_frametime = interfaces::globals->frametime;
	//	auto old_ragpos = local_player->get_ragdoll_pos();
		interfaces::globals->curtime = local_player->GetSimulationTime();
		interfaces::globals->frametime = interfaces::globals->interval_per_tick;
		auto player_animation_state = reinterpret_cast<DWORD*>(local_player + 0x3894);
		//		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);


	//	local_player->get_ragdoll_pos() = old_ragpos;
		local_player->UpdateClientSideAnimation();

		interfaces::globals->curtime = old_curtime;
		interfaces::globals->frametime = old_frametime;

		local_player->SetAbsAngles(Vector(0.f, hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw, 0.f));//if u not doin dis it f*cks up the model lol

		local_player->client_side_animation() = false;
	}
}

void force_full_update() {
	static auto full_update = game_utils::pattern_scan("engine.dll", "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");

	typedef void(*fn_full_update) (void);
	fn_full_update cl_fullupdate = (fn_full_update)(full_update);
	cl_fullupdate();
}

auto smoke_count = *(DWORD*)(Utilities::Memory::FindPatternV2("client_panorama", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	IClientEntity* local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;

	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		for (int i = 0; i < interfaces::globals->max_clients; i++)
		{
	
			if (!options::menu.aimbot.AimbotEnable.getstate())
				continue;

			// because fuck running an """""anim fix""""" if you ain't doin shit
			animfix->re_work(curStage);

			auto m_entity = interfaces::ent_list->get_client_entity(i);

			if (i == interfaces::engine->GetLocalPlayer())
				continue;

			if (!m_entity || !m_entity->cs_player()|| !m_entity->IsAlive() || m_entity == local_player && curStage != FRAME_NET_UPDATE_END)
			{
				continue;
			}

		//	lagcompensation.disable_interpolation(m_entity);

			CTickRecord trans = CTickRecord(m_entity);
			cbacktracking::Get().ClearRecord(m_entity);
			cbacktracking::Get().SaveTemporaryRecord(m_entity, trans);

			*(int*)((uintptr_t)m_entity + 0xA30) = interfaces::globals->framecount;
			*(int*)((uintptr_t)m_entity + 0xA28) = 0;
		}

		if (options::menu.misc.OtherThirdperson.getstate() && local_player)
		{
			
			static bool enabledtp = false, check = false;


			if (GetAsyncKeyState(options::menu.misc.ThirdPersonKeyBind.GetKey()) && hackManager.pLocal()->IsAlive())
			{
				if (!check)
					enabledtp = !enabledtp;
				check = true;
			}
			else
				check = false;

			if (enabledtp)
			{
				ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(local_player) + 0x31D8) = LastAngleAAReal;
				local_player->set_abs_angles_v2(Vector(0.f, c_beam->real, 0.f)); // because FUCK that nae nae effect
			}
		
			IClientEntity* obstarget = interfaces::ent_list->GetClientEntityFromHandle(local_player->GetObserverTargetHandle());
			if (interfaces::pinput->m_fCameraInThirdPerson)
			{
				Vector viewangs = *(Vector*)(reinterpret_cast<uintptr_t>(local_player) + 0x31D8); 
				
				if (GlobalBREAK::TicksOnGround == 1)
				{
					viewangs.x = -10;
					viewangs.y = LastAngleAAReal.y;
					viewangs.z = 0.f;
				}
				else
				{
					viewangs = LastAngleAAReal;
				}
			}
	
			if (enabledtp &&local_player->IsAlive())
			{
				interfaces::pinput->m_fCameraInThirdPerson = true;
				Vector camForward;
			}

			else
			{
				interfaces::pinput->m_fCameraInThirdPerson = false;
			}

		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			UpdateIncomingSequences();

			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().StartLagCompensation(local_player);
			
		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {

			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().FinishLagCompensation(local_player);
		}

		if (curStage == FRAME_NET_UPDATE_END)
		{
			CMBacktracking::Get().FrameUpdatePostEntityThink();
		}
		
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) 
		{
			if (local_player && local_player->IsAlive())
			{

				if (!local_player->m_bIsControllingBot())
				{
					UINT *hWeapons = (UINT*)((DWORD)local_player + 0x2DF8);
					if (hWeapons) 
					{
						player_info_t pLocalInfo;
						interfaces::engine->GetPlayerInfo(interfaces::engine->GetLocalPlayer(), &pLocalInfo);
						
						for (int i = 0; hWeapons[i]; i++) 
						{
							CBaseAttributableItem* pWeapon = (CBaseAttributableItem*)interfaces::ent_list->GetClientEntityFromHandle((HANDLE)hWeapons[i]);
							if (!pWeapon)
								continue;
							int nWeaponIndex = *pWeapon->GetItemDefinitionIndex();
							if (g_ViewModelCFG.find(pWeapon->GetModelIndex()) != g_ViewModelCFG.end())
							{
								pWeapon->SetModelIndex(interfaces::model_info->GetModelIndex(g_ViewModelCFG[pWeapon->GetModelIndex()]));
							}
							
							if (!interfaces::pinput->m_fCameraInThirdPerson)
								skinchanger.apply_viewmodel(local_player, pWeapon, nWeaponIndex);

							if (pLocalInfo.xuidlow != *pWeapon->GetOriginalOwnerXuidLow())
								continue;
							if (pLocalInfo.xuidhigh != *pWeapon->GetOriginalOwnerXuidHigh())
								continue;

							skinchanger.apply_skins(pWeapon, nWeaponIndex);
							*pWeapon->GetAccountID() = pLocalInfo.xuidlow;

						}
					}
				}
				
				resolver_setup::GetInst().FSN(pEntity, curStage);
			}
		}

		if (curStage == FRAME_RENDER_START)
		{
		//	if (interfaces::pinput->m_fCameraInThirdPerson)

			for (int i = 1; i <= 65; i++)
			{
			//	AnimFix_ghetto();
				if (i == interfaces::engine->GetLocalPlayer()) continue;
				IClientEntity* pEnt = interfaces::ent_list->get_client_entity(i);
				if (!pEnt || !pEnt->cs_player())
					continue;

				if (pEnt->team() == local_player->team())
					continue;

				*(int*)((uintptr_t)pEnt + 0xA30) = interfaces::globals->framecount;
				*(int*)((uintptr_t)pEnt + 0xA28) = 0;
			} 

			bool lucky_is_a_tranny = false;

			if (options::menu.visuals.nosmoke.getstate())
			{
				for (auto matName : vistasmoke_mats)
				{
					IMaterial* mat = interfaces::materialsystem->FindMaterial(matName, "Other textures");
					mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

					*(int*)smoke_count = 0;
					lucky_is_a_tranny - true;
				}
			}

			if (lucky_is_a_tranny && !options::menu.visuals.nosmoke.getstate())
			{
				for (auto matName : vistasmoke_mats)
				{
					IMaterial* mat = interfaces::materialsystem->FindMaterial(matName, "Other textures");
					mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

					lucky_is_a_tranny = false;
				}
			}

		}


	}
	oFrameStageNotify(curStage);
}
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local || !interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

	auto zoom = options::menu.visuals.OtherFOV.getvalue();

	if (local->IsScoped() && options::menu.visuals.RemoveZoom.getstate() && !(GetAsyncKeyState(options::menu.visuals.zoom.GetKey()))) {
		zoom += 90.0f - pSetup->fov;
	}

	if (GetAsyncKeyState(options::menu.visuals.zoom.GetKey()))
	{
		zoom += 20 - pSetup->fov;
	}
	pSetup->fov += zoom;

	
	if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !interfaces::pinput->m_fCameraInThirdPerson)
	{
		pSetup->origin.z = hackManager.pLocal()->GetAbsOrigin().z + 64.f;
	}
	
	oOverrideView(ecx, edx, pSetup);
}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		if (!localplayer)
			return;
		if (options::menu.visuals.Active.getstate())
			fov = options::menu.visuals.OtherViewmodelFOV.getvalue();
	}
}
float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();
	GetViewModelFOV(fov);
	return fov;
}
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);
	IClientEntity* pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}

