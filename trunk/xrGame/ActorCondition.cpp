#include "pch_script.h"
#include "actorcondition.h"
#include "actor.h"
#include "actorEffector.h"
#include "inventory.h"
#include "level.h"
#include "sleepeffector.h"
#include "game_base_space.h"
#include "autosave_manager.h"
#include "xrserver.h"
#include "ai_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "game_object_space.h"
#include "ui\UIVideoPlayerWnd.h"
#include "object_broker.h"
#include "weapon.h"

#define ENEMIES_RADIUS				20.f

#define MAX_SATIETY					1.0f
#define START_SATIETY				0.5f

BOOL	GodMode	()	
{ 
	if (GameID() == GAME_SINGLE) 
		return psActorFlags.test(AF_GODMODE|AF_GODMODE_RT); 
	return FALSE;	
}

bool CActor::IsLimping()
{
	return m_entity_condition->IsLimping();
}

CActorCondition::CActorCondition(CActor *object) :
	inherited	(object)
{
	m_fJumpPower				= 0.f;
	m_fStandPower				= 0.f;
	m_fWalkPower				= 0.f;
	m_fJumpWeightPower			= 0.f;
	m_fWalkWeightPower			= 0.f;
	m_fOverweightWalkK			= 0.f;
	m_fOverweightJumpK			= 0.f;
	m_fAccelK					= 0.f;
	m_fSprintK					= 0.f;
	m_fAlcohol					= 0.f;
	m_fSatiety					= 1.0f;

	VERIFY						(object);
	m_object					= object;
	m_actor_sleep_wnd			= NULL;
	m_can_sleep_callback		= NULL;
	m_get_sleep_video_name_callback	= NULL;
	m_condition_flags.zero		();

}

CActorCondition::~CActorCondition(void)
{
	xr_delete					(m_actor_sleep_wnd);
	xr_delete					(m_can_sleep_callback);
	xr_delete					(m_get_sleep_video_name_callback);
}

void CActorCondition::LoadCondition(LPCSTR entity_section)
{
	inherited::LoadCondition(entity_section);

	LPCSTR						section = READ_IF_EXISTS(pSettings,r_string,entity_section,"condition_sect",entity_section);
	if(IsGameTypeSingle())
		m_change_v_sleep.load		(section,"_sleep");

	m_fJumpPower				= pSettings->r_float(section,"jump_power");
	m_fStandPower				= pSettings->r_float(section,"stand_power");
	m_fWalkPower				= pSettings->r_float(section,"walk_power");
	m_fJumpWeightPower			= pSettings->r_float(section,"jump_weight_power");
	m_fWalkWeightPower			= pSettings->r_float(section,"walk_weight_power");
	m_fOverweightWalkK			= pSettings->r_float(section,"overweight_walk_k");
	m_fOverweightJumpK			= pSettings->r_float(section,"overweight_jump_k");
	m_fAccelK					= pSettings->r_float(section,"accel_k");
	m_fSprintK					= pSettings->r_float(section,"sprint_k");

	//порог силы и здоровья меньше которого актер начинает хромать
	m_fLimpingHealthBegin		= pSettings->r_float(section,	"limping_health_begin");
	m_fLimpingHealthEnd			= pSettings->r_float(section,	"limping_health_end");
	R_ASSERT					(m_fLimpingHealthBegin<=m_fLimpingHealthEnd);

	m_fLimpingPowerBegin		= pSettings->r_float(section,	"limping_power_begin");
	m_fLimpingPowerEnd			= pSettings->r_float(section,	"limping_power_end");
	R_ASSERT					(m_fLimpingPowerBegin<=m_fLimpingPowerEnd);

	m_fCantWalkPowerBegin		= pSettings->r_float(section,	"cant_walk_power_begin");
	m_fCantWalkPowerEnd			= pSettings->r_float(section,	"cant_walk_power_end");
	R_ASSERT					(m_fCantWalkPowerBegin<=m_fCantWalkPowerEnd);

	m_fCantSprintPowerBegin		= pSettings->r_float(section,	"cant_sprint_power_begin");
	m_fCantSprintPowerEnd		= pSettings->r_float(section,	"cant_sprint_power_end");
	R_ASSERT					(m_fCantSprintPowerBegin<=m_fCantSprintPowerEnd);

	m_fPowerLeakSpeed			= pSettings->r_float(section,"max_power_leak_speed");
	if(IsGameTypeSingle())
		m_fK_SleepMaxPower		= pSettings->r_float(section,"max_power_leak_speed_sleep");
	
	m_fV_Alcohol				= pSettings->r_float(section,"alcohol_v");

//. ???	m_fSatietyCritical			= pSettings->r_float(section,"satiety_critical");
	m_fV_Satiety				= pSettings->r_float(section,"satiety_v");		
	m_fV_SatietyPower			= pSettings->r_float(section,"satiety_power_v");
	m_fV_SatietyHealth			= pSettings->r_float(section,"satiety_health_v");
	
	m_MaxWalkWeight					= pSettings->r_float(section,"max_walk_weight");

	LPCSTR cb_name			= READ_IF_EXISTS(pSettings,r_string,section,"can_sleep_callback","");

	if(cb_name && xr_strlen(cb_name)){
		m_can_sleep_callback		= xr_new<CScriptCallbackEx<LPCSTR> >();
		luabind::functor<LPCSTR>		f;
		R_ASSERT					(ai().script_engine().functor<LPCSTR>(cb_name,f));
		m_can_sleep_callback->set	(f);
	}
	cb_name					= READ_IF_EXISTS(pSettings,r_string,section,"sleep_video_name_callback","");

	if(cb_name && xr_strlen(cb_name)){
		m_get_sleep_video_name_callback		= xr_new<CScriptCallbackEx<LPCSTR> >();
		luabind::functor<LPCSTR>			fl;
		R_ASSERT							(ai().script_engine().functor<LPCSTR>(cb_name,fl));
		m_get_sleep_video_name_callback->set(fl);
	}
}


