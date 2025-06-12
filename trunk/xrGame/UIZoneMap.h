#pragma once


#include "ui/UIStatic.h"

class CActor;
class CUICustomMap;
//////////////////////////////////////////////////////////////////////////


class CUIZoneMap
{
	CUICustomMap*				m_activeMap;
	float						m_fScale;
	u32							m_bMode;
	bool						m_bMinimapZoom;

	CUIStatic					m_background;
	CUIStatic					m_center;
	CUIStatic					m_compass;
	CUIStatic					m_clipFrame;
	CUIStatic					m_pointerDistanceText;

	Fvector2					m_zoom_text;

public:
								CUIZoneMap		();
	virtual						~CUIZoneMap		();

	void						SetHeading		(float angle);
	void						Init			();

	void						Render			();
	void						UpdateRadar		(Fvector pos);

	void						SetScale		(float s)							{m_fScale = s;}
	float						GetScale		()									{return m_fScale;}

	void						ResetZoomFactor	();
	void						ZoomIn			();
	void						ZoomOut			();

	CUIStatic&					Background		()									{return m_background;};
	void						SetupCurrentMap	();
};

