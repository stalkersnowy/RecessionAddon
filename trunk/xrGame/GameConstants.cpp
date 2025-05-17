#include "StdAfx.h"
#include "GameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"

bool	m_bDisableStopping					= true;
bool	m_bDisableStoppingBolt				= true;
bool	m_bDisableStoppingGrenade			= true;
bool	m_bMergeAmmoLineWithFiremode		= true;
bool	m_bCheckOverlapForPickup			= false;
bool	m_bEnableCheats						= false;

bool	m_bShowContactBio					= true;
bool	m_bShowPartnerWeightInCarBody		= false;
bool	m_bShowTrackBarValues				= false;
bool	m_bShowNumBeforeAnswers				= true;
bool	m_bShowSatietyInInventory			= false;

namespace GameConstants
{
	void LoadConstants()
	{
		if (!pConstantsSettings)
		{
			Msg("# GameConstants file does not exists");
			return;
		}
		m_bDisableStopping					= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_empty", true);
		m_bDisableStoppingBolt				= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_bolt", true);
		m_bDisableStoppingGrenade			= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_grenade", true);
		m_bMergeAmmoLineWithFiremode		= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "merge_ammo_line_with_firemode", true);
		m_bCheckOverlapForPickup			= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "check_overlap_for_pickup", false);
		m_bEnableCheats						= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "enable_cheats", false);

		m_bShowContactBio					= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_contact_bio", false);
		m_bShowPartnerWeightInCarBody		= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_partner_weight_in_carbody", false);
		m_bShowTrackBarValues				= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_track_bar_values", false);
		m_bShowNumBeforeAnswers				= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_numbers_before_answers", false);
		m_bShowSatietyInInventory			= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_satiety_bar_in_inventory", false);

		Msg("# GameConstants are loaded");
	}

	bool GetDisableStopping()
	{
		return m_bDisableStopping;
	}

	bool GetDisableStoppingBolt()
	{
		return m_bDisableStoppingBolt;
	}

	bool GetDisableStoppingGrenade()
	{
		return m_bDisableStoppingGrenade;
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
}