//вычисление параметров с ходом времени
#include "UI.h"
#include "HUDManager.h"

void CActorCondition::UpdateCondition()
{
	if (GodMode())				return;
	if (!object().g_Alive())	return;
	if (!object().Local() && m_object != Level().CurrentViewEntity())		return;
	
	float base_weight			= object().MaxCarryWeight();
	float cur_weight			= object().inventory().TotalWeight();

	if ((object().mstate_real&mcAnyMove))
	{
		ConditionWalk( cur_weight / base_weight,
			isActorAccelerated( object().mstate_real,object().IsZoomAimingMode() ),
			(object().mstate_real&mcSprint) != 0 );
	}
	else
	{
		ConditionStand( cur_weight / base_weight );
	}
	
	if( IsGameTypeSingle() )
	{
		float k_max_power = 1.0f;
		if( !IsSleeping() )
		{
			k_max_power = 1.0f + _min(cur_weight, base_weight) / base_weight
				+ _max(0.0f, (cur_weight - base_weight) / 10.0f);
		}
		else
		{
			k_max_power = 1.0f;
		}
		SetMaxPower		(GetMaxPower() - m_fPowerLeakSpeed*m_fDeltaTime*k_max_power);
	}


	m_fAlcohol		+= m_fV_Alcohol*m_fDeltaTime;
	clamp			(m_fAlcohol,			0.0f,		1.0f);

	if ( IsGameTypeSingle() )
	{	
		CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effAlcohol);
		if	((m_fAlcohol>0.0001f) ){
			if(!ce){
				AddEffector(m_object,effAlcohol, "effector_alcohol", GET_KOEFF_FUNC(this, &CActorCondition::GetAlcohol));
			}
		}else{
			if(ce)
				RemoveEffector(m_object,effAlcohol);
		}

		
		CEffectorPP* ppe = object().Cameras().GetPPEffector((EEffectorPPType)effPsyHealth);
		
		string64			pp_sect_name;
		shared_str ln		= Level().name();
		strconcat			(sizeof(pp_sect_name),pp_sect_name, "effector_psy_health", "_", *ln);
		if(!pSettings->section_exist(pp_sect_name))
			strcpy_s			(pp_sect_name, "effector_psy_health");

		if	( !fsimilar(GetPsyHealth(), 1.0f, 0.05f) )
		{
			if(!ppe)
			{
				AddEffector(m_object,effPsyHealth, pp_sect_name, GET_KOEFF_FUNC(this, &CActorCondition::GetPsy));
			}
		}else
		{
			if(ppe)
				RemoveEffector(m_object,effPsyHealth);
		}
		if(fis_zero(GetPsyHealth()))
			health() =0.0f;
	};

	UpdateSatiety				();

	inherited::UpdateCondition	();

	if( IsGameTypeSingle() )
		UpdateTutorialThresholds();
}


