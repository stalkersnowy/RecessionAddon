#include "stdafx.h"
#include "poltergeist.h"

CPolterBoth::CPolterBoth(CPoltergeist* polter) : inherited(polter)
{
	time_tele_start		= 0;
	tele_enemy			= 0;
}

CPolterBoth::~CPolterBoth()
{
}

void CPolterBoth::load(LPCSTR section)
{
	m_particles_hidden			= pSettings->r_string(section,"Hidden_Particles");
	m_particles_idle			= pSettings->r_string(section,"Hide_Particles");

	LoadFlame(section);

	m_flame_delay.min			= pSettings->r_u32(section,"Delay_Flame_Min");
	m_flame_delay.normal		= pSettings->r_u32(section,"Delay_Flame_Normal");
	m_flame_delay.aggressive	= pSettings->r_u32(section,"Delay_Flame_Aggressive");

	m_tele_delay.min			= pSettings->r_u32(section,"Delay_Tele_Min");
	m_tele_delay.normal			= pSettings->r_u32(section,"Delay_Tele_Normal");
	m_tele_delay.aggressive		= pSettings->r_u32(section,"Delay_Tele_Aggressive");

	m_scare_delay.min			= pSettings->r_u32(section,"Delay_Scare_Min");
	m_scare_delay.normal		= pSettings->r_u32(section,"Delay_Scare_Normal");
	m_scare_delay.aggressive	= pSettings->r_u32(section,"Delay_Scare_Aggressive");
}

void CPolterBoth::on_reinit()
{
	time_tele_start		= 0;
	tele_enemy			= 0;
}

void CPolterBoth::update_schedule()
{
	UpdateFlame();
	UpdateTelekinesis();
}

void CPolterBoth::on_destroy()
{
	inherited::on_destroy();
	
	RemoveFlames();
}

void CPolterBoth::on_die()
{
	Fvector particles_position	= m_object->m_current_position;
	particles_position.y		+= m_object->target_height;

	CParticlesObject::Destroy		(m_particles_object_electro);
	CParticlesObject::Destroy		(m_particles_object);
}

#define TIME_SEEN_FOR_FIRE 5000

void CPoltergeist::PerformAttack(u32& flame, u32& tele, u32& scare)
{
	u32 time = Device.dwTimeGlobal;
	const CEntityAlive* target	= EnemyMan.get_enemy();
	
	bool agr = conditions().GetHealth() < 0.5f;
	
	CPolterBoth* action = smart_cast<CPolterBoth*>(m_both);

	if ((flame < time) && (EnemyMan.get_enemy_time_last_seen() + TIME_SEEN_FOR_FIRE > time)) {
		action->FireFlame(target);
		flame = time + Random.randI(action->m_flame_delay.min, (agr) ? action->m_flame_delay.aggressive : action->m_flame_delay.normal);
	}

	if (tele < time) {
		action->ProcessTelekinesis(target);
		tele = time + Random.randI(action->m_tele_delay.min, (agr) ? action->m_tele_delay.aggressive : action->m_tele_delay.normal);
	}

	if (scare < time) {
		if (Random.randI(2))
			PhysicalImpulse(target->Position());
		else 
			StrangeSounds(target->Position());
		
		scare = time + Random.randI(action->m_scare_delay.min, (agr) ? action->m_scare_delay.aggressive : action->m_scare_delay.normal);
	}
}