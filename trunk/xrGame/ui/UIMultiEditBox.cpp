#include "stdafx.h"

#include "UIMultiEditBox.h"
#include "UILines.h"
#include "uiabstract.h"
#include "../xr_3da/line_edit_control.h"
#include "../xr_3da/GameFont.h"
#include "../xr_3da/xr_input.h"

CUIMultiEditBox::CUIMultiEditBox() : inherited()
, m_cursor_dy(0.0f)
{
	Init((u32)EDIT_BUF_SIZE);
	if (m_pLines)
	{
		m_pLines->SetVTextAlignment(EVTextAlignment::valTop);
		m_pLines->SetUseNewLineMode(true);   // учитывать \n
		m_pLines->SetCutWordsMode(false);     // переносить слова по ширине
		m_pLines->SetColoringMode(true);     // переносить слова по ширине
	}
}

void  CUIMultiEditBox::Init(u32 max_char_count, bool number_only_mode, bool read_mode, bool fn_mode)
{
	if (read_mode)
	{
		m_editor_control->init(max_char_count, text_editor::im_read_only, true);
		m_editor_control->set_selected_mode(true);
		m_read_mode = true;
	}
	else
	{
		if (number_only_mode)
		{
			m_editor_control->init(max_char_count, text_editor::im_number_only, true);
		}
		else if (fn_mode)
		{
			m_editor_control->init(max_char_count, text_editor::im_file_name_mode, true);
		}
		else
		{
			m_editor_control->init(max_char_count, text_editor::im_standart, true);
		}
		m_editor_control->set_selected_mode(false);
		m_read_mode = false;
	}

	Register_callbacks();
	ClearText();

	m_bInputFocus = false;
}

void CUIMultiEditBox::Draw()
{
	VERIFY(m_pLines);

	Fvector2 pos, out;
	GetAbsolutePos(pos);
	CGameFont* font = m_pLines->GetFont();   // тот же шрифт, что и у текста
	R_ASSERT(font);

	if (ec().need_update() || m_force_update)
	{
		// 1. Обновляем текст для UILines — он сам сделает переноси и раскраску
		LPCSTR full_text = ec().str_edit();
		CUIStatic::SetText(full_text);

		// 2. Берём сырой текст ДО каретки
		LPCSTR cursor_str = ec().str_before_cursor();
		u32    len = xr_strlen(cursor_str);

		// 3. Ширина области (в экранных координатах)
		float width_scr = GetWidth();
		UI()->ClientToScreenScaledWidth(width_scr);

		float line_w_scr = 0.0f;                 // ширина текущей строки
		float line_h_scr = font->CurrentHeight_();
		float cur_y_scr = 0.0f;                 // смещение по Y (вниз)

		u32 i = 0;
		while (i < len)
		{
			// --- специфика: "\n" как два символа '\' 'n' ---
			if (cursor_str[i] == '\\' && (i + 1) < len && cursor_str[i + 1] == 'n')
			{
				cur_y_scr += line_h_scr;
				line_w_scr = 0.0f;
				i += 2;
				continue;
			}

			// --- на всякий случай: реальный '\n' ---
			if (cursor_str[i] == '\n')
			{
				cur_y_scr += line_h_scr;
				line_w_scr = 0.0f;
				++i;
				continue;
			}

			// --- цветовой тег: %c[ ... ] — СКРЫТЫЙ ЭЛЕМЕНТ ---
			if (cursor_str[i] == '%' &&
				(i + 2) < len &&
				cursor_str[i + 1] == 'c' &&
				cursor_str[i + 2] == '[')
			{
				// ищем закрывающую ']'
				u32 j = i + 3;
				while (j < len && cursor_str[j] != ']')
					++j;
				if (j < len && cursor_str[j] == ']')
					++j; // позиция ПОСЛЕ ']'

				// тег не даёт ширины, просто перепрыгиваем
				i = j;
				continue;
			}

			// --- обычный видимый символ ---
			char ch[2] = { cursor_str[i], 0 };
			float w_scr = font->SizeOf_(ch);    // ширина символа
			UI()->ClientToScreenScaledWidth(w_scr);

			// перенос по ширине контрола (по символам!)
			if (line_w_scr + w_scr > width_scr)
			{
				cur_y_scr += line_h_scr;
				line_w_scr = 0.0f;
			}

			line_w_scr += w_scr;
			++i;
		}

		// сохраняем позицию каретки в экранных координатах
		m_dx_cur = line_w_scr;
		m_cursor_dy = cur_y_scr;
		m_force_update = false;
	}

	// 4. Рисуем сам текст
	CUIStatic::Draw();

	// 5. Рисуем мигающую каретку
	if (ec().need_update() || m_force_update)
	{
		// 1. Обновляем текст в UILines
		LPCSTR full_text = ec().str_edit();
		CUIStatic::SetText(full_text);

		// 2. Строка до каретки
		LPCSTR cursor_str = ec().str_before_cursor();
		u32    len = xr_strlen(cursor_str);

		// 3. Ширина области в *UI-координатах*
		float width_ui = GetWidth() * 1.75f;

		float line_w_ui = 0.0f;
		float line_h_ui = font->CurrentHeight_();
		float cur_y_ui = 0.0f;

		u32 i = 0;
		while (i < len)
		{
			// "\n" как два символа
			if (cursor_str[i] == '\\' && i + 1 < len && cursor_str[i + 1] == 'n')
			{
				cur_y_ui += line_h_ui;
				line_w_ui = 0.0f;
				i += 2;
				continue;
			}

			// реальный '\n' — на всякий случай
			if (cursor_str[i] == '\n')
			{
				cur_y_ui += line_h_ui;
				line_w_ui = 0.0f;
				++i;
				continue;
			}

			// %c[ ... ] — скрытый элемент, его пропускаем
			if (cursor_str[i] == '%' && i + 2 < len &&
				cursor_str[i + 1] == 'c' && cursor_str[i + 2] == '[')
			{
				u32 j = i + 3;
				while (j < len && cursor_str[j] != ']')
					++j;
				if (j < len && cursor_str[j] == ']')
					++j; // после ']'

				i = j;
				continue;
			}

			// обычный видимый символ
			char ch[2] = { cursor_str[i], 0 };
			float w_ui = font->SizeOf_(ch);  // ширина символа в UI-координатах

			// перенос строки по ширине контрола
			if (line_w_ui + w_ui > width_ui)
			{
				cur_y_ui += line_h_ui;
				line_w_ui = 0.0f;
			}

			line_w_ui += w_ui;
			++i;
		}

		m_dx_cur = line_w_ui;  // всё в UI-координатах
		m_cursor_dy = cur_y_ui;
		m_force_update = false;
	}

	CUIStatic::Draw();

	if (m_bInputFocus && ec().cursor_view())
	{
		out.x = pos.x + GetTextX() + m_pLines->GetIndentByAlign();
		out.y = pos.y + GetTextY() + m_pLines->GetVIndentByAlign();
		UI()->ClientToScreenScaled(out);        // масштабируем *базу*

		out.x += m_dx_cur;                      // dx, dy уже в тех же единицах
		out.y += m_cursor_dy;

		font->Out(out.x, out.y, "_");
		font->OnRender();
	}
}

bool CUIMultiEditBox::OnKeyboardAction(int dik, EUIMessages action)
{
	if (!m_bInputFocus)
		return false;

	//if (action == WINDOW_KEY_PRESSED)
	//{
	//	switch (dik)
	//	{
	//	case DIK_LEFT:  MoveCursorLeft();  return true;
	//	case DIK_RIGHT: MoveCursorRight(); return true;
	//	case DIK_UP:    MoveCursorUp();    return true;
	//	case DIK_DOWN:  MoveCursorDown();  return true;
	//	}
	//}

	return inherited::OnKeyboardAction(dik, action);
}

bool CUIMultiEditBox::OnKeyboardHold(int dik)
{
	if (!m_bInputFocus)
		return false;

	//switch (dik)
	//{
	//case DIK_LEFT:  MoveCursorLeft();  return true;
	//case DIK_RIGHT: MoveCursorRight(); return true;
	//case DIK_UP:    MoveCursorUp();    return true;
	//case DIK_DOWN:  MoveCursorDown();  return true;
	//}

	return inherited::OnKeyboardHold(dik);
}

void CUIMultiEditBox::press_commit()
{
	if (!pInput->iGetAsyncKeyState(DIK_LSHIFT))
	{
		// текущий текст и позиция курсора
		LPCSTR  src = ec().str_edit();
		int     cur = ec().get_cursor_pos();
		int     len = (int)xr_strlen(src);

		clamp(cur, 0, len);

		xr_string text(src);
		// вставляем два символа: '\' и 'n'
		text.insert((u32)cur, "\\n");

		// line_edit_control::set_edit сам обрежет по своему buffer_size,
		// так что overflow по внутреннему буферу не будет.
		ec().set_edit(text.c_str());

		// сдвигаем курсор за вставленный "\n"
		int new_len = (int)text.size();
		int new_pos = cur + 2;
		if (new_pos > new_len)
			new_pos = new_len;

		ec().set_cursor_pos(new_pos);

		// форсим обновление отрисовки
		m_force_update = true;
	}
	else
	{
		m_bInputFocus = false;
		GetParent()->SetKeyboardCapture(this, false);
		GetMessageTarget()->SendMessage(this, EDIT_TEXT_COMMIT, NULL);
	}
}

void CUIMultiEditBox::press_escape()
{
	//if (xr_strlen(ec().str_edit()) != 0)
	//{
	//	if (!m_read_mode)
	//	{
	//		ec().set_edit("");
	//	}
	//}
	//else
	{
		GetParent()->SetKeyboardCapture(this, false);
		m_bInputFocus = false;
	}
}

static void split_lines(const shared_str& text, xr_vector<int>& line_start)
{
	line_start.clear();
	line_start.push_back(0);

	LPCSTR s = text.c_str();
	int idx = 0;

	while (*s)
	{
		if (*s == '\n')
			line_start.push_back(idx + 1);

		++idx;
		++s;
	}
}

void get_line_and_column(int cursor, const xr_vector<int>& starts, int& line, int& col)
{
	line = 0;
	for (u32 i = 0; i < starts.size(); i++)
	{
		if (cursor >= starts[i])
			line = i;
		else
			break;
	}
	col = cursor - starts[line];
}
