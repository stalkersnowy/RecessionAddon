///////////////////////////////////////////////////////////////
// Phrase.h
// класс, описывающий фразу (элемент диалога)
///////////////////////////////////////////////////////////////

#pragma once

#include "PhraseScript.h"

class CPhraseDialog;
class CGameObject;

struct SPhraseInfo
{
	shared_str	sIconName;
	bool		bUseIconLtx;
};

class CPhrase
{
private:
	friend CPhraseDialog;
public:
	CPhrase(void);
	virtual ~CPhrase(void);

	void				SetText				(LPCSTR text)			{m_text = text; }
	LPCSTR				GetText				()	const;

	void				SetSnd				(LPCSTR snd)			{m_snd = snd; }
	LPCSTR				GetSnd				()	const;

	void				SetID				(const shared_str& id)	{ m_ID = id; }
	const shared_str&	GetID				()	const				{ return m_ID; }

	int					GoodwillLevel		()	const				{ return m_iGoodwillLevel; }

	bool				IsDummy				()	const;

	CPhraseScript*		GetPhraseScript		()						{ return &m_PhraseScript; }
	shared_str			GetIconName			()	const				{ return m_sIconName; }
	bool				GetIconUsingLTX		()	const				{ return m_bUseIconLtx; }

	void				SetIconName			(shared_str s)			{ m_sIconName = s; }
	void				SetIconUsingLTX		(bool b)				{ m_bUseIconLtx = b; }

protected:
	//уникальный индекс в списке фраз диалога
	shared_str		m_ID;
	//текстовое представление фразы
	xr_string		m_text;
	//SNW: переназначение звука
	xr_string		m_snd;
	
	shared_str		m_sIconName;
	bool			m_bUseIconLtx;

	//минимальный уровень благосклонности, необходимый для того
	//чтоб фразу можно было сказать
	int				m_iGoodwillLevel;
	
	//для вызова скриптовых функций
	CPhraseScript	m_PhraseScript;
};