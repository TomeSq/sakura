/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�J�X�^�����j���[�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyrithg (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildCustMenu.h"
#include "prop/CDlgConfig.h"
#include "dlg/CDlgInput1.h"
#include "env/CShareData.h"
#include "util/shell.h"
#include "util/window.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static	int 	nSpecialFuncsNum;		// ���ʋ@�\�̃R���{�{�b�N�X���ł̔ԍ�

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE,				//���j���[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT,				//���j���[�֋@�\�}��
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD,				//���j���[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_UP,					//���j���[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN,				//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,				//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,				//�G�N�X�|�[�g
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,			//�@�\�̎��
	IDC_COMBO_MENU,					HIDC_COMBO_MENU,				//���j���[�̎��
	IDC_EDIT_MENUNAME,				HIDC_EDIT_MENUNAME,				//���j���[��		// 2009.02.20 ryoji
	IDC_BUTTON_MENUNAME,			HIDC_BUTTON_MENUNAME,			//���j���[���ݒ�	// 2009.02.20 ryoji
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,					//�@�\�ꗗ
	IDC_LIST_RES,					HIDC_LIST_RES,					//���j���[�ꗗ
	IDC_CHECK_SUBMENU,				HIDC_CHECK_SUBMENU,				//�T�u���j���[�Ƃ��ĕ\��
	IDC_BUTTON_INITIALIZE,			HIDC_BUTTON_INITIALIZE_CUSTMENU,	//������Ԃɖ߂�
//	IDC_LABEL_MENUFUNCKIND,			-1,
//	IDC_LABEL_MENUCHOICE,			-1,
//	IDC_LABEL_MENUFUNC,				-1,
//	IDC_LABEL_MENU,					-1,
//	IDC_LABEL_MENUKEYCHANGE,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildCustMenu::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_CUSTMENU, 0, SW_SHOW );
}


BOOL CDlgConfigChildCustMenu::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	/* �L�[�I�����̏��� */
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND ) );

	HWND hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	HWND hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
	int nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
	int nIdx2 = List_GetCurSel( hwndLIST_RES );
	int nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
	int nIdx4 = List_GetCurSel( hwndLIST_FUNC );
	if( nIdx1 != CB_ERR ){
		::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_MENU );
		if( nIdx2 != LB_ERR ){
			List_SetCurSel( hwndLIST_RES, nIdx2 );
		}
	}
	if( nIdx3 != CB_ERR ){
		::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );
		if( nIdx4 != LB_ERR ){
			List_SetCurSel( hwndLIST_FUNC, nIdx4 );
		}
	}
	::SetTimer( hwndDlg, 1, 300, NULL );

	return result;
}


