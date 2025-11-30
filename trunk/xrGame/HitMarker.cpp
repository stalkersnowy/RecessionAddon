// exxZERO Time Stamp AddIn. Document modified at : Thursday, March 07, 2002 14:12:50 , by user : Oles , from computer : OLES
#include "stdafx.h"
#include "HitMarker.h"
#include "../xr_3da/Render.h"
#include "../xr_3da/LightAnimLibrary.h"
#include "UIStaticItem.h"
#include "grenade.h"

static Fvector2			as_PC[5];
static Fvector2			as_TC[5];
const static u32		as_id[4*3] = {0,1,4,  1,2,4,  2,3,4,  3,0,4};

//--------------------------------------------------------------------
CHitMarker::CHitMarker()
{
	InitShader_Grenade( pSettings->r_string( "hud_hitmark", "grenade_mark_texture" ) );
}

void CHitMarker::InitShader	(int index)
{
	if(index){
		string32		buf;
		xr_sprintf		(buf,"ui\\ui_hud_hit_mark_%02d",index);
		hShader2.create	("hud\\default", buf);
	}else{
		hShader2.create	("hud\\default", "ui\\ui_hud_hit_mark");
	}
}

void CHitMarker::InitShader_Grenade( LPCSTR tex_name )
{
	hShader_Grenade.create( "hud\\default", tex_name ); // "hud\\default2"
}

//--------------------------------------------------------------------
CHitMarker::~CHitMarker()
{
	while( m_HitMarks.size() ){
		xr_delete	( m_HitMarks.front() );
		m_HitMarks.pop_front	();
	}

	while( m_GrenadeMarks.size() )
	{
		xr_delete( m_GrenadeMarks.front() );
		m_GrenadeMarks.pop_front();
	}
} 
//--------------------------------------------------------------------
const static float fShowTime = 0.5f;
void CHitMarker::Render()
{
	float h1,p1;
	Device.vCameraDirection.getHP	(h1,p1);

	while( m_HitMarks.size() && !m_HitMarks.front()->IsActive() ){
		xr_delete	( m_HitMarks.front() );
		m_HitMarks.pop_front	();
	}

	while( m_GrenadeMarks.size() && !m_GrenadeMarks.front()->IsActive() )
	{
		xr_delete( m_GrenadeMarks.front() );
		m_GrenadeMarks.pop_front();
	}

	HITMARKS::iterator it = m_HitMarks.begin();
	HITMARKS::iterator it_e = m_HitMarks.end();
	for(;it!=it_e;++it)
		(*it)->Draw(-h1);
	
	GRENADEMARKS::iterator itg_b = m_GrenadeMarks.begin();
	GRENADEMARKS::iterator itg_e = m_GrenadeMarks.end();
	for( ; itg_b != itg_e; ++itg_b )
	{
		(*itg_b)->Draw( -h1 );
	}
}
//--------------------------------------------------------------------

void CHitMarker::Hit(int id, const Fvector& dir){

	Fvector hit_dir = dir;
	hit_dir.mul(-1.0f);
	m_HitMarks.push_back	(xr_new<SHitMark>(hShader2,hit_dir));
}
//--------------------------------------------------------------------

bool CHitMarker::AddGrenade_ForMark( CGrenade* grn )
{
	if ( !grn ) return false;
	u16 new_id = grn->ID();
	
	GRENADEMARKS::iterator	it_b = m_GrenadeMarks.begin();
	GRENADEMARKS::iterator	it_e = m_GrenadeMarks.end();

	for ( ; it_b != it_e; ++it_b )
	{
		if ( (*it_b)->removed_grenade ) continue;
		if ( (*it_b)->p_grenade->ID() == new_id ) return false;
	}
	
	m_GrenadeMarks.push_back( xr_new<SGrenadeMark>( hShader_Grenade, grn ) );

	return true;
}

void CHitMarker::Update_GrenadeView( Fvector& pos_actor )
{
	GRENADEMARKS::iterator	it_b = m_GrenadeMarks.begin();
	GRENADEMARKS::iterator	it_e = m_GrenadeMarks.end();

	for ( ; it_b != it_e; ++it_b )
	{
		if ( (*it_b)->removed_grenade ) continue;

		CGrenade* grn = (*it_b)->p_grenade;
		if( grn->IsExploding() )
		{
			(*it_b)->removed_grenade = true;
			continue;
		}

		Fvector pos_grn, dir;
		grn->Center( pos_grn );
		
		dir.sub( pos_grn, pos_actor );
		//dir.sub( pos_actor, pos_grn );
		dir.normalize();

		(*it_b)->Update( dir.getH() );
	}

}
//--------------------------------------------------------------------

void CHitMarker::net_Relcase( CObject* obj )
{
	u16 remove_id = obj->ID();

	GRENADEMARKS::iterator	it_b = m_GrenadeMarks.begin();
	GRENADEMARKS::iterator	it_e = m_GrenadeMarks.end();

	for ( ; it_b != it_e; ++it_b )
	{
		if ( (*it_b)->removed_grenade ) continue;

		if ( (*it_b)->p_grenade->ID() == remove_id )
		{
			(*it_b)->removed_grenade = true;
			//break;
		}
	}//for
}



SHitMark::SHitMark		(const ref_shader& sh, const Fvector& dir)
{
	m_StartTime							= Device.fTimeGlobal;
	m_lanim								= LALib.FindItem("hud_hit_mark");
	m_HitDirection						= dir.getH();
	m_UIStaticItem						= xr_new<CUIStaticItem>();
	m_UIStaticItem->SetShader			(sh);
	m_UIStaticItem->SetPos				(256.0f, 128.0f);
	m_UIStaticItem->SetRect				(.0f, .0f, UI_BASE_WIDTH / 2.f, UI_BASE_WIDTH / 2.f);
}

void SHitMark::UpdateAnim	()
{
	int frame;
	u32 clr							= m_lanim->CalculateRGB(Device.fTimeGlobal-m_StartTime,frame);
	m_UIStaticItem->SetTextureColor	(subst_alpha(m_UIStaticItem->GetTextureColor(), color_get_A(clr)));
}

SHitMark::~SHitMark		()
{
	xr_delete(m_UIStaticItem);
}

bool	SHitMark::IsActive()
{
	return ((Device.fTimeGlobal-m_StartTime) < m_lanim->Length_sec());
}

void	SHitMark::Draw(float cam_dir)
{
	UpdateAnim						();

	float res_h						= cam_dir + m_HitDirection;
	m_UIStaticItem->Render			(res_h);
}

// ######################################################################################

SGrenadeMark::SGrenadeMark( const ref_shader& sh, CGrenade* grn )
{
	p_grenade						= grn;
	removed_grenade					= false;
	m_LastTime						= Device.fTimeGlobal;
	m_LightAnim						= LALib.FindItem( "hud_hit_mark" );
	m_Angle							= 0.0f;

	m_UIStaticItem					= xr_new<CUIStaticItem>();
	m_UIStaticItem->SetShader		( sh );
	float xs = 640.0f;
	float ys = 640.0f;
	m_UIStaticItem->SetPos			( ( UI_BASE_WIDTH - xs ) * 0.5f, ( UI_BASE_HEIGHT - ys ) * 0.5f );
	m_UIStaticItem->SetRect			(.0f, .0f, xs, ys);
}

SGrenadeMark::~SGrenadeMark()
{
	xr_delete( m_UIStaticItem );
}

void SGrenadeMark::Update( float angle )
{
	m_Angle = angle;
	m_LastTime = Device.fTimeGlobal;
}

bool SGrenadeMark::IsActive() const
{
	return ( 2.0f*( Device.fTimeGlobal - m_LastTime ) < m_LightAnim->Length_sec() );
}

void SGrenadeMark::Draw( float cam_dir )
{
	int frame;
	u32 clr = m_LightAnim->CalculateRGB( 2.0f*(Device.fTimeGlobal - m_LastTime), frame );
	m_UIStaticItem->SetTextureColor( subst_alpha( m_UIStaticItem->GetTextureColor(), color_get_A(clr) ) );

	m_UIStaticItem->Render( cam_dir + m_Angle );
}



//--------------------------------------------------------------------
CHitMarkerOld::CHitMarkerOld()
{
}

void CHitMarkerOld::InitShader(int ind)
{
	ZeroMemory		(fHitMarks,sizeof(float)*4);
	as_PC[0].set	(-0.5f,-0.67f);
	as_PC[1].set	(0.5f,-0.67f);
	as_PC[2].set	(0.5f,0.67f);
	as_PC[3].set	(-0.5f,0.67f);
	as_PC[4].set	(0.f,0.f);
	as_TC[0].set	(0.f,0.f);
	as_TC[1].set	(1.f,0.f);
	as_TC[2].set	(0.f,0.f);
	as_TC[3].set	(1.f,0.f);
	as_TC[4].set	(.5f,1.f);

	hShader.create	("hud\\hitmarker","ui\\hud_hitmarker");
	hGeom.create	(FVF::F_TL, RCache.Vertex.Buffer(), NULL);
}
//--------------------------------------------------------------------
CHitMarkerOld::~CHitMarkerOld()
{
} 
//--------------------------------------------------------------------
void CHitMarkerOld::Render()
{
	if (fHitMarks[0]>0 || fHitMarks[1]>0 || fHitMarks[2]>0 || fHitMarks[3]>0)	
	{
		float		w_2		= float(::Render->getTarget()->get_width())/2;
		float		h_2		= float(::Render->getTarget()->get_height())/2;

		u32			dwOffset;
		FVF::TL* D		= (FVF::TL*)RCache.Vertex.Lock(12,hGeom.stride(),dwOffset);
		FVF::TL* Start	= D;
		for (int i=0; i<4; ++i){
			if (fHitMarks[i]>0){
				u32 Alpha = iFloor((fHitMarks[i]/float(fShowTime))*255.f);
				u32 Color = color_rgba(255,255,255,Alpha);
				const u32* idx = as_id+i*3;
				
				const Fvector2& P1 = as_PC[idx[0]]; const Fvector2& T1 = as_TC[idx[0]]; D->set((P1.x+1)*w_2,(P1.y+1)*h_2,.0001f,.9999f, Color, T1.x, T1.y); ++D;
				const Fvector2& P2 = as_PC[idx[1]]; const Fvector2& T2 = as_TC[idx[1]]; D->set((P2.x+1)*w_2,(P2.y+1)*h_2,.0001f,.9999f, Color, T2.x, T2.y); ++D;
				const Fvector2& P3 = as_PC[idx[2]]; const Fvector2& T3 = as_TC[idx[2]]; D->set((P3.x+1)*w_2,(P3.y+1)*h_2,.0001f,.9999f, Color, T3.x, T3.y); ++D;
				fHitMarks[i] -= Device.fTimeDelta;
			}
		}
		u32 Count				= (u32)(D - Start);
		RCache.Vertex.Unlock	(Count,hGeom.stride());
		if (Count)
		{
			RCache.set_Shader	(hShader);
			RCache.set_Geometry (hGeom);
			RCache.Render		(D3DPT_TRIANGLELIST,dwOffset,Count/3);
		}
	}
}
//--------------------------------------------------------------------

void CHitMarkerOld::Hit(int id, const Fvector& dir){
	VERIFY((id>=0)&&(id<8));
	switch (id){
	case 4:{
		fHitMarks[0] = fShowTime;
		fHitMarks[3] = fShowTime;
	}break;
	case 5:{
		fHitMarks[2] = fShowTime;
		fHitMarks[3] = fShowTime;
	}break;
	case 6:{
		fHitMarks[0] = fShowTime;
		fHitMarks[1] = fShowTime;
	}break;
	case 7:{
		fHitMarks[1] = fShowTime;
		fHitMarks[2] = fShowTime;
	}break;
	default:
		fHitMarks[id] = fShowTime;
	};
}
//--------------------------------------------------------------------