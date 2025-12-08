////////////////////////////////////////////////////////////////////////////
//	Module 		: line_edit_control.cpp
//	Created 	: 21.02.2008
//	Author		: Evgeniy Sokolov
//	Description : line edit control class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "line_edit_control.h"

#include "../xrCore/os_clipboard.h"
#include "../xrGame/object_broker.h"
#include "xr_input.h"

#include "edit_actions.h"
#include "buffer_vector.h"

ENGINE_API float g_console_sensitive = 0.15f;

namespace text_editor
{

static bool terminate_char( char c, bool check_space = false )
{
	switch ( c )
	{
		case ' ':
			return check_space;
		case '(':	case ')':
		case '{':	case '}':
		case '[':	case ']':
		case '<':	case '>':
		case '\'':	case '\"':
		case '=':	case '+':	case '-':	case '*':	case '\\':
		case '/':	case '&':	case '|':
		case '!':	case '@':	case '#':	case '~':	case '`':
		case '$':	case '%':	case '^':
		case ':':	case ';':
		case '?':	case ',':	case '.':
		case '_':
			return true;
	}
	return false;
}

// -------------------------------------------------------------------------------------------------

line_edit_control::line_edit_control( u32 str_buffer_size, bool cyrillic )
{
	m_edit_str	= NULL;
	m_inserted	= NULL;
	m_undo_buf	= NULL;
	m_buf0		= NULL;
	m_buf1		= NULL;
	m_buf2		= NULL;
	m_buf3		= NULL;

	m_bCyrillic = cyrillic;
	m_bMultiLine = false;

	for ( u32 i = 0; i < DIK_COUNT; ++i )
	{
		m_actions[i] = NULL;
	}

	init( str_buffer_size );

	update_key_states	();
}

line_edit_control::~line_edit_control()
{
	xr_free( m_edit_str );
	xr_free( m_inserted );
	xr_free( m_undo_buf );
	xr_free( m_buf0 );
	xr_free( m_buf1 );
	xr_free( m_buf2 );
	xr_free( m_buf3 );

	size_t const array_size	= sizeof(m_actions)/sizeof(m_actions[0]);
	buffer_vector<Base*>	actions(m_actions, array_size, &m_actions[0], &m_actions[0] + array_size);
	std::sort				(actions.begin(), actions.end());
	actions.erase			(
		std::unique(
			actions.begin(),
			actions.end()
		),
		actions.end()
	);
	delete_data				( actions );
}

static inline bool get_caps_lock_state	()
{
#if 0
	static bool first_time					= true;
	static bool is_windows_vista_or_later	= false;
	if ( first_time ) {
		first_time							= false;
		OSVERSIONINFO						version_info;
		ZeroMemory							( &version_info, sizeof(version_info) );
		version_info.dwOSVersionInfoSize	= sizeof(version_info);
		GetVersionEx						( &version_info );
		is_windows_vista_or_later			= version_info.dwMajorVersion >= 6;
	}

	if ( is_windows_vista_or_later )
		return		!!(GetKeyState(VK_CAPITAL) & 1);
	else
#else // #if 0
		return		false;
#endif // #if 0
}

void line_edit_control::update_key_states	()
{
	if (!pInput)
	{
		BOOL bCaptureInput = !strstr(Core.Params, "-i");
		//if (g_dedicated_server)
		//	bCaptureInput = FALSE;

		pInput = xr_new<CInput>(bCaptureInput);
	}
	m_key_state.zero( );

	set_key_state	( ks_LShift,	!!pInput->iGetAsyncKeyState(DIK_LSHIFT)		);
	set_key_state	( ks_RShift,	!!pInput->iGetAsyncKeyState(DIK_RSHIFT)		);
	set_key_state	( ks_LCtrl,		!!pInput->iGetAsyncKeyState(DIK_LCONTROL)	);
	set_key_state	( ks_RCtrl,		!!pInput->iGetAsyncKeyState(DIK_RCONTROL)	);
	set_key_state	( ks_LAlt,		!!pInput->iGetAsyncKeyState(DIK_LALT)		);
	set_key_state	( ks_RAlt,		!!pInput->iGetAsyncKeyState(DIK_RALT)		);
	set_key_state	( ks_CapsLock,	text_editor::get_caps_lock_state()			);
}

void line_edit_control::clear_states()
{
	m_edit_str[0]	= 0;
	clear_inserted();
	m_undo_buf[0]	= 0;

	m_buf0[0]		= 0;
	m_buf1[0]		= 0;
	m_buf2[0]		= 0;
	m_buf3[0]		= 0;

	m_cur_pos		= 0;
	m_select_start	= 0;
	m_p1			= 0;
	m_p2			= 0;

	m_accel				= 1.0f;
	m_cur_time			= 0.0f;
	m_rep_time			= 0.0f;
	m_last_frame_time	= 0;
	m_last_key_time		= 0.0f;
	m_last_changed_frame  = 0;

	m_hold_mode			= false;
	m_insert_mode		= false;
	m_repeat_mode		= false;
	m_mark				= false;
	m_cursor_view		= false;
	m_need_update		= false;
	m_unselected_mode	= false;

	update_key_states	( );
}

void line_edit_control::init( u32 str_buffer_size, init_mode mode, bool multiline_mode )
{
	m_buffer_size = str_buffer_size;
	clamp( m_buffer_size, (int)MIN_BUF_SIZE, (int)MAX_BUF_SIZE );
	m_bMultiLine = multiline_mode;

	xr_free( m_edit_str );	m_edit_str = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_inserted );	m_inserted = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_undo_buf );	m_undo_buf = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	
	xr_free( m_buf0 );		m_buf0 = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_buf1 );		m_buf1 = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_buf2 );		m_buf2 = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_buf3 );		m_buf3 = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );

	clear_states();

	for ( u32 i = 0; i < DIK_COUNT; ++i )
	{
		xr_delete( m_actions[i] );
		m_actions[i] = NULL;
	}

	if ( mode == im_read_only )
	{
		assign_callback( DIK_A     , ks_Ctrl, Callback( this, &line_edit_control::select_all_buf       ) );
		assign_callback( DIK_C     , ks_Ctrl, Callback( this, &line_edit_control::copy_to_clipboard    ) );
		assign_callback( DIK_INSERT, ks_Ctrl, Callback( this, &line_edit_control::copy_to_clipboard    ) );

		assign_callback( DIK_HOME  , ks_free, Callback( this, &line_edit_control::move_pos_home        ) );
		assign_callback( DIK_END   , ks_free, Callback( this, &line_edit_control::move_pos_end         ) );
		assign_callback( DIK_LEFT  , ks_free, Callback( this, &line_edit_control::move_pos_left        ) );
		assign_callback( DIK_RIGHT , ks_free, Callback( this, &line_edit_control::move_pos_right       ) );
		assign_callback( DIK_LEFT  , ks_Ctrl, Callback( this, &line_edit_control::move_pos_left_word   ) );
		assign_callback( DIK_RIGHT , ks_Ctrl, Callback( this, &line_edit_control::move_pos_right_word  ) );
	}
	else
	{
		assign_char_pairs( mode );

		assign_callback( DIK_INSERT, ks_free, Callback( this, &line_edit_control::flip_insert_mode     ) );
		assign_callback( DIK_A     , ks_Ctrl, Callback( this, &line_edit_control::select_all_buf       ) );
		assign_callback( DIK_Z     , ks_Ctrl, Callback( this, &line_edit_control::undo_buf             ) );

		assign_callback( DIK_C     , ks_Ctrl, Callback( this, &line_edit_control::copy_to_clipboard    ) );
		assign_callback( DIK_V     , ks_Ctrl, Callback( this, &line_edit_control::paste_from_clipboard ) );
		assign_callback( DIK_X     , ks_Ctrl, Callback( this, &line_edit_control::cut_to_clipboard     ) );

		assign_callback( DIK_INSERT, ks_Ctrl, Callback( this, &line_edit_control::copy_to_clipboard    ) );
		assign_callback( DIK_INSERT, ks_Shift,Callback( this, &line_edit_control::paste_from_clipboard ) );
		assign_callback( DIK_DELETE, ks_Shift,Callback( this, &line_edit_control::cut_to_clipboard     ) );

		assign_callback( DIK_HOME  , ks_free, Callback( this, &line_edit_control::move_pos_home        ) );
		assign_callback( DIK_END   , ks_free, Callback( this, &line_edit_control::move_pos_end         ) );
		assign_callback( DIK_LEFT  , ks_free, Callback( this, &line_edit_control::move_pos_left        ) );
		assign_callback( DIK_RIGHT , ks_free, Callback( this, &line_edit_control::move_pos_right       ) );
		assign_callback( DIK_LEFT  , ks_Ctrl, Callback( this, &line_edit_control::move_pos_left_word   ) );
		assign_callback( DIK_RIGHT , ks_Ctrl, Callback( this, &line_edit_control::move_pos_right_word  ) );

		assign_callback( DIK_BACK  , ks_free, Callback( this, &line_edit_control::delete_selected_back ) );
		assign_callback( DIK_DELETE, ks_free, Callback( this, &line_edit_control::delete_selected_forward ) );
		assign_callback( DIK_BACK  , ks_Ctrl, Callback( this, &line_edit_control::delete_word_back     ) );
		assign_callback( DIK_DELETE, ks_Ctrl, Callback( this, &line_edit_control::delete_word_forward  ) );

		assign_callback( DIK_LSHIFT, ks_Ctrl, Callback( this, &line_edit_control::SwitchKL ) );
		assign_callback( DIK_LSHIFT, ks_Alt, Callback( this, &line_edit_control::SwitchKL  ) );

	} // if mode

	create_key_state( DIK_LSHIFT  , ks_LShift );
	create_key_state( DIK_RSHIFT  , ks_RShift );
	create_key_state( DIK_LCONTROL, ks_LCtrl  );
	create_key_state( DIK_RCONTROL, ks_RCtrl  );
	create_key_state( DIK_LALT    , ks_LAlt   );
	create_key_state( DIK_RALT    , ks_RAlt   );

	if (m_bMultiLine)
	{
		assign_callback(DIK_UP	  , ks_free, Callback(this, &line_edit_control::move_pos_up));
		assign_callback(DIK_DOWN  , ks_free, Callback(this, &line_edit_control::move_pos_down));
	}
}