BOOL CDlgConfigChildCustMenu::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();
	int nIdx1;
	int nIdx2;
	int nIdx3;
	int nIdx4;
	int nNum2;
	int i;
	EFunctionCode	eFuncCode = F_0;
	HWND hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	HWND hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
	WCHAR		szLabel[300];

	switch( wID ){
	case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
		/* �J�X�^�����j���[�ݒ���C���|�[�g���� */
		Import();
		return TRUE;

	case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
		/* �J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
		Export();
		return TRUE;

	case IDC_BUTTON_MENUNAME:
		{
			WCHAR buf[ MAX_CUSTOM_MENU_NAME_LEN + 1 ];
			HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
			//	���j���[������̐ݒ�
			int nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
			if( CB_ERR == nIdx1 ){
				break;
			}
			::DlgItem_GetText( hwndDlg, IDC_EDIT_MENUNAME,
				m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1], MAX_CUSTOM_MENU_NAME_LEN );
			//	Combo Box���ύX �폜���ēo�^
			Combo_DeleteString( hwndCOMBO_MENU, nIdx1 );
			Combo_InsertString( hwndCOMBO_MENU, nIdx1,
				m_pDlgConfigArg->m_cLookup.Custmenu2Name( nIdx1, buf, _countof(buf) ) );
			// �폜����ƑI�������������̂ŁC���ɖ߂�
			Combo_SetCurSel( hwndCOMBO_MENU, nIdx1 );
		}
		return TRUE;
		// 2014.10.03 ������
	case IDC_BUTTON_INITIALIZE:
		if( IDYES == ConfirmMessage( hwndDlg, LS(STR_PROPCOMCUSTMENU_INIT) ) ){
			CShareData::InitPopupMenu( &m_Common );
			// ��ʍX�V
			HWND	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
			::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCtrl );
		}
		return TRUE;
	case IDC_BUTTON_INSERTSEPARATOR:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}

		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			break;
		}

		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			nIdx2 = 0;
		}
		nIdx2 = List_InsertString( hwndLIST_RES, nIdx2, LSW(STR_PROPCOMCUSTMENU_SEP) );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
		if( nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE ){
			break;
		}
		List_SetCurSel( hwndLIST_RES, nIdx2 );

		for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i--){
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
		}
		m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = F_0;
		m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
		m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText( hwndEDIT_KEY, L"" );
		break;

	case IDC_BUTTON_DELETE:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}

		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			break;
		}

		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			break;
		}
		nNum2 = List_DeleteString( hwndLIST_RES, nIdx2 );
		if( nNum2 == LB_ERR ){
			break;
		}

		for( i = nIdx2; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i + 1];
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i + 1];
		}
		m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]--;

		if( nNum2 > 0 ){
			if( nNum2 <= nIdx2 ){
				nIdx2 = nNum2 - 1;
			}
			nIdx2 = List_SetCurSel( hwndLIST_RES, nIdx2 );

		}else{
		}
		break;

	case IDC_BUTTON_INSERT:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}

		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			break;
		}

		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			nIdx2 = 0;
		}
		nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
		if( CB_ERR == nIdx3 ){
			break;
		}
		nIdx4 = List_GetCurSel( hwndLIST_FUNC );
		if( LB_ERR == nIdx4 ){
			break;
		}
		List_GetText( hwndLIST_FUNC, nIdx4, szLabel );

		for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i-- ){
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
		}
		//	Oct. 3, 2001 genta
		if (nIdx3 == nSpecialFuncsNum) {
			// ����@�\
			eFuncCode = nsFuncCode::pnFuncList_Special[nIdx4];
		}else{
			eFuncCode = m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
		}
		m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = eFuncCode;
		m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
		m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

		nIdx2 = List_InsertString( hwndLIST_RES, nIdx2, szLabel );
		if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
			break;
		}
		List_SetCurSel( hwndLIST_RES, nIdx2 );
		break;

	case IDC_BUTTON_ADD:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}

		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			break;
		}

		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			nIdx2 = 0;
		}
		nNum2 = List_GetCount( hwndLIST_RES );
		if( LB_ERR == nNum2 ){
			nIdx2 = 0;
		}
		nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
		if( CB_ERR == nIdx3 ){
			break;
		}
		nIdx4 = List_GetCurSel( hwndLIST_FUNC );
		if( LB_ERR == nIdx4 ){
			break;
		}

		List_GetText( hwndLIST_FUNC, nIdx4, szLabel );
		eFuncCode = F_DISABLE;
		if (nIdx3 == nSpecialFuncsNum) {
			// ����@�\
			if( 0 <= nIdx4 && nIdx4 < nsFuncCode::nFuncList_Special_Num ){
				eFuncCode = nsFuncCode::pnFuncList_Special[nIdx4];
			}
		}else{
			eFuncCode = m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
		}
		//	Oct. 3, 2001 genta
		if( eFuncCode == F_DISABLE )
			break;
		m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nNum2] = eFuncCode;
		m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
		m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

		nIdx2 = List_AddString( hwndLIST_RES, szLabel );
		if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
			break;
		}
		List_SetCurSel( hwndLIST_RES, nIdx2 );

		break;

	case IDC_BUTTON_UP:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}
		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			break;
		}
		if( 0 == nIdx2 ){
			break;
		}

		{
			EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
			KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
		}

		List_GetText( hwndLIST_RES, nIdx2, szLabel );
		List_DeleteString( hwndLIST_RES, nIdx2 );
		List_InsertString( hwndLIST_RES, nIdx2 - 1, szLabel );
		List_SetCurSel( hwndLIST_RES, nIdx2 - 1 );
		break;

	case IDC_BUTTON_DOWN:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}
		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			break;
		}
		nNum2 = List_GetCount( hwndLIST_RES );
		if( LB_ERR == nNum2 ){
			break;
		}
		if( nNum2 - 1 <= nIdx2 ){
			break;
		}

		{
			EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
			KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
			m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
		}
		List_GetText( hwndLIST_RES, nIdx2, szLabel );
		List_DeleteString( hwndLIST_RES, nIdx2 );
		List_InsertString( hwndLIST_RES, nIdx2 + 1, szLabel );
		List_SetCurSel( hwndLIST_RES, nIdx2 + 1 );
		break;
	case IDC_CHECK_SUBMENU:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			break;
		}
		m_Common.m_sCustomMenu.m_bCustMenuPopupArr[nIdx1] = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_SUBMENU );
		break;
	}
	return TRUE;
}


BOOL CDlgConfigChildCustMenu::OnCbnSelChange( HWND hwndCtl, int wID )
{
	HWND hwndDlg = GetHwnd();
	switch( wID ){
	case IDC_COMBO_MENU:
		{
			HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
			int nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
			if( CB_ERR == nIdx1 ){
				break;
			}
			SetDataMenuList( nIdx1 );
		}
		break;

	case IDC_COMBO_FUNCKIND:
		{
			HWND hwndCOMBO_FUNCKIND = hwndCtl;
			HWND hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
			int nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );

			if (nIdx3 == nSpecialFuncsNum) {
				// �@�\�ꗗ�ɓ���@�\���Z�b�g
				List_ResetContent( hwndLIST_FUNC );
				int i;
				for (i = 0; i < nsFuncCode::nFuncList_Special_Num; i++) {
					List_AddString( hwndLIST_FUNC, LS( nsFuncCode::pnFuncList_Special[i] ) );
				}
			}
			else {
				// Oct. 3, 2001 genta
				// ��p���[�`���ɒu������
				m_pDlgConfigArg->m_cLookup.SetListItem( hwndLIST_FUNC, nIdx3 );
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL CDlgConfigChildCustMenu::OnLbnDblclk( int wID )
{
	HWND hwndDlg = GetHwnd();
	int nIdx1;
	int nIdx2;

	switch( wID ){
	case IDC_LIST_RES:
		HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
		HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		if( CB_ERR == nIdx1 ){
			return FALSE;
		}
		nIdx2 = List_GetCurSel( hwndLIST_RES );
		if( LB_ERR == nIdx2 ){
			return FALSE;
		}

		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] ){
			return FALSE;
		}

		TCHAR		szKey[2];
		auto_sprintf( szKey, _T("%hc"), m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
		{
			CDlgInput1	cDlgInput1;
			BOOL bDlgInputResult = cDlgInput1.DoModal(
				G_AppInstance(),
				hwndDlg,
				LS(STR_PROPCOMCUSTMENU_AC1),
				LS(STR_PROPCOMCUSTMENU_AC2),
				1,
				szKey
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		WCHAR		szLabel[300];
		WCHAR		szLabel2[300+4];
		//	Oct. 3, 2001 genta
		m_pDlgConfigArg->m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );

		{
			KEYCODE keycode[3]={0}; _tctomb(szKey, keycode);
			m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = keycode[0];
		}
//@@@ 2002.01.08 YAZAKI �J�X�^�����j���[�ŃA�N�Z�X�L�[�����������A���J�b�R ( �����j���[���ڂɈ��c��o�O�C��
		if (m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]){
			auto_sprintf( szLabel2, LTEXT("%ts(%hc)"),
				szLabel,
				m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]
			);
		}
		else {
			auto_sprintf( szLabel2, LTEXT("%ls"), szLabel );
		}

		List_InsertString( hwndLIST_RES, nIdx2, szLabel2 );
		List_DeleteString( hwndLIST_RES, nIdx2 + 1 );
	}
	return FALSE;
}


BOOL CDlgConfigChildCustMenu::OnLbnSelChange( HWND hwndCtl, int wID )
{
	HWND hwndDlg = GetHwnd();
	switch( wID ){
	case IDC_LIST_RES:
		{
			HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
			HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
			int nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
			if( CB_ERR == nIdx1 ){
				break;
			}

			if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
				break;
			}

			int nIdx2 = List_GetCurSel( hwndLIST_RES );
			if( LB_ERR == nIdx2 ){
				break;
			}

			/* �L�[ */
			if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ||
				' '  == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ){
			}else{
			}
		}
		break;
	}
	return FALSE;
}


BOOL CDlgConfigChildCustMenu::OnTimer( WPARAM wParam )
{
	HWND hwndDlg = GetHwnd();

	HWND hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	HWND hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	HWND hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
	int nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
	int nIdx2 = List_GetCurSel( hwndLIST_RES );
	int nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
	int nIdx4 = List_GetCurSel( hwndLIST_FUNC );
	int i = List_GetCount( hwndLIST_RES );
	if( LB_ERR == nIdx2	){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
		if( nIdx2 <= 0 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
		}
		if( nIdx2 + 1 >= i ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
		}
	}
	if( LB_ERR == nIdx2 || LB_ERR == nIdx4 ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
	}
	if( LB_ERR == nIdx4 ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
	}
	if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERTSEPARATOR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
	}
	if( CB_ERR != nIdx3 && LB_ERR != nIdx4 &&
	 	m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0 &&
		!(nIdx3 == nSpecialFuncsNum && 0 <= nIdx4 && nIdx4 < nsFuncCode::nFuncList_Special_Num)
	){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
	}
	return TRUE;
}

BOOL CDlgConfigChildCustMenu::OnDestroy(){
	::KillTimer( GetHwnd(), 1 );
	return CDlgConfigChild::OnDestroy();
}


/* �_�C�A���O�f�[�^�̐ݒ� Custom menu */
void CDlgConfigChildCustMenu::SetData()
{
	HWND hwndDlg = GetHwnd();
	HWND		hwndCOMBO_MENU;
	HWND		hwndCombo;
	int			i;
	WCHAR		buf[ MAX_CUSTOM_MENU_NAME_LEN + 1 ];

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_pDlgConfigArg->m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 3, 2001 genta
	// ���ʋ@�\�ǉ�
	nSpecialFuncsNum = Combo_AddString( hwndCombo, LS( STR_SPECIAL_FUNC ) );

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* ���j���[�ꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		Combo_AddString( hwndCOMBO_MENU, m_pDlgConfigArg->m_cLookup.Custmenu2Name( i, buf, _countof( buf ) ) );
	}
	/* ���j���[�ꗗ�̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	Combo_SetCurSel( hwndCOMBO_MENU, 0 );
	SetDataMenuList( 0 );

//	/* �J�X�^�����j���[�̐擪�̍��ڂ�I���i���X�g�{�b�N�X�j*/	//Oct. 8, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
	HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
	List_SetCurSel( hwndLIST_RES, 0 );
}

void CDlgConfigChildCustMenu::SetDataMenuList(int nIdx)
{
	HWND hwndDlg = GetHwnd();
	int			i;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300+4];

	/* ���j���[���ڈꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	HWND hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//	hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );
	List_ResetContent( hwndLIST_RES );
	for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx]; ++i ){
		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i] ){
			auto_strncpy( szLabel, LSW(STR_PROPCOMCUSTMENU_SEP), _countof(szLabel) - 1 );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
			
			szLabel[_countof(szLabel) - 1] = L'\0';
		}else{
			EFunctionCode code = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i];
			//	Oct. 3, 2001 genta
			m_pDlgConfigArg->m_cLookup.Funccode2Name( code, szLabel, 256 );
		}
		/* �L�[ */
		if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i] ){
			auto_strcpy( szLabel2, szLabel );
		}else{
			auto_sprintf( szLabel2, LTEXT("%ls(%hc)"),
				szLabel,
				m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i]
			);
		}
		::List_AddString( hwndLIST_RES, szLabel2 );
	}
	
	//	Oct. 15, 2001 genta ���j���[����ݒ�
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx] );

	CheckDlgButtonBool( hwndDlg, IDC_CHECK_SUBMENU, m_Common.m_sCustomMenu.m_bCustMenuPopupArr[nIdx] );
	return;
}



/* �_�C�A���O�f�[�^�̎擾 Custom menu */
int CDlgConfigChildCustMenu::GetData()
{
	return TRUE;
}





/* �J�X�^�����j���[�ݒ���C���|�[�g���� */
void CDlgConfigChildCustMenu::Import()
{
	HWND hwndDlg = GetHwnd();
	CImpExpCustMenu	cImpExpCustMenu( m_Common );

	// �C���|�[�g
	if (!cImpExpCustMenu.ImportUI( G_AppInstance(), hwndDlg )) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}
	
	// ��ʍX�V
	HWND	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCtrl );
}

/* �J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
void CDlgConfigChildCustMenu::Export()
{
	HWND hwndDlg = GetHwnd();
	CImpExpCustMenu	cImpExpCustMenu( m_Common );

	// �G�N�X�|�[�g
	if (!cImpExpCustMenu.ExportUI( G_AppInstance(), hwndDlg )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}


LPVOID CDlgConfigChildCustMenu::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
