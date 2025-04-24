// File:        UI3tButton.cpp
// Description: Button with 3 texutres (for <enabled>, <disabled> and <touched> states)
// Created:     07.12.2004
// Author:      Serhiy 0. Vynnychenk0
// Mail:        narrator@gsc-game.kiev.ua
//
// copyright 2004 GSC Game World
//

#pragma once
#include "UIButton.h"
#include "UIIBStatic.h"

class CUI3tButton : public CUIButton
{
	friend class CUIXmlInit;
	using CUIButton::SetTextColor;
public:
					CUI3tButton					();
	virtual			~CUI3tButton				();
	// appearance
	using CUIButton::Init;

	virtual void 	Init						(float x, float y, float width, float height);
	virtual void 	InitTexture					(LPCSTR tex_name);
	virtual void 	InitTexture					(LPCSTR tex_enabled, LPCSTR tex_disabled, LPCSTR tex_touched, LPCSTR tex_highlighted);	

			void 	SetTextColor				(u32 color);
			void 	SetTextColorH				(u32 color);
			void 	SetTextColorD				(u32 color);
			void 	SetTextColorT				(u32 color);
			void 	SetBaseTextColor			(u32 color);
			void 	SetBaseTextColorH			(u32 color);
			void 	SetBaseTextColorD			(u32 color);
			void 	SetBaseTextColorT			(u32 color);
	virtual void 	SetTextureOffset			(float x, float y);
	virtual void 	SetBaseTextureOffset		(float x, float y);
	virtual void 	SetTextureOffset			(Fvector2 offset);
	virtual void 	SetBaseTextureOffset		(Fvector2 offset);
	virtual void 	SetWidth					(float width);
	virtual void 	SetHeight					(float height);
	virtual void 	InitSoundH					(LPCSTR sound_file);
	virtual void 	InitSoundT					(LPCSTR sound_file);
	virtual void 	SetBtnStaticClrE			(u32 clr);
	virtual void 	SetBtnStaticClrD			(u32 clr);
	virtual void 	SetBtnStaticClrT			(u32 clr);
	virtual void 	SetBtnStaticClrH			(u32 clr);

	virtual void 	OnClick						();
	virtual void 	OnFocusReceive				();
	virtual void	OnFocusLost					();

	// check button
	bool			GetCheck					() const {return m_eButtonState == BUTTON_PUSHED;}
	void			SetCheck					(bool ch) {m_eButtonState = ch ? BUTTON_PUSHED : BUTTON_NORMAL;}
	
	// behavior
	virtual void	DrawTexture					();
	virtual void	Update						();
	virtual void	AddStatic					();
	virtual void	SetStaticColorChanging		(bool status) { m_BtnStaticParams.m_bNeedClrChanging = status; }
	CUIStatic*		GetBtnStatic				() { return m_BtnStatic; }
	
	//virtual void Enable(bool bEnable);	
	virtual bool 	OnMouseAction				(float x, float y, EUIMessages mouse_action);
	virtual bool 	OnMouseDown					(int mouse_btn);
			void 	SetCheckMode				(bool mode)		{m_bCheckMode = mode;}
			void 	SetAlwaysHighlight			(bool status)	{m_bAlwaysHighlight = status;}

	CUIStatic			m_hint;
	CUIIBStatic			m_background;
protected:
	bool				m_bCheckMode;
	bool				m_bWasAppliedBaseTexScaleUsing;
	bool				m_bAlwaysHighlight;
	struct sBtnStaticParams
	{
		bool			m_bNeedClrChanging;
		u32				m_ClrStateE;
		u32				m_ClrStateD;
		u32				m_ClrStateT;
		u32				m_ClrStateH;
	};
	sBtnStaticParams m_BtnStaticParams;
private:	
	virtual void	PlaySoundH					();
	virtual void	PlaySoundT					();

	ref_sound			m_sound_h;
	ref_sound			m_sound_t;
	CUIStatic*			m_BtnStatic;
};