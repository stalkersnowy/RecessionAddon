#include "stdafx.h"
#include "weaponBM16.h"
#include "WeaponHUD.h"
#include "entity.h"
#include "actor.h"

CWeaponBM16::~CWeaponBM16()
{
	HUD_SOUND::DestroySound(m_sndReload1);
}

void CWeaponBM16::Load	(LPCSTR section)
{
	inherited::Load		(section);

	animGet	(mhud_reload1,	pSettings->r_string(*hud_sect,"anim_reload_1"));
	animGet	(mhud_shot1,	pSettings->r_string(*hud_sect,"anim_shoot_1"));
	animGet	(mhud_idle1,		pSettings->r_string(*hud_sect,"anim_idle_1"));
	animGet	(mhud_idle2,		pSettings->r_string(*hud_sect,"anim_idle_2"));
	animGet	(mhud_zoomed_idle1,		pSettings->r_string(*hud_sect,"anim_zoomed_idle_1"));
	animGet	(mhud_zoomed_idle2,		pSettings->r_string(*hud_sect,"anim_zoomedidle_2"));
	animGet	(mhud_draw1,		pSettings->r_string(*hud_sect,"anim_draw_1"));
	animGet	(mhud_draw2,		pSettings->r_string(*hud_sect,"anim_draw_2"));
	animGet	(mhud_holster1,		pSettings->r_string(*hud_sect,"anim_holster_1"));
	animGet	(mhud_holster2,		pSettings->r_string(*hud_sect,"anim_holster_2"));
	animGet	(mhud_sprint1,		pSettings->r_string(*hud_sect,"anim_idle_sprint_1"));
	animGet	(mhud_sprint2,		pSettings->r_string(*hud_sect,"anim_idle_sprint_2"));


	HUD_SOUND::LoadSound(section, "snd_reload_1", m_sndReload1, m_eSoundShot);

}

void CWeaponBM16::PlayReloadSound()
{
	if(m_magazine.size()==1)	PlaySound	(m_sndReload1,get_LastFP());
	else						PlaySound	(sndReload,get_LastFP());
}

void CWeaponBM16::PlayAnimShoot()
{
	if(m_magazine.size()==1)
		m_pHUD->animPlay(random_anim(mhud_shot1),TRUE,this,GetState());
	else
		m_pHUD->animPlay(random_anim(mhud.mhud_shots),TRUE,this,GetState());
}

void CWeaponBM16::PlayAnimReload()
{
	bool b_both = HaveCartridgeInInventory(2);

	VERIFY(GetState()==eReload);
	if(m_magazine.size()==1 || !b_both)
		m_pHUD->animPlay(random_anim(mhud_reload1),TRUE,this,GetState());
	else
		m_pHUD->animPlay(random_anim(mhud.mhud_reload),TRUE,this,GetState());

}

bool CWeaponBM16::TryPlayAnimIdle()
{
	VERIFY(GetState() == eIdle);
	if (!IsZoomed())
	{
		CActor* pActor = smart_cast<CActor*>(H_Parent());
		if (pActor)
		{
			CEntity::SEntityState st;
			pActor->g_State(st);
			if (st.bSprint)
			{
				switch (m_magazine.size())
				{
				case 0:{
					m_pHUD->animPlay(random_anim(mhud.mhud_idle_sprint), TRUE, NULL, GetState());
				}break;
				case 1:{
					m_pHUD->animPlay(random_anim(mhud_sprint1), TRUE, NULL, GetState());
				}break;
				case 2:{
					m_pHUD->animPlay(random_anim(mhud_sprint2), TRUE, NULL, GetState());
				}break;
				}
				return true;
			}
		}
	}
	return false;
}

void CWeaponBM16::PlayAnimIdle()
{
	if(TryPlayAnimIdle())	return;

	if(IsZoomed())
	{
		switch (m_magazine.size())
		{
		case 0:{
			m_pHUD->animPlay(random_anim(mhud.mhud_idle_aim), TRUE, NULL, GetState());
		}break;
		case 1:{
			m_pHUD->animPlay(random_anim(mhud_zoomed_idle1), TRUE, NULL, GetState());
		}break;
		case 2:{
			m_pHUD->animPlay(random_anim(mhud_zoomed_idle2), TRUE, NULL, GetState());
		}break;
		};
	}else{
		switch (m_magazine.size())
		{
		case 0:{
			m_pHUD->animPlay(random_anim(mhud.mhud_idle), TRUE, NULL, GetState());
		}break;
		case 1:{
			m_pHUD->animPlay(random_anim(mhud_idle1), TRUE, NULL, GetState());
		}break;
		case 2:{
			m_pHUD->animPlay(random_anim(mhud_idle2), TRUE, NULL, GetState());
		}break;
		};
	}
}

void CWeaponBM16::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);
	switch (m_magazine.size())
	{
	case 0:{
		m_pHUD->animPlay(random_anim(mhud.mhud_show),FALSE,this,GetState());
	}break;
	case 1:{
		m_pHUD->animPlay(random_anim(mhud_draw1),FALSE,this,GetState());
	}break;
	case 2:{
		m_pHUD->animPlay(random_anim(mhud_draw2),FALSE,this,GetState());
	}break;
	}
}

void CWeaponBM16::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);
	switch (m_magazine.size())
	{
	case 0:{
		m_pHUD->animPlay (random_anim(mhud.mhud_hide),TRUE,this,GetState());
	}break;
	case 1:{
		m_pHUD->animPlay (random_anim(mhud_holster1),TRUE,this,GetState());
	}break;
	case 2:{
		m_pHUD->animPlay (random_anim(mhud_holster2),TRUE,this,GetState());
	}break;
	}
}