void line_edit_control::assign_char_pairs( init_mode mode )
{
	create_char_pair( DIK_NUMPAD0, '0', '0' );
	create_char_pair( DIK_NUMPAD1, '1', '1' );
	create_char_pair( DIK_NUMPAD2, '2', '2' );
	create_char_pair( DIK_NUMPAD3, '3', '3' );
	create_char_pair( DIK_NUMPAD4, '4', '4' );
	create_char_pair( DIK_NUMPAD5, '5', '5' );
	create_char_pair( DIK_NUMPAD6, '6', '6' );
	create_char_pair( DIK_NUMPAD7, '7', '7' );
	create_char_pair( DIK_NUMPAD8, '8', '8' );
	create_char_pair( DIK_NUMPAD9, '9', '9' );

	if ( mode == im_number_only )
	{
		create_char_pair( DIK_0, '0', '0' );
		create_char_pair( DIK_1, '1', '1' );
		create_char_pair( DIK_2, '2', '2' );
		create_char_pair( DIK_3, '3', '3' );
		create_char_pair( DIK_4, '4', '4' );
		create_char_pair( DIK_5, '5', '5' );
		create_char_pair( DIK_6, '6', '6' );
		create_char_pair( DIK_7, '7', '7' );
		create_char_pair( DIK_8, '8', '8' );
		create_char_pair( DIK_9, '9', '9' );
		create_char_pair( DIK_NUMPADMINUS , '-', '-' );
		create_char_pair( DIK_MINUS       , '-', '-' );
		create_char_pair( DIK_NUMPADPLUS  , '+', '+' );
		create_char_pair( DIK_EQUALS	  , '+', '+' );
		return;
	}

	if ( mode != im_file_name_mode )
	{
		create_char_pair( DIK_0, '0', ')', true );
		create_char_pair( DIK_1, '1', '!', true );
		create_char_pair( DIK_2, '2', '@', true );
		create_char_pair( DIK_3, '3', '#', true );
		create_char_pair( DIK_4, '4', '$', true );
		create_char_pair( DIK_5, '5', '%', true );
		create_char_pair( DIK_6, '6', '^', true );
		create_char_pair( DIK_7, '7', '&', true );
		create_char_pair( DIK_8, '8', '*', true );
		create_char_pair( DIK_9, '9', '(', true );

		create_char_pair( DIK_BACKSLASH , '\\', '|', true );
		create_char_pair( DIK_LBRACKET  , '[' , '{', true );
		create_char_pair( DIK_RBRACKET  , ']' , '}', true );
		create_char_pair( DIK_APOSTROPHE, '\'', '\"',true );
		create_char_pair( DIK_COMMA     , ',' , '<', true );
		create_char_pair( DIK_PERIOD    , '.' , '>', true );
		create_char_pair( DIK_EQUALS    , '=' , '+', true );
		create_char_pair( DIK_SEMICOLON , ';' , ':', true );
		create_char_pair( DIK_SLASH     , '/' , '?', true );

		create_char_pair( DIK_NUMPADSTAR , '*', '*' );
		create_char_pair( DIK_NUMPADSLASH, '/', '/' );
	}
	else
	{
		create_char_pair( DIK_0, '0', '0' );
		create_char_pair( DIK_1, '1', '1' );
		create_char_pair( DIK_2, '2', '2' );
		create_char_pair( DIK_3, '3', '3' );
		create_char_pair( DIK_4, '4', '4' );
		create_char_pair( DIK_5, '5', '5' );
		create_char_pair( DIK_6, '6', '6' );
		create_char_pair( DIK_7, '7', '7' );
		create_char_pair( DIK_8, '8', '8' );
		create_char_pair( DIK_9, '9', '9' );
	}

	create_char_pair( DIK_NUMPADMINUS , '-', '-' );
	create_char_pair( DIK_NUMPADPLUS  , '+', '+' );
	create_char_pair( DIK_NUMPADPERIOD, '.', '.' );

	create_char_pair( DIK_MINUS       , '-', '_', true );
	create_char_pair( DIK_SPACE       , ' ', ' ' );
	create_char_pair( DIK_GRAVE       , '`', '~', true );

	create_char_pair( DIK_A, 'a', 'A', true );
	create_char_pair( DIK_B, 'b', 'B', true );
	create_char_pair( DIK_C, 'c', 'C', true );
	create_char_pair( DIK_D, 'd', 'D', true );
	create_char_pair( DIK_E, 'e', 'E', true );
	create_char_pair( DIK_F, 'f', 'F', true );
	create_char_pair( DIK_G, 'g', 'G', true );
	create_char_pair( DIK_H, 'h', 'H', true );
	create_char_pair( DIK_I, 'i', 'I', true );
	create_char_pair( DIK_J, 'j', 'J', true );
	create_char_pair( DIK_K, 'k', 'K', true );
	create_char_pair( DIK_L, 'l', 'L', true );
	create_char_pair( DIK_M, 'm', 'M', true );
	create_char_pair( DIK_N, 'n', 'N', true );
	create_char_pair( DIK_O, 'o', 'O', true );
	create_char_pair( DIK_P, 'p', 'P', true );
	create_char_pair( DIK_Q, 'q', 'Q', true );
	create_char_pair( DIK_R, 'r', 'R', true );
	create_char_pair( DIK_S, 's', 'S', true );
	create_char_pair( DIK_T, 't', 'T', true );
	create_char_pair( DIK_U, 'u', 'U', true );
	create_char_pair( DIK_V, 'v', 'V', true );
	create_char_pair( DIK_W, 'w', 'W', true );
	create_char_pair( DIK_X, 'x', 'X', true );
	create_char_pair( DIK_Y, 'y', 'Y', true );
	create_char_pair( DIK_Z, 'z', 'Z', m_bCyrillic );
}