void CActorCondition::UpdateSatiety()
{
	if (!IsGameTypeSingle()) return;

	float k = 1.0f;
	if(m_fSatiety>0)
	{
		m_fSatiety -=	m_fV_Satiety*
						k*
						m_fDeltaTime;
	
		clamp			(m_fSatiety,		0.0f,		1.0f);

	}
		
	//сытость увеличивает здоровье только если нет открытых ран
	if(!m_bIsBleeding)
	{
		m_fDeltaHealth += CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT) ? 
					(m_fV_SatietyHealth*(m_fSatiety>0.0f?1.f:-1.f)*m_fDeltaTime)
					: 0;
	}

	//коэффициенты уменьшения восстановления силы от сытоти и радиации
	float radiation_power_k		= 1.f;
	float satiety_power_k		= 1.f;
			
	m_fDeltaPower += m_fV_SatietyPower*
				radiation_power_k*
				satiety_power_k*
				m_fDeltaTime;
}


CWound* CActorCondition::ConditionHit(SHit* pHDS)
{
	if (GodMode()) return NULL;
	return inherited::ConditionHit(pHDS);
}

//weight - "удельный" вес от 0..1
void CActorCondition::ConditionJump(float weight)
{
	float power			=	m_fJumpPower;
	power				+=	m_fJumpWeightPower*weight*(weight>1.f?m_fOverweightJumpK:1.f);
	m_fPower			-=	HitPowerEffect(power);
}
void CActorCondition::ConditionWalk(float weight, bool accel, bool sprint)
{	
	float power			=	m_fWalkPower;
	power				+=	m_fWalkWeightPower*weight*(weight>1.f?m_fOverweightWalkK:1.f);
	power				*=	m_fDeltaTime*(accel?(sprint?m_fSprintK:m_fAccelK):1.f);
	m_fPower			-=	HitPowerEffect(power);
}

void CActorCondition::ConditionStand(float weight)
{	
	float power			= m_fStandPower;
	power				*= m_fDeltaTime;
	m_fPower			-= power;
}


bool CActorCondition::IsCantWalk() const
{
	if(m_fPower< m_fCantWalkPowerBegin)
		m_bCantWalk		= true;
	else if(m_fPower > m_fCantWalkPowerEnd)
		m_bCantWalk		= false;
	return				m_bCantWalk;
}

#include "CustomOutfit.h"

bool CActorCondition::IsCantWalkWeight()
{
	if(IsGameTypeSingle() && !GodMode())
	{
		float max_w	= m_object->MaxWalkWeight();

		if( object().inventory().TotalWeight() > max_w )
		{
			return true;
		}
	}
	return false;
}

bool CActorCondition::IsCantSprint() const
{
	if(m_fPower< m_fCantSprintPowerBegin)
		m_bCantSprint	= true;
	else if(m_fPower > m_fCantSprintPowerEnd)
		m_bCantSprint	= false;
	return				m_bCantSprint;
}

bool CActorCondition::IsLimping() const
{
	if(m_fPower< m_fLimpingPowerBegin || GetHealth() < m_fLimpingHealthBegin)
		m_bLimping = true;
	else if(m_fPower > m_fLimpingPowerEnd && GetHealth() > m_fLimpingHealthEnd)
		m_bLimping = false;
	return m_bLimping;
}
extern bool g_bShowHudInfo;

