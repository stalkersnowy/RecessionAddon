#pragma once

#include "UIEditBoxEx.h"

class CUIFrameWindow;

class CUIMultiEditBox : public CUIEditBoxEx
{
private:
	typedef CUIEditBoxEx inherited;
public:
					CUIMultiEditBox				();
	virtual			~CUIMultiEditBox			() {};

	virtual void	Draw						();
	virtual bool	OnKeyboardAction			(int dik, EUIMessages keyboard_action);
	virtual bool	OnKeyboardHold				(int dik);
	virtual	void	Init						(u32 max_char_count, bool number_only_mode = false, bool read_mode = false, bool fn_mode = false );
private:
	virtual void  	__stdcall	press_commit();
	virtual void  	__stdcall	press_escape();
	float           m_cursor_dy;    // вертикальное смещение каретки
};