void line_edit_control::create_key_state( u32 const dik, key_state state )
{
	Base* prev = m_actions[dik];
	//if ( m_actions[dik] )
	//{
	//	xr_delete( m_actions[dik] );
	//}
	m_actions[dik] = xr_new<text_editor::key_state_base>( state, prev );
}

void line_edit_control::create_char_pair( u32 const dik, char c, char c_shift, bool translate )
{
	if ( m_actions[dik] )
	{
		xr_delete( m_actions[dik] );
	}

	m_actions[dik] = xr_new<text_editor::type_pair>( dik, c, c_shift, translate );
}

void line_edit_control::assign_callback( u32 const dik, key_state state, Callback const& callback )
{
	VERIFY( dik < DIK_COUNT );
	Base* prev_action = m_actions[dik];
	m_actions[dik] = xr_new<text_editor::callback_base>( callback, state );
	m_actions[dik]->on_assign( prev_action );
}

void line_edit_control::insert_character( char c )
{
	m_inserted[0] = c;
}

void line_edit_control::clear_inserted()
{
	m_inserted[0] = m_inserted[1] = 0;
}

bool line_edit_control::empty_inserted()
{
	return (m_inserted[0] == 0);
}

void line_edit_control::set_edit( LPCSTR str )
{
	u32 str_size = xr_strlen( str );
	clamp( str_size, (u32)0, (u32)(m_buffer_size-1) );
	strncpy_s( m_edit_str, m_buffer_size, str, str_size );
	m_edit_str[str_size] = 0;

	m_cur_pos      = str_size;
	m_select_start = m_cur_pos;
	m_accel        = 1.0f;
	update_bufs();
}