bool CActorCondition::AllowSleep ()
{
	EActorSleep result		= CanSleepHere		();
	return( !stricmp(ACTOR_DEFS::easCanSleepResult, result)  );
}

EActorSleep CActorCondition::GoSleep(ALife::_TIME_ID sleep_time, bool without_check)
{
	if (IsSleeping()) return ACTOR_DEFS::easCanSleepResult;

	EActorSleep result = without_check?ACTOR_DEFS::easCanSleepResult : CanSleepHere();
	if( 0 != stricmp(ACTOR_DEFS::easCanSleepResult, result)  ) 
			return result;

	g_bShowHudInfo				= false;
	m_bIsSleeping				= true;

//.	ProcessSleep				(sleep_time);// change conditions

	std::swap					(m_change_v_sleep,		m_change_v);
	std::swap					(m_fK_SleepMaxPower,	m_fPowerLeakSpeed);

	

	object().mstate_wishful	&=		~mcAnyMove;
	object().mstate_real	&=		~mcAnyMove;


	//ïîñòàâèòü áóäèëüíèê
	object().m_dwWakeUpTime = Level().GetGameTime() + sleep_time;

	VERIFY	(m_object == smart_cast<CActor*>(Level().CurrentEntity()));

	m_object->Cameras().RemovePPEffector(EEffectorPPType(SLEEP_EFFECTOR_TYPE_ID));
	object().m_pSleepEffectorPP = xr_new<CSleepEffectorPP>(object().m_pSleepEffector->ppi,
													object().m_pSleepEffector->time,
													object().m_pSleepEffector->time_attack,
													object().m_pSleepEffector->time_release);

	m_object->Cameras().AddPPEffector(object().m_pSleepEffectorPP);

	m_object->callback(GameObject::eActorSleep)( m_object->lua_game_object() );


	m_actor_sleep_wnd			= xr_new<CUIActorSleepVideoPlayer>();
	m_actor_sleep_wnd->Init		( (*m_get_sleep_video_name_callback)() );

	return ACTOR_DEFS::easCanSleepResult;
}

void CActorCondition::Awoke()
{
	if(!IsSleeping())		return;

	m_bIsSleeping			= false;

	std::swap				(m_change_v_sleep,		m_change_v);
	std::swap				(m_fK_SleepMaxPower,	m_fPowerLeakSpeed);

	if ( ai().get_alife() ) {
		NET_Packet		P;
		P.w_begin		(M_SWITCH_DISTANCE);
		P.w_float		(object().m_fOldOnlineRadius);
		Level().Send	(P,net_flags(TRUE,TRUE));
	}

	m_actor_sleep_wnd->DeActivate	();
	xr_delete						(m_actor_sleep_wnd);

	VERIFY(m_object == smart_cast<CActor*>(Level().CurrentEntity()));
	VERIFY(object().m_pSleepEffectorPP);

	object().m_pSleepEffectorPP->m_eSleepState = CSleepEffectorPP::AWAKING;
	object().m_pSleepEffectorPP = NULL;

	g_bShowHudInfo			= true;
	
}

//ïðîâåðêà ìîæåì ëè ìû ñïàòü íà ýòîì ìåñòå
EActorSleep CActorCondition::CanSleepHere()
{
	if(m_can_sleep_callback && *m_can_sleep_callback){
		if(!stricmp((*m_can_sleep_callback)(), easCanSleepResult)){
			Fvector pos;
			pos.set(object().Position());
			pos.y += 0.1f;

			xr_vector<CObject*> NearestList;
			Level().ObjectSpace.GetNearest	(NearestList, pos, ENEMIES_RADIUS, &object()); 

			for(xr_vector<CObject*>::iterator it = NearestList.begin();
											NearestList.end() != it;
											it++)
			{
				CEntityAlive* entity = smart_cast<CEntityAlive*>(*it);
				if(entity && entity->g_Alive() && entity->is_relation_enemy(m_object))
					return "cant_sleep_near_enemies";
			}
			
			return easCanSleepResult;
		}else return (*m_can_sleep_callback)();
	}

	return easCanSleepResult;
}

void CActorCondition::save(NET_Packet &output_packet)
{
	inherited::save		(output_packet);
	save_data			(m_fAlcohol, output_packet);
	save_data			(m_condition_flags, output_packet);
	save_data			(m_fSatiety, output_packet);
}

void CActorCondition::load(IReader &input_packet)
{
	inherited::load		(input_packet);
	load_data			(m_fAlcohol, input_packet);
	load_data			(m_condition_flags, input_packet);
	load_data			(m_fSatiety, input_packet);
}

void CActorCondition::reinit	()
{
	inherited::reinit	();
	m_bLimping					= false;
	m_bIsSleeping				= false;
	m_fSatiety					= 1.f;
}

void CActorCondition::ChangeAlcohol	(float value)
{
	m_fAlcohol += value;
}

void CActorCondition::ChangeSatiety(float value)
{
	m_fSatiety += value;
	clamp		(m_fSatiety, 0.0f, 1.0f);
}

void CActorCondition::UpdateTutorialThresholds()
{
	string256						cb_name;
	static float _cPowerThr			= pSettings->r_float("tutorial_conditions_thresholds","power");
	static float _cPowerMaxThr		= pSettings->r_float("tutorial_conditions_thresholds","max_power");
	static float _cBleeding			= pSettings->r_float("tutorial_conditions_thresholds","bleeding");
	static float _cSatiety			= pSettings->r_float("tutorial_conditions_thresholds","satiety");
	static float _cRadiation		= pSettings->r_float("tutorial_conditions_thresholds","radiation");
	static float _cWpnCondition		= pSettings->r_float("tutorial_conditions_thresholds","weapon_jammed");
	static float _cPsyHealthThr		= pSettings->r_float("tutorial_conditions_thresholds","psy_health");



	bool b = true;
	if(b && !m_condition_flags.test(eCriticalPowerReached) && GetPower()<_cPowerThr){
		m_condition_flags.set			(eCriticalPowerReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_critical_power");
	}

	if(b && !m_condition_flags.test(eCriticalMaxPowerReached) && GetMaxPower()<_cPowerMaxThr){
		m_condition_flags.set			(eCriticalMaxPowerReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_critical_max_power");
	}

	if(b && !m_condition_flags.test(eCriticalBleedingSpeed) && BleedingSpeed()>_cBleeding){
		m_condition_flags.set			(eCriticalBleedingSpeed, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_bleeding");
	}

	if(b && !m_condition_flags.test(eCriticalSatietyReached) && GetSatiety()<_cSatiety){
		m_condition_flags.set			(eCriticalSatietyReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_satiety");
	}

	if(b && !m_condition_flags.test(eCriticalRadiationReached) && GetRadiation()>_cRadiation){
		m_condition_flags.set			(eCriticalRadiationReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_radiation");
	}
/*
	if(b && !m_condition_flags.test(ePhyHealthMinReached) && GetPsyHealth()>_cPsyHealthThr){
//.		m_condition_flags.set			(ePhyHealthMinReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_psy");
	}
*/
	if(b && !m_condition_flags.test(eCantWalkWeight) && IsCantWalkWeight()){
		m_condition_flags.set			(eCantWalkWeight, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_cant_walk_weight");
	}

	if(b && !m_condition_flags.test(eWeaponJammedReached)&&m_object->inventory().GetActiveSlot()!=NO_ACTIVE_SLOT){
		PIItem item							= m_object->inventory().ItemFromSlot(m_object->inventory().GetActiveSlot());
		CWeapon* pWeapon					= smart_cast<CWeapon*>(item); 
		if(pWeapon&&pWeapon->GetCondition()<_cWpnCondition){
			m_condition_flags.set			(eWeaponJammedReached, TRUE);b=false;
			strcpy_s(cb_name,"_G.on_actor_weapon_jammed");
		}
	}
	
	if(!b){
		luabind::functor<LPCSTR>			fl;
		R_ASSERT							(ai().script_engine().functor<LPCSTR>(cb_name,fl));
		fl									();
	}
}
