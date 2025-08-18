////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.h
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameObject.h"
#include "../xr_3da/feel_touch.h"
#include "game_graph_space.h"
//#include "script_export_space.h"

class CLevelChanger : public CGameObject, public Feel::Touch {
private:
	typedef	CGameObject	inherited;

private:
	GameGraph::_GRAPH_ID	m_game_vertex_id;
	u32						m_level_vertex_id;
	Fvector					m_position;
	Fvector					m_angles;
	float					m_entrance_time;
	shared_str				m_invite_str;

	void					update_actor_invitation	();
	bool					m_bSilentMode;
	bool					m_bEnabled;
	bool					get_reject_pos(Fvector& p, Fvector& r);
public:
	virtual				~CLevelChanger		();
	virtual BOOL		net_Spawn			(CSE_Abstract* DC);
	virtual void		net_Destroy			();
	virtual void		net_Export			(NET_Packet& P);
	virtual void		net_Import			(NET_Packet& P);
	virtual void		Center				(Fvector& C) const;
	virtual float		Radius				() const;
	virtual void		shedule_Update		(u32 dt);
	virtual void		feel_touch_new		(CObject* O);
	virtual BOOL		feel_touch_contact	(CObject* O);

	virtual bool		IsVisibleForZones() { return false;		}
	void				EnableLevelChanger	(bool b)				{m_bEnabled=b;}
	bool				IsLevelChangerEnabled() const				{return m_bEnabled;}
	void				SetLEvelChangerInvitationStr(LPCSTR str)	{m_invite_str = str;}

//	DECLARE_SCRIPT_REGISTER_FUNCTION
};
/*add_to_type_list(CLevelChanger)
#undef script_type_list
#define script_type_list save_type_list(CLevelChanger)*/