// ========================================================

void line_edit_control::on_key_press( int dik )
{
	if ( DIK_COUNT <= dik )
	{
		return;
	}
	if ( !m_hold_mode )
	{
		m_last_key_time = 0.0f;
		m_accel = 1.0f;
	}
	m_mark = true;

	clamp_cur_pos();
	clear_inserted();
	compute_positions();

    if (m_actions[dik])
    {
        m_actions[dik]->on_key_press(this);
    }

    if (is_russian_layout() && !empty_inserted())
    {
        char c = m_inserted[0];
        bool is_upper = (c >= 'A' && c <= 'Z');
        char c_lower = is_upper ? c + ('a' - 'A') : c;

        if (const char* pos = strchr(latin_chars, c_lower))
        {
            int index = pos - latin_chars;
            char new_c = russian_chars[index];

            if (is_upper)
            {
                if ((u8)new_c >= 0xE0 && (u8)new_c <= 0xFF) {
                    new_c = (char)((u8)new_c - 0x20);
                }
            }
            m_inserted[0] = new_c;
        }
    }
	// ===========
	if ( dik == DIK_LCONTROL || dik == DIK_RCONTROL )
	{
		m_mark = false;	
	}
	
	m_edit_str[m_buffer_size-1] = 0;
	clamp_cur_pos();

	add_inserted_text();
	if ( m_mark && (!get_key_state( ks_Shift ) || !empty_inserted() ) )
	{
		m_select_start = m_cur_pos;
	}
	compute_positions();

	m_repeat_mode = false;
	m_rep_time    = 0.0f;
	
	update_key_states( );
	update_bufs();
}

// -------------------------------------------------------------------------------------------------

void line_edit_control::on_key_hold( int dik )
{
	update_key_states( );
	update_bufs();
	switch ( dik )
	{
	case DIK_TAB:
	case DIK_LSHIFT:   case DIK_RSHIFT:
	case DIK_LCONTROL: case DIK_RCONTROL:
	case DIK_LALT:     case DIK_RALT:
		return;
		break;
	}

	if ( m_repeat_mode && m_last_key_time > 5.0f * g_console_sensitive )
	{
		float buf_time = m_rep_time;
		m_hold_mode    = true;
		
		on_key_press( dik );
		
		m_hold_mode    = false;
		m_rep_time     = buf_time;
	}
}

void line_edit_control::on_key_release( int dik )
{
	m_accel         = 1.0f;
	m_rep_time      = 0.0f;
	m_last_key_time = 0.0f;

	update_key_states( );
	update_bufs		( );
}

