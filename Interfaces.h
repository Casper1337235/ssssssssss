#pragma once

#include "CommonIncludes.h"
#include "SDK.h"
#include "CClientState.h"
#include "glow_outline_effect.h"
#include "IClientMode.h"
#include <d3d9.h>
#include "CUtlVector.h"

class IMoveHelper : public Interface
{
public:
	bool bFirstRunOfFunctions : 1;
	bool bGameCodeMovedPlayer : 1;
	int nPlayerHandle; // edict index on server, client entity handle on client=
	int nImpulseCommand; // Impulse command issued.
	Vector vecViewAngles; // Command view angles (local space)
	Vector vecAbsViewAngles; // Command view angles (world space)
	int nButtons; // Attack buttons.
	int nOldButtons; // From host_client->oldbuttons;
	float flForwardMove;
	float flSideMove;
	float flUpMove;
	float flMaxSpeed;
	float flClientMaxSpeed;
	Vector vecVelocity; // edict::velocity // Current movement direction.
	Vector vecAngles; // edict::angles
	Vector vecOldAngles;
	float outStepHeight; // how much you climbed this move
	Vector outWishVel; // This is where you tried
	Vector outJumpVel; // This is your jump velocity
	Vector vecConstraintCenter;
	float flConstraintRadius;
	float flConstraintWidth;
	float flConstraintSpeedFactor;
	float flUnknown[5];
	Vector vecAbsOrigin;

	virtual	void _vpad() = 0;
	virtual void SetHost(IClientEntity* host) = 0;
};


class CMoveData {
public:
	bool m_bFirstRunOfFunctions : 1;
	bool m_bGameCodeMovedPlayer : 1;
	bool m_bNoAirControl : 1;

	unsigned long m_nPlayerHandle;
	int m_nImpulseCommand;
	QAngle m_vecViewAngles;
	QAngle m_vecAbsViewAngles;
	int m_nButtons;
	int m_nOldButtons;
	float m_flForwardMove;
	float m_flSideMove;
	float m_flUpMove;

	float m_flMaxSpeed;
	float m_flClientMaxSpeed;

	Vector m_vecVelocity;
	Vector m_vecOldVelocity;
	float somefloat;
	QAngle m_vecAngles;
	QAngle m_vecOldAngles;

	float m_outStepHeight;
	Vector m_outWishVel;
	Vector m_outJumpVel;

	Vector m_vecConstraintCenter;
	float m_flConstraintRadius;
	float m_flConstraintWidth;
	float m_flConstraintSpeedFactor;
	bool m_bConstraintPastRadius;

	void SetAbsOrigin(const Vector& vec);
	const Vector& GetOrigin() const;

private:
	Vector m_vecAbsOrigin;
};

class IPrediction : public Interface
{
public:
	void SetupMove(IClientEntity *player, CUserCmd *pCmd, IMoveHelper *pHelper, void *move)
	{
		typedef void(__thiscall* SetupMoveFn)(void*, IClientEntity*, CUserCmd*, IMoveHelper*, void*);
		return call_virtual_test<SetupMoveFn>(this, 20)(this, player, pCmd, pHelper, move);
	}	

	void	FinishMove(IClientEntity *player, CUserCmd *pCmd, void *move)
	{
		typedef void(__thiscall* FinishMoveFn)(void*, IClientEntity*, CUserCmd*, void*);
		return call_virtual_test<FinishMoveFn>(this, 21)(this, player, pCmd, move);
	}

	void set_local_viewangles_rebuilt(Vector& angle)
	{
		typedef void(__thiscall *o_updateClientSideAnimation)(void*, Vector&);
		call_vfunc<o_updateClientSideAnimation>(this, 13)(this, angle);
	}

	void CheckMovingGround(IClientEntity *player, double frametime)
	{
		using fnCheckMovingGround = void(__thiscall *)(void *, IClientEntity *, double);
		call_vfunc< fnCheckMovingGround >(this, 18)(this, player, frametime);
	}

	char pad00[8]; 					// 0x0000
	bool m_bInPrediction;				// 0x0008
	char pad01[1];					// 0x0009
	bool m_bEnginePaused;				// 0x000A
	char pad02[13];					// 0x000B
	bool m_bIsFirstTimePredicted;		// 0x0018
};

class IGameMovement : public Interface
{
public:
	void ProcessMovement(IClientEntity *pPlayer, void *pMove)
	{
		typedef void(__thiscall* ProcessMovementFn)(void*, IClientEntity*, void*);
		return VFunc<ProcessMovementFn>(1)(this, pPlayer, pMove);
	}

	void StartTrackPredictionErrors(IClientEntity *pPlayer)
	{
		typedef void(__thiscall *o_StartTrackPredictionErrors)(void*, void*);
		return VFunc<o_StartTrackPredictionErrors>(3)(this, pPlayer);
	}

	void FinishTrackPredictionErrors(IClientEntity *pPlayer)
	{
		typedef void(__thiscall *o_FinishTrackPredictionErrors)(void*, void*);
		return VFunc<o_FinishTrackPredictionErrors>(4)(this, pPlayer);
	}
};

class CGameMovement
{
public:
	virtual			~CGameMovement(void) {}

	virtual void	ProcessMovement(IClientEntity *pPlayer, CMoveData *pMove) = 0;
	virtual void	Reset(void) = 0;
	virtual void	StartTrackPredictionErrors(IClientEntity *pPlayer) = 0;
	virtual void	FinishTrackPredictionErrors(IClientEntity *pPlayer) = 0;
	virtual void	DiffPrint(char const *fmt, ...) = 0;

	virtual Vector const&	GetPlayerMins(bool ducked) const = 0;
	virtual Vector const&	GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector const&   GetPlayerViewOffset(bool ducked) const = 0;

	virtual bool			IsMovingPlayerStuck(void) const = 0;
	virtual IClientEntity*	GetMovingPlayer(void) const = 0;
	virtual void			UnblockPusher(IClientEntity* pPlayer, IClientEntity *pPusher) = 0;

	virtual void    SetupMovementBounds(CMoveData *pMove) = 0;
};


namespace interfaces
{
	void Initialise();
	extern IBaseClientDLL* client;
	extern IVEngineClient* engine;
	extern IPanel* panels;
	extern IClientEntityList* ent_list;
	extern ISurface* surface;
	extern IVDebugOverlay* DebugOverlay;
	extern IClientModeShared *ClientMode;
	extern CGlobalVarsBase *globals;
	extern DWORD *prediction_dword;
	extern CMaterialSystem* materialsystem;
	extern CVRenderView* render_view;
	extern IDirect3DDevice9 *g_pD3DDevice9;
	extern IVModelRender* model_render;
	extern CGlowObjectManager* glow_manager;
	extern CModelInfo* model_info;
	extern HWND window;
	extern IInputSystem* m_iInputSys;
	extern CUtlVectorSimple *g_ClientSideAnimationList;
	extern IEngineTrace* trace;
	extern Trace2* trace_2;
	extern IPhysicsSurfaceProps* phys_props;
	extern ICVar *cvar;
	extern CInput* pinput;
	extern IGameEventManager2 *event_manager;
	extern HANDLE __FNTHANDLE;
	extern IVEffects* effects;
	extern IVEffects* effects;
	extern IMoveHelper* movehelper;
	extern IPrediction *c_prediction;
	extern CClientState* client_state;
	extern IGameMovement* gamemovement;
	extern IViewRenderBeams* render_beams;
	extern	CGameMovement * c_gamemove;

};