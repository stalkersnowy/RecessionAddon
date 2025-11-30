#ifndef __XR_HITMARKER_H__
#define __XR_HITMARKER_H__
#pragma once

class CUIStaticItem;
class CLAItem;
class CGrenade;

struct SHitMark{
	CUIStaticItem*	m_UIStaticItem;
	float			m_StartTime;
	float			m_HitDirection;
	CLAItem*		m_lanim;

					SHitMark		(const ref_shader& sh, const Fvector& dir);
					~SHitMark		();
	bool			IsActive		();
	void			UpdateAnim		();
	void			Draw			(float dir);
};

struct SGrenadeMark
{
	CGrenade*		p_grenade;
	bool			removed_grenade;

	CUIStaticItem*	m_UIStaticItem;
	float			m_LastTime;
	float			m_Angle;
	CLAItem*		m_LightAnim;

					SGrenadeMark( const ref_shader& sh, CGrenade* grn );
					~SGrenadeMark();

	bool			IsActive() const;
	void			Draw( float cam_dir );
	void			Update( float angle );

};


class IHitMarker
{
public:
	virtual void			Render		()								= 0;
	virtual void			Hit			(int id, const Fvector& dir)	= 0;
	virtual void			InitShader	(int index)						= 0;
	virtual bool			AddGrenade_ForMark( CGrenade* grn )			= 0;
	virtual void			Update_GrenadeView( Fvector& pos_actor )	= 0;
	virtual void			net_Relcase( CObject* obj )					= 0;
};


class CHitMarker : public IHitMarker
{
public:
/*
	float					fHitMarks[4];
	ref_shader				hShader;
	ref_geom				hGeom;
*/	
	typedef xr_deque<SHitMark*>		HITMARKS;
	typedef xr_deque<SGrenadeMark*>	GRENADEMARKS;

	ref_shader				hShader2;
	ref_shader				hShader_Grenade;

	HITMARKS				m_HitMarks;
	GRENADEMARKS			m_GrenadeMarks;

public:
							CHitMarker	();
							~CHitMarker	();

	virtual void			Render		();
	virtual void			Hit			(int id, const Fvector& dir);
	virtual bool			AddGrenade_ForMark( CGrenade* grn );
	virtual void			Update_GrenadeView( Fvector& pos_actor );

	virtual void			InitShader	(int index);
	void					InitShader_Grenade( LPCSTR tex_name );

	virtual void			net_Relcase( CObject* obj );
};


class CHitMarkerOld : public IHitMarker
{
public:
	float					fHitMarks[4];
	ref_shader				hShader;
	ref_geom				hGeom;
public:
							CHitMarkerOld	();
							~CHitMarkerOld	();

	virtual void			Render		();
	virtual void			Hit			(int id, const Fvector& dir);
	virtual void			InitShader	(int index);

	virtual bool			AddGrenade_ForMark( CGrenade* grn )			{return true;}
	virtual void			Update_GrenadeView( Fvector& pos_actor )	{}
	virtual void			net_Relcase( CObject* obj )					{}
};

#endif // __XR_HITMARKER_H__