void line_edit_control::on_frame()
{
	update_key_states	( );

	u32   fr_time = Device.dwTimeContinual;
	float dt      = (fr_time - m_last_frame_time) * 0.001f;
	if ( dt > 0.06666f )
	{
		dt = 0.06666f;
	}
	m_last_frame_time = fr_time;
	m_cur_time += dt;

	m_cursor_view = true;
	if ( m_cur_time > 0.3f ) { m_cursor_view = false; }
	if ( m_cur_time > 0.4f ) { m_cur_time = 0.0f; }

	m_rep_time += dt * m_accel;
	if ( m_rep_time > g_console_sensitive )//0.2
	{
		m_rep_time    = 0.0f;
		m_repeat_mode = true;
		m_accel       += 0.2f;
	}
	m_last_key_time += dt;

	if ( m_last_changed_frame + 1 < Device.dwFrame )
	{
		m_need_update = false;
	}
	
	/*if ( Device.dwFrame % 100 == 0 )
	{
	Msg( " cur_time=%.2f  re=%d  acc=%.2f   rep_time=%.2f", cur_time, bRepeat, fAccel, rep_time );
	}*/
}

void line_edit_control::update_bufs()
{
	//separate_buffer
	m_buf0[0] = 0;
	m_buf1[0] = 0;
	m_buf2[0] = 0;
	m_buf3[0] = 0;

	int edit_size = (int)xr_strlen( m_edit_str );
	int ds = (m_cursor_view && m_insert_mode && m_p2 < edit_size)? 1 : 0;
	strncpy_s( m_buf0, m_buffer_size, m_edit_str,             m_cur_pos             );
	strncpy_s( m_buf1, m_buffer_size, m_edit_str,             m_p1                  );
	strncpy_s( m_buf2, m_buffer_size, m_edit_str + m_p1,      m_p2 - m_p1 + ds      );
	strncpy_s( m_buf3, m_buffer_size, m_edit_str + m_p2 + ds, edit_size - m_p2 - ds );
	
	m_need_update = true;
	m_last_changed_frame = Device.dwFrame;
//	if ( m_cursor_view )	{
//		Msg( " m_p1=%d  m_p2=%d  cur=%d  sstart=%d", m_p1, m_p2, m_cur_pos, m_select_start );	}
}

void line_edit_control::add_inserted_text()
{
	if ( empty_inserted() )
	{
		return;
	}
	
	int old_edit_size = (int)xr_strlen( m_edit_str );
	for ( int i = 0; i < old_edit_size; ++i )
	{
		if ( !m_bMultiLine && ( m_edit_str[i] == '\n' ) || ( m_edit_str[i] == '\t' ) )
		{
			m_edit_str[i]=' ';
		}
	}

	PSTR buf = (PSTR)_alloca( (m_buffer_size + 1) * sizeof(char) );

	strncpy_s( buf,        m_buffer_size, m_edit_str,        m_p1        ); // part 1
	strncpy_s( m_undo_buf, m_buffer_size, m_edit_str + m_p1, m_p2 - m_p1 );

	int new_size = (int)xr_strlen( m_inserted );
	if ( m_buffer_size - 1 < m_p1 + new_size )
	{
		m_inserted[m_buffer_size - 1 - m_p1] = 0;
		new_size = xr_strlen( m_inserted );
	}
	strncpy_s( buf + m_p1, m_buffer_size, m_inserted, _min(new_size, m_buffer_size - m_p1) ); // part 2

	u8 ds = (m_insert_mode && m_p2 < old_edit_size)? 1 : 0;
	strncpy_s( buf + m_p1 + new_size, m_buffer_size, m_edit_str + m_p2 + ds,
		_min(old_edit_size - m_p2 - ds, m_buffer_size - m_p1 - new_size) ); // part 3
	buf[m_buffer_size] = 0;

	int szn = m_p1 + new_size + old_edit_size - m_p2 - ds;
	if ( szn < m_buffer_size )
	{
		strncpy_s( m_edit_str, m_buffer_size, buf, szn ); // part 1+2+3
		m_edit_str[m_buffer_size-1] = 0;
		m_cur_pos = m_p1 + new_size;
	}
	clamp_cur_pos();
}

//------------------------------------------------

void line_edit_control::copy_to_clipboard()
{
	if ( m_p1 >= m_p2 )
	{
		return;
	}
	u32 edit_len = xr_strlen( m_edit_str );
	PSTR buf = (PSTR)_alloca( (edit_len + 1) * sizeof(char) );
	strncpy_s( buf, edit_len + 1, m_edit_str + m_p1, m_p2 - m_p1 );
	buf[edit_len] = 0;
	os_clipboard::copy_to_clipboard( buf );
	m_mark = false;
}

void line_edit_control::paste_from_clipboard()
{
	os_clipboard::paste_from_clipboard( m_inserted, m_buffer_size-1 );
}

void line_edit_control::cut_to_clipboard()
{
	copy_to_clipboard();
	delete_selected_forward();
}

// =================================================================================================

void line_edit_control::undo_buf()
{
	xr_strcpy( m_inserted, m_buffer_size, m_undo_buf );
	m_undo_buf[0] = 0;
}

void line_edit_control::select_all_buf()
{
	m_select_start = 0;
	m_cur_pos = (int)xr_strlen( m_edit_str );
	m_mark = false;
}

void line_edit_control::flip_insert_mode()
{
	m_insert_mode = !m_insert_mode;
}


void line_edit_control::delete_selected_back()
{
	delete_selected( true );
}

void line_edit_control::delete_selected_forward()
{
	delete_selected( false );
}

void line_edit_control::delete_selected( bool back )
{
	clamp_cur_pos();
	int edit_len = (int)xr_strlen( m_edit_str );
	if ( edit_len > 0 )
	{
		if ( back )
		{
			u8 dp = ( (m_p1 == m_p2) && m_p1 > 0 )? 1 : 0;
			strncpy_s( m_undo_buf,             m_buffer_size, m_edit_str + m_p1 - dp, m_p2 - m_p1 + dp );
			strncpy_s( m_edit_str + m_p1 - dp, m_buffer_size, m_edit_str + m_p2,      edit_len - m_p2  );
			m_cur_pos = m_p1 - dp;
		}
		else
		{
			u8 dn = ( (m_p1 == m_p2) && m_p2 < edit_len )? 1 : 0;
			strncpy_s( m_undo_buf,        m_buffer_size, m_edit_str + m_p1,      m_p2 - m_p1 + dn     );
			strncpy_s( m_edit_str + m_p1, m_buffer_size, m_edit_str + m_p2 + dn, edit_len - m_p2 - dn );
			m_cur_pos = m_p1;
		}
		clamp_cur_pos();
	}
	m_select_start = m_cur_pos;
}

void line_edit_control::delete_word_back()
{
	bool const left_shift	= get_key_state(ks_LShift);
	bool const right_shift	= get_key_state(ks_RShift);
	set_key_state			( ks_Shift, true );

	move_pos_left_word		( );
	compute_positions		( );
	delete_selected			( true );

	set_key_state			( ks_LShift, left_shift );
	set_key_state			( ks_RShift, right_shift );
}

void line_edit_control::delete_word_forward()
{
	set_key_state( ks_Shift, true );
	move_pos_right_word();
	compute_positions();
	delete_selected( false );
	set_key_state( ks_Shift, false );
}

void line_edit_control::move_pos_home()
{
	m_cur_pos = 0;
}

void line_edit_control::move_pos_end()
{
	m_cur_pos = (int)xr_strlen( m_edit_str );
}

void line_edit_control::move_pos_left()
{
	if (m_cur_pos <= 0)
		return;

	// сн€тие выделени€ как раньше Ц по желанию
	if (m_p1 != m_p2 && !get_key_state(ks_Shift))
	{
		m_cur_pos = _min(m_p1, m_p2);
		m_select_start = m_cur_pos;
		m_mark = false;
	}
	else
	{
		--m_cur_pos;
	}

	m_cur_pos = normalize_cursor_pos(m_cur_pos, true);
}

void line_edit_control::move_pos_right()
{
	int len = (int)xr_strlen(m_edit_str);
	if (m_cur_pos >= len)
		return;

	if (m_p1 != m_p2 && !get_key_state(ks_Shift))
	{
		m_cur_pos = _max(m_p1, m_p2);
		m_select_start = m_cur_pos;
		m_mark = false;
	}
	else
	{
		++m_cur_pos;
	}

	m_cur_pos = normalize_cursor_pos(m_cur_pos, false);
}

void line_edit_control::move_pos_left_word()
{
	int i = m_cur_pos - 1;
	while ( i >= 0 && m_edit_str[i] == ' ' ) { --i; }
	if ( !terminate_char( m_edit_str[i] ) )
	{
		while ( i >= 0 && !terminate_char( m_edit_str[i], true )  ) { --i; }
		++i;
	}
	m_cur_pos = i;
}

void line_edit_control::move_pos_right_word()
{
	int edit_len = (int)xr_strlen( m_edit_str );
	int i = m_cur_pos + 1;
	while( i < edit_len && !terminate_char( m_edit_str[i], true ) )	{ ++i; }
	//while( i < edit_len && terminate_char( m_edit_str[i] ) )		{ ++i; }
	while( i < edit_len && m_edit_str[i] == ' ' )					{ ++i; }
	m_cur_pos = i;
}

void line_edit_control::compute_positions()
{
	m_p1 = m_cur_pos;
	m_p2 = m_cur_pos;
	if ( m_unselected_mode )
	{
		return;
	}

	if( m_cur_pos > m_select_start )
	{
		m_p1 = m_select_start;
	}
	else if( m_cur_pos < m_select_start )
	{
		m_p2 = m_select_start;
	}
}

void line_edit_control::clamp_cur_pos()
{
	clamp( m_cur_pos, 0, (int)xr_strlen( m_edit_str ) );
}

void line_edit_control::SwitchKL()
{
	ActivateKeyboardLayout( (HKL)HKL_NEXT, 0 );
}

static bool is_newline_token(const char* txt, int len, int pos_start, int& tok_start, int& tok_end)
{
	// "\n" = '\' 'n'
	if (pos_start < 0 || pos_start + 1 >= len)
		return false;

	if (txt[pos_start] == '\\' && txt[pos_start + 1] == 'n')
	{
		tok_start = pos_start;
		tok_end = pos_start + 1; // включительно
		return true;
	}
	return false;
}

static bool find_newline_token_at_pos(const char* txt, int len, int pos, int& tok_start, int& tok_end)
{
	// pos Ч "позици€ между символами": 0..len
	// недопустима€ позици€ дл€ каретки Ч между '\' и 'n', то есть pos == tok_start + 1
	for (int i = 0; i + 1 < len; ++i)
	{
		if (txt[i] == '\\' && txt[i + 1] == 'n')
		{
			if (pos == i + 1)
			{
				tok_start = i;
				tok_end = i + 1;
				return true;
			}
		}
	}
	return false;
}

static bool find_color_token_at_pos(const char* txt, int len, int pos, int& tok_start, int& tok_end)
{
	// токен: %c[ ... ]
	// допустимые позиции дл€ каретки: tok_start (перед '%') и tok_end+1 (после ']')
	// недопустимые: (tok_start < pos && pos <= tok_end+1)

	int i = 0;
	while (i < len)
	{
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;

			if (j >= len)
				break; // незакрытый тег Ц считаем обычным текстом

			int start = i;
			int end = j;   // индекс ']'

			// pos Ц позици€ между символами: 0..len
			// символы тега занимают индексы [start .. end]
			// недопустимо: start < pos && pos <= end+1
			if (pos > start && pos <= end + 1)
			{
				tok_start = start;
				tok_end = end;
				return true;
			}

			i = end + 1;
		}
		else
		{
			++i;
		}
	}

	return false;
}

int line_edit_control::normalize_cursor_pos(int pos, bool move_left) const
{
	LPCSTR txt = m_edit_str;
	int len = (int)xr_strlen(txt);

	if (pos < 0)   pos = 0;
	if (pos > len) pos = len;

	int tok_start, tok_end;

	// сначала провер€ем \n
	if (find_newline_token_at_pos(txt, len, pos, tok_start, tok_end))
	{
		// pos == tok_start+1 Ч между '\' и 'n'
		if (move_left)
			pos = tok_start;     // встать перед токеном
		else
			pos = tok_end + 1;   // встать после токена
		return pos;
	}

	// затем цвет
	if (find_color_token_at_pos(txt, len, pos, tok_start, tok_end))
	{
		if (move_left)
			pos = tok_start;
		else
			pos = tok_end + 1;
		return pos;
	}

	return pos;
}

void line_edit_control::move_pos_up()
{
	if (!m_bMultiLine)
		return;

	LPCSTR txt = m_edit_str;
	int len = (int)xr_strlen(txt);
	if (len <= 0)
		return;

	int cur = m_cur_pos;
	if (cur < 0)   cur = 0;
	if (cur > len) cur = len;

	struct LineInfo
	{
		int start;
		int end;
		int visual_len;
	};

	xr_vector<LineInfo> lines;
	lines.reserve(32);

	int line_start = 0;
	int visual_len = 0;
	int i = 0;

	// 1) –азбиваем текст на логические строки по "\n" или '\n'
	while (i < len)
	{
		int tok_len = 0;

		// перенос строки: "\n" (два символа) или насто€щий '\n'
		if (txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n')
		{
			tok_len = 2;
		}
		else if (txt[i] == '\n')
		{
			tok_len = 1;
		}

		if (tok_len > 0)
		{
			LineInfo li;
			li.start = line_start;
			li.end = i;
			li.visual_len = visual_len;
			lines.push_back(li);

			i += tok_len;
			line_start = i;
			visual_len = 0;
			continue;
		}

		// цветовой тег: %c[ ... ]
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j; // позици€ после ']'

			i = j;
			continue;
		}

		// обычный видимый символ
		++visual_len;
		++i;
	}

	// последн€€ строка
	{
		LineInfo li;
		li.start = line_start;
		li.end = len;
		li.visual_len = visual_len;
		lines.push_back(li);
	}

	if (lines.empty())
		return;

	// 2) Ќаходим, в какой строке сейчас курсор
	int cur_line = 0;
	for (u32 li = 0; li < lines.size(); ++li)
	{
		const LineInfo& L = lines[li];
		if (cur >= L.start && cur <= L.end)
			cur_line = (int)li;
		if (cur < L.start)
			break;
	}

	if (cur_line <= 0)
		return; // уже на первой строке

	const LineInfo& Lcur = lines[cur_line];

	// 3) —читаем Ђвидимуюї колонку в текущей строке
	int col = 0;
	i = Lcur.start;
	while (i < cur)
	{
		// переносы внутри строки здесь не ожидаютс€, но на вс€кий случай выходим
		if ((txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n') || txt[i] == '\n')
			break;

		// цветовой тег
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j;

			i = j;
			continue;
		}

		++col;
		++i;
	}

	// 4) ѕереносим эту колонку на предыдущую строку
	const LineInfo& Lprev = lines[cur_line - 1];
	int target_col = col;
	if (target_col > Lprev.visual_len)
		target_col = Lprev.visual_len;

	int new_pos = Lprev.start;
	int cur_col = 0;
	i = Lprev.start;
	while (i < Lprev.end)
	{
		if ((txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n') || txt[i] == '\n')
			break;

		// цветовой тег
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j;

			i = j;
			continue;
		}

		if (cur_col == target_col)
		{
			new_pos = i;
			break;
		}

		++cur_col;
		++i;
	}

	if (target_col == Lprev.visual_len)
		new_pos = Lprev.end;

	m_cur_pos = new_pos;
}

void line_edit_control::move_pos_down()
{
	if (!m_bMultiLine)
		return;

	LPCSTR txt = m_edit_str;
	int len = (int)xr_strlen(txt);
	if (len <= 0)
		return;

	int cur = m_cur_pos;
	if (cur < 0)   cur = 0;
	if (cur > len) cur = len;

	struct LineInfo
	{
		int start;
		int end;
		int visual_len;
	};

	xr_vector<LineInfo> lines;
	lines.reserve(32);

	int line_start = 0;
	int visual_len = 0;
	int i = 0;

	// 1) –азбиваем текст на строки
	while (i < len)
	{
		int tok_len = 0;

		if (txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n')
		{
			tok_len = 2;
		}
		else if (txt[i] == '\n')
		{
			tok_len = 1;
		}

		if (tok_len > 0)
		{
			LineInfo li;
			li.start = line_start;
			li.end = i;
			li.visual_len = visual_len;
			lines.push_back(li);

			i += tok_len;
			line_start = i;
			visual_len = 0;
			continue;
		}

		// цветовой тег
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j;

			i = j;
			continue;
		}

		++visual_len;
		++i;
	}

	{
		LineInfo li;
		li.start = line_start;
		li.end = len;
		li.visual_len = visual_len;
		lines.push_back(li);
	}

	if (lines.empty())
		return;

	// 2) Ќаходим текущую строку
	int cur_line = 0;
	for (u32 li = 0; li < lines.size(); ++li)
	{
		const LineInfo& L = lines[li];
		if (cur >= L.start && cur <= L.end)
			cur_line = (int)li;
		if (cur < L.start)
			break;
	}

	if (cur_line >= (int)lines.size() - 1)
		return; // уже на последней строке

	const LineInfo& Lcur = lines[cur_line];

	// 3) —читаем колонку в текущей строке
	int col = 0;
	i = Lcur.start;
	while (i < cur)
	{
		if ((txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n') || txt[i] == '\n')
			break;

		// цветовой тег
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j;

			i = j;
			continue;
		}

		++col;
		++i;
	}

	// 4) ѕереносим эту колонку на следующую строку
	const LineInfo& Lnext = lines[cur_line + 1];
	int target_col = col;
	if (target_col > Lnext.visual_len)
		target_col = Lnext.visual_len;

	int new_pos = Lnext.start;
	int cur_col = 0;
	i = Lnext.start;
	while (i < Lnext.end)
	{
		if ((txt[i] == '\\' && i + 1 < len && txt[i + 1] == 'n') || txt[i] == '\n')
			break;

		// цветовой тег
		if (txt[i] == '%' && i + 2 < len && txt[i + 1] == 'c' && txt[i + 2] == '[')
		{
			int j = i + 3;
			while (j < len && txt[j] != ']')
				++j;
			if (j < len && txt[j] == ']')
				++j;

			i = j;
			continue;
		}

		if (cur_col == target_col)
		{
			new_pos = i;
			break;
		}

		++cur_col;
		++i;
	}

	if (target_col == Lnext.visual_len)
		new_pos = Lnext.end;

	m_cur_pos = new_pos;
}

// -------------------------------------------------------------------------------------------------

void remove_spaces( PSTR str ) // in & out
{
	u32 str_size = xr_strlen( str );
	if ( str_size < 1 )
	{
		return;
	}
	PSTR new_str = (PSTR)_alloca( (str_size + 1) * sizeof(char) );
	new_str[0] = 0;

	u32 a = 0, b = 0, i = 0;
	while ( b < str_size )
	{
		a = b;
		while ( a < str_size && str[a] == ' ' ) { ++a; }
		b = a;
		while ( b < str_size && str[b] != ' ' ) { ++b; }
		strncpy_s( new_str + i, str_size+1, str + a, b - a );
		i += (b-a);
		if ( i < str_size ) 
		{
			new_str[i] = ' ';
		}
		++b;
		++i;
	}
	--i;
	if ( i < str_size )
	{
		strncpy_s( str, str_size, new_str, i );
	}
}

void split_cmd( PSTR first, PSTR second, LPCSTR str )
{
	first[0] = 0;
	second[0] = 0;
	u32 str_size = xr_strlen( str );
	if ( str_size < 1 )
	{
		return;
	}

	// split into =>>(cmd) (params)
	u32 a = 0;
	while ( a < str_size && str[a] != ' ' ) { ++a; }
	strncpy_s( first, str_size+1, str, a );
	if ( a < str_size )		{ first[a] = 0; }	else	{ first[str_size] = 0; }
	++a;
	if ( a < str_size )
	{
		strncpy_s( second, str_size+1, str + a, str_size - a );
		second[str_size - a] = 0;
	}
}

} // namespace text_editor