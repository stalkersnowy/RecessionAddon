#pragma once

#include "UIWindow.h"
#include "UIOptionsItem.h"

class CUI3tButton;
class CUIFrameLineWnd;
class CUITrackButton;

enum ETrackBarMode
{
	eTrackBarModeInt,
	eTrackBarModeFloat,
	eTrackBarModeToken,
	eTrackBarModeBool
};

class CUITrackBar : public CUIWindow, public CUIOptionsItem 
{
	friend class CUITrackButton;
public:
					CUITrackBar				();
	// CUIOptionsItem
	virtual void 	SetCurrentValue			();
	virtual void 	SaveValue				();
	virtual bool 	IsChanged				();
	virtual void 	SaveBackUpValue			();
	virtual void 	Undo					();
	virtual void	Draw					();
	virtual void	Update					();
	virtual bool	OnMouseAction			(float x, float y, EUIMessages mouse_action);
	virtual	void 	OnMessage				(const char* message);
	// CUIWindow
			void	InitTrackBar			(Fvector2 pos, Fvector2 size);
	virtual void	Enable					(bool status);
			void	SetInvert				(bool v) { m_b_invert = v; }
			bool	GetInvert				() const { return m_b_invert; }
			void	SetStep					(float step);
			bool	GetCheck				() const;
			void	SetCheck				(bool b);
	CUI3tButton*	GetSlider				() { return m_pSlider; }

// modes code
			void	SetNumOfSigns			(int num_of_signs) { m_i_num_of_signs = num_of_signs; }
			void	SetTrackBarMode			(ETrackBarMode mode) { m_mode = mode; }
ETrackBarMode		GetTrackBarMode			() const { return m_mode; }
			bool	IsIntMode				() const { return m_mode == eTrackBarModeInt; }
			bool	IsFltMode				() const { return m_mode == eTrackBarModeFloat; }
			bool	IsTokenMode				() const { return m_mode == eTrackBarModeToken; }
			bool	IsBoolMode				() const { return m_mode == eTrackBarModeBool; }
			void	SetTokenValues			(xr_token* tokens);
			int		CurrentID				() const { return (m_i_val - 1); }
			void	SetCurrentID			(int val_id) { m_i_val = val_id + 1;}

			void	SetSliderText			(bool v) { m_b_slider_text = v; }
			bool	GetSliderText			() const { return m_b_slider_text; }
protected:
			void 	UpdatePos				();
			void 	UpdatePosRelativeToMouse();

	CUI3tButton*		m_pSlider;
	CUIFrameLineWnd*	m_pFrameLine;
	CUIFrameLineWnd*	m_pFrameLine_d;
	bool				m_b_invert;
	bool				m_b_mouse_capturer;
	int					m_i_num_of_signs;
	xr_token*			m_tokens;
	ETrackBarMode		m_mode;
	bool				m_b_slider_text;

	union{
		struct{
			float				m_f_val;
			float				m_f_max;
			float				m_f_min;
			float				m_f_step;
			float				m_f_back_up;
		};
		struct{
			int					m_i_val;
			int					m_i_max;
			int					m_i_min;
			int					m_i_step;
			int					m_i_back_up;
		};
	};
};