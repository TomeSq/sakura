/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildSearch.h"
#include "prop/CDlgConfig.h"
#include "extmodule/CBregexp.h"	// 2007.08/12 genta �o�[�W�����擾
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10500
	IDC_EDIT_REGEXPLIB,				HIDC_EDIT_REGEXPLIB,	//���K�\�����C�u�����I��	// 2007.09.02 genta
	IDC_LABEL_REGEXP,				HIDC_EDIT_REGEXPLIB,
	IDC_LABEL_REGEXP_VER,			HIDC_LABEL_REGEXPVER,	//���K�\�����C�u�����o�[�W����	// 2007.09.02 genta
	IDC_CHECK_bCaretTextForSearch,	HIDC_CHECK_bCaretTextForSearch,	//�J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���	// 2006.08.23 ryoji
	IDC_CHECK_INHERIT_KEY_OTHER_VIEW, HIDC_CHECK_INHERIT_KEY_OTHER_VIEW,	// ���E�O�����ő��̃r���[�̌��������������p��	// 2011.12.18 Moca
	IDC_CHECK_bGrepExitConfirm,		HIDC_CHECK_bGrepExitConfirm,	//GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,			HIDC_CHECK_GTJW_RETURN,			//�^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,			HIDC_CHECK_GTJW_LDBLCLK,		//�^�O�W�����v�i�_�u���N���b�N�j
	IDC_CHECK_GREPREALTIME,			HIDC_CHECK_GREPREALTIME,		//���A���^�C���ŕ\������	// 2006.08.08 ryoji
	IDC_COMBO_TAGJUMP,				HIDC_COMBO_TAGJUMP,				//�^�O�t�@�C���̌���
	IDC_COMBO_KEYWORD_TAGJUMP,		HIDC_COMBO_KEYWORD_TAGJUMP,		//�^�O�t�@�C���̌���
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildSearch::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_GREP, 0, SW_SHOW );
}


BOOL CDlgConfigChildSearch::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	return result;
}


BOOL CDlgConfigChildSearch::OnEnKillFocus( HWND hwndCtl, int wID )
{
	//	2007.08.12 genta ���K�\��DLL�̕ύX�ɉ�����Version���Ď擾����
	if( wID == IDC_EDIT_REGEXPLIB ){
		SetRegexpVersion();
	}
	return FALSE;
}

struct tagTagJumpMode{
	DWORD	m_nMethod;
	DWORD	m_nNameID;
};

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgConfigChildSearch::SetData()
{
	HWND hwndDlg = GetHwnd();

	/* 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bCaretTextForSearch, m_Common.m_sSearch.m_bCaretTextForSearch );

	CheckDlgButtonBool( hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW, m_Common.m_sSearch.m_bInheritKeyOtherView );

	/* Grep���[�h�ŕۑ��m�F���邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bGrepExitConfirm, m_Common.m_sSearch.m_bGrepExitConfirm );

	/* Grep���ʂ̃��A���^�C���\�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GREPREALTIME, m_Common.m_sSearch.m_bGrepRealTimeView );	// 2006.08.08 ryoji ID�C��


	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_Common.m_sSearch.m_bGTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_Common.m_sSearch.m_bGTJW_LDBLCLK );

	//	2007.08.12 genta ���K�\��DLL
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEXPLIB ), _countof(m_Common.m_sSearch.m_szRegexpLib ) - 1 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEXPLIB, m_Common.m_sSearch.m_szRegexpLib);
	SetRegexpVersion();

	struct tagTagJumpMode TagJumpMode1Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		//{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for(int i = 0; i < _countof(TagJumpMode1Arr); ++i){
		Combo_InsertString(hwndCombo, i, LS(TagJumpMode1Arr[i].m_nNameID));
		Combo_SetItemData(hwndCombo, i, TagJumpMode1Arr[i].m_nMethod);
		if(TagJumpMode1Arr[i].m_nMethod == m_Common.m_sSearch.m_nTagJumpMode ){
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	struct tagTagJumpMode TagJumpMode2Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for(int i = 0; i < _countof(TagJumpMode2Arr); ++i){
		Combo_InsertString(hwndCombo, i, LS(TagJumpMode2Arr[i].m_nNameID));
		Combo_SetItemData(hwndCombo, i, TagJumpMode2Arr[i].m_nMethod);
		if(TagJumpMode2Arr[i].m_nMethod == m_Common.m_sSearch.m_nTagJumpModeKeyword ){
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgConfigChildSearch::GetData()
{
	HWND hwndDlg = GetHwnd();

	/* 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� */
	m_Common.m_sSearch.m_bCaretTextForSearch = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bCaretTextForSearch );

	m_Common.m_sSearch.m_bInheritKeyOtherView = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW );

	/* Grep���[�h�ŕۑ��m�F���邩 */
	m_Common.m_sSearch.m_bGrepExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bGrepExitConfirm );

	/* Grep���ʂ̃��A���^�C���\�� */
	m_Common.m_sSearch.m_bGrepRealTimeView = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GREPREALTIME );	// 2006.08.08 ryoji ID�C��

	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	m_Common.m_sSearch.m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	m_Common.m_sSearch.m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );

	//	2007.08.12 genta ���K�\��DLL
	::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEXPLIB, m_Common.m_sSearch.m_szRegexpLib, _countof( m_Common.m_sSearch.m_szRegexpLib ));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	m_Common.m_sSearch.m_nTagJumpMode = Combo_GetItemData(hwndCombo, nSelPos);
	
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	nSelPos = Combo_GetCurSel(hwndCombo);
	m_Common.m_sSearch.m_nTagJumpModeKeyword = Combo_GetItemData(hwndCombo, nSelPos);

	return TRUE;
}

void CDlgConfigChildSearch::SetRegexpVersion()
{
	HWND hwndDlg = GetHwnd();
	TCHAR regexp_dll[_MAX_PATH];
	
	::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEXPLIB, regexp_dll, _countof( regexp_dll ));
	CBregexp breg;
	if( DLL_SUCCESS != breg.InitDll( regexp_dll ) ){
		::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEXP_VER, LS(STR_PROPCOMGREP_DLL) );
		return;
	}
	::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEXP_VER, breg.GetVersionT() );
}


LPVOID CDlgConfigChildSearch::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
