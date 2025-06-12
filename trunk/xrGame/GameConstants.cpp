#include "StdAfx.h"
#include "GameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"

bool	m_bMergeAmmoLineWithFiremode		= true;
bool	m_bCheckOverlapForPickup			= true;
bool	m_bEnableCheats						= false;
bool	m_bEnableSleeping					= false;
bool	m_bEnableDuplet						= false;
bool	m_bOldShotEffector					= false;

bool	m_bShowContactBio					= true;
bool	m_bShowPartnerWeightInCarBody		= false;
bool	m_bShowTrackBarValues				= true;
bool	m_bShowNumBeforeAnswers				= true;
bool	m_bShowSatietyInInventory			= false;
bool	m_bOldLuminosityBar					= false;
bool	m_bRussianPDATexture				= false;
bool	m_bMinimapZoom						= false;

namespace GameConstants
{
	void LoadConstants()
	{
		if (!pConstantsSettings)
		{
			Msg("# GameConstants file does not exists");
			return;
		}
		m_bMergeAmmoLineWithFiremode		= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "merge_ammo_line_with_firemode", m_bMergeAmmoLineWithFiremode);
		m_bCheckOverlapForPickup			= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "check_overlap_for_pickup", m_bCheckOverlapForPickup);
		m_bEnableCheats						= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "enable_cheats", m_bEnableCheats);
		m_bEnableSleeping					= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "enable_sleeping", m_bEnableSleeping);
		m_bEnableDuplet						= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "enable_duplet", m_bEnableDuplet);
		m_bOldShotEffector					= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "old_shot_effector", m_bOldShotEffector);

		m_bShowContactBio					= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_contact_bio", m_bShowContactBio);
		m_bShowPartnerWeightInCarBody		= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_partner_weight_in_carbody", m_bShowPartnerWeightInCarBody);
		m_bShowTrackBarValues				= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_track_bar_values", m_bShowTrackBarValues);
		m_bShowNumBeforeAnswers				= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_numbers_before_answers", m_bShowNumBeforeAnswers);
		m_bShowSatietyInInventory			= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_satiety_bar_in_inventory", m_bShowSatietyInInventory);
		m_bOldLuminosityBar					= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "old_luminosity_bar", m_bOldLuminosityBar);
		m_bRussianPDATexture				= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "russian_pda_texture", m_bRussianPDATexture);
		m_bMinimapZoom						= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "minimap_zoom", m_bMinimapZoom);

		Msg("# GameConstants are loaded");
	}

	bool GetMergedAmmoLineWithFiremodes()
	{
		return m_bMergeAmmoLineWithFiremode;
	}

	bool GetCheckOverlapForPickup()
	{
		return m_bCheckOverlapForPickup;
	}

	bool GetEnableCheats()
	{
		return m_bEnableCheats;
	}

	bool GetEnableSleeping()
	{
		return m_bEnableSleeping;
	}

	bool GetEnableDuplet()
	{
		return m_bEnableDuplet;
	}

	bool GetOldShotEffector()
	{
		return m_bOldShotEffector;
	}


	bool GetContactsBioShowing()
	{
		return m_bShowContactBio;
	}

	bool GetPartnerWeightInCarBodyShowing()
	{
		return m_bShowPartnerWeightInCarBody;
	}

	bool GetTrackBarValuesShowing()
	{
		return m_bShowTrackBarValues;
	}

	bool GetNumBeforeAnswersShowing()
	{
		return m_bShowNumBeforeAnswers;
	}

	bool GetSatietyBarInInventoryShowing()
	{
		return m_bShowSatietyInInventory;
	}

	bool GetOldLuminosityBar()
	{
		return m_bOldLuminosityBar;
	}

	bool GetRussianPDATexture()
	{
		return m_bRussianPDATexture;
	}

	bool GetMinimapZoom()
	{
		return m_bMinimapZoom;
	}
}
