#pragma once
#include "UIWindow.h"
#include "alife_space.h"

class CUIXml;
class CUIStatic;

class CUIArtefactParams :public CUIWindow
{
public:
								CUIArtefactParams		();
	virtual						~CUIArtefactParams		();
	void 						InitFromXml				(CUIXml& xml_doc);
	bool 						Check					(const shared_str& af_section);
	void 						SetInfo					(const shared_str& af_section);

protected:
	enum{
		_item_start						=0,
		_item_health_restore_speed		=_item_start,
		_item_radiation_restore_speed,
		_item_satiety_restore_speed,
		_item_power_restore_speed,
		_item_bleeding_restore_speed,

		_max_item_index,

	};
	CUIStatic*					m_info_items_restore[_max_item_index];
	CUIStatic*					m_info_items_immunity[ALife::eHitTypeMax];
	CUIStatic*					m_additional_weight;
};