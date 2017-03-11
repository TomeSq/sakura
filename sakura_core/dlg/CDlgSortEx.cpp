/*
	Copyright (C) 2016, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "dlg/CDlgSortEx.h"
#include "_os/COsVersionInfo.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDOK,					HIDOK_SORTEX,
	IDCANCEL,				HIDCANCEL_SORTEX,
	IDC_BUTTON_HELP,		HIDC_SORTEX_BUTTON_HELP,
	IDC_RADIO_ASC,			HIDC_RADIO_ASC,
	IDC_RADIO_DESC,			HIDC_RADIO_DESC,
	IDC_RADIO_STRING,		HIDC_RADIO_STRING,
	IDC_RADIO_API1,			HIDC_RADIO_API1,
	IDC_RADIO_API2,			HIDC_RADIO_API2,
	IDC_RADIO_API3,			HIDC_RADIO_API3,
	IDC_RADIO_DECVALUE,		HIDC_RADIO_DECVALUE,
	IDC_RADIO_HEXVALUE,		HIDC_RADIO_HEXVALUE,
	IDC_CHECK_CASE,			HIDC_CHECK_CASE,
	IDC_CHECK_HIRAKATA,		HIDC_CHECK_HIRAKATA,
	IDC_CHECK_WIDTH,		HIDC_CHECK_WIDTH,
	IDC_CHECK_SPACE,		HIDC_CHECK_SPACE,
	IDC_CHECK_NUM,			HIDC_CHECK_NUM,
	0, 0
};


CDlgSortEx::CDlgSortEx()
{
	m_bOrderAsc = true;

	m_eMode = ESortExMode_String;
	
	m_bCaseIgnore = false;
	m_bHiraKataIgnore = false;
	m_bWidthIgnore = false;
	m_bSpaceIgnore = false;
	m_bStringSort  = false;
	m_bNumSort     = false;
}


int CDlgSortEx::DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam)
{
	return (int)CDialog::DoModal(hInstance, hwndParent, IDD_SORTEX, lParam);
}


void CDlgSortEx::SetData()
{
	CheckDlgButtonBool(GetHwnd(), IDC_RADIO_ASC, true);
	CheckDlgButtonBool(GetHwnd(), IDC_RADIO_STRING, true);

	HMODULE hModule = ::GetModuleHandle(_T("KERNEL32"));
	if( NULL == GetProcAddress(hModule, "CompareStringOrdinal") ){
		::EnableWindow(GetItemHwnd(IDC_RADIO_API2), FALSE);
	}
	SetEnableStirngCompareOption(FALSE);
}


int CDlgSortEx::GetData()
{
	m_bOrderAsc = IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_ASC);

	if( IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_API1) ){
		m_eMode = ESortExMode_API1;
	}else if( IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_API2) ){
		m_eMode = ESortExMode_API2;
	}else if( IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_API3) ){
		m_eMode = ESortExMode_API3;
	}else if( IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_DECVALUE) ){
		m_eMode = ESortExMode_Dec;
	}else if( IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_HEXVALUE) ){
		m_eMode = ESortExMode_Hex;
	}else{
		m_eMode = ESortExMode_String;
	}

	m_bCaseIgnore = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_CASE);
	m_bHiraKataIgnore = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_HIRAKATA);
	m_bWidthIgnore = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_WIDTH);
	m_bSpaceIgnore = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_SPACE);
	m_bStringSort = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_STRING);
	m_bNumSort = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_NUM);
	return 1;
}


void CDlgSortEx::SetEnableIgnores(BOOL bEnable)
{
	::EnableWindow(GetItemHwnd(IDC_CHECK_CASE), bEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_HIRAKATA), bEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_WIDTH), bEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_SPACE), bEnable);
}

void CDlgSortEx::SetEnableStirngCompareOption(BOOL bEnable)
{
	::EnableWindow(GetItemHwnd(IDC_CHECK_STRING), bEnable);
	BOOL bNum = bEnable;
	if( false == IsWin7_or_later() ){
		bNum = FALSE;
	}
	::EnableWindow(GetItemHwnd(IDC_CHECK_NUM), bNum);
}

BOOL CDlgSortEx::OnBnClicked(int wID)
{
	switch( wID ){
	case IDC_RADIO_STRING:
	case IDC_RADIO_API2:
		SetEnableIgnores(TRUE);
		SetEnableStirngCompareOption(FALSE);
		break;
	case IDC_RADIO_API1:
		SetEnableIgnores(TRUE);
		SetEnableStirngCompareOption(TRUE);
		break;
	case IDC_RADIO_API3:
	case IDC_RADIO_HEXVALUE:
	case IDC_RADIO_DECVALUE:
		SetEnableIgnores(FALSE);
		SetEnableStirngCompareOption(FALSE);
		break;
	case IDC_BUTTON_HELP:
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SORT_EX));
		break;
	case IDOK:
		GetData();
		CloseDialog(1);
		return TRUE;
	case IDCANCEL:
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}


LPVOID CDlgSortEx::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

