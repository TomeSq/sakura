/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�L�[�o�C���h�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/


#include "StdAfx.h"
#include "prop/CDlgConfigChildKeybind.h"
#include "prop/CDlgConfig.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

#define STR_SHIFT_PLUS        _T("Shift+")  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         _T("Ctrl+")  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          _T("Alt+")  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYBIND,		//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYBIND,		//�G�N�X�|�[�g
	IDC_BUTTON_ASSIGN,				HIDC_BUTTON_ASSIGN,				//�L�[���蓖��
	IDC_BUTTON_RELEASE,				HIDC_BUTTON_RELEASE,			//�L�[����
	IDC_CHECK_SHIFT,				HIDC_CHECK_SHIFT,				//Shift�L�[
	IDC_CHECK_CTRL,					HIDC_CHECK_CTRL,				//Ctrl�L�[
	IDC_CHECK_ALT,					HIDC_CHECK_ALT,					//Alt�L�[
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_KEYBIND,	//�@�\�̎��
	IDC_EDIT_KEYSFUNC,				HIDC_EDIT_KEYSFUNC,				//�L�[�Ɋ��蓖�Ă��Ă���@�\
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_KEYBIND,			//�@�\�ꗗ
	IDC_LIST_KEY,					HIDC_LIST_KEY,					//�L�[�ꗗ
	IDC_LIST_ASSIGNEDKEYS,			HIDC_LIST_ASSIGNEDKEYS,			//�@�\�Ɋ��蓖�Ă��Ă���L�[
	IDC_BUTTON_INITIALIZE,			HIDC_BUTTON_INITIALIZE_KEYBIND,	//������Ԃɖ߂�
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_KEYKIND,				(DWORD)-1,
	IDC_LABEL_FUNCtoKEY,			(DWORD)-1,
	IDC_LABEL_KEYtoFUNC,			(DWORD)-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

/* From Here Oct. 13, 2000 Studio C��Mr.�R�[�q�[���ɋ�����������ł������܂������Ă܂��� */
// �E�B���h�E�v���V�[�W���̒��ŁE�E�E
LRESULT CALLBACK CPropComKeybindWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	// WM_CTLCOLORSTATIC ���b�Z�[�W�ɑ΂���
	case WM_CTLCOLORSTATIC:
	// ���F�̃u���V�n���h����Ԃ�
		return (LRESULT)GetStockObject(WHITE_BRUSH);
//	default:
//		break;
	}
	return 0;
}
/* To Here Oct. 13, 2000 */


HWND CDlgConfigChildKeybind::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_KEYBIND, 0, SW_SHOW );
}


BOOL CDlgConfigChildKeybind::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	/* �R���g���[���̃n���h�����擾 */
	m_hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	m_hwndAssignedkeyList = ::GetDlgItem( hwndDlg, IDC_LIST_ASSIGNEDKEYS );
	m_hwndCheckShift = ::GetDlgItem( hwndDlg, IDC_CHECK_SHIFT );
	m_hwndCheckCtrl = ::GetDlgItem( hwndDlg, IDC_CHECK_CTRL );
	m_hwndCheckAlt = ::GetDlgItem( hwndDlg, IDC_CHECK_ALT );
	m_hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	m_hwndEDIT_KEYSFUNC = ::GetDlgItem( hwndDlg, IDC_EDIT_KEYSFUNC );

	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	/* �L�[�I�����̏��� */
//	From Here Oct. 14, 2000 JEPRO �킩��ɂ����̂őI�����Ȃ��悤�ɕύX	//Oct. 17, 2000 JEPRO �����I
//	/* �L�[���X�g�̐擪�̍��ڂ�I���i���X�g�{�b�N�X�j*/
	List_SetCurSel( m_hwndKeyList, 0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)m_hwndKeyList );	//Oct. 14, 2000 JEPRO �����͂ǂ����ł������H(�킩���)
//	To Here Oct. 14, 2000
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)m_hwndCombo );

	::SetTimer( hwndDlg, 1, 300, NULL );	// 2007.11.02 ryoji
	return result;
}


BOOL CDlgConfigChildKeybind::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	EFunctionCode	nFuncCode;
	switch( wID ){
	case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
		/* Keybind:�L�[���蓖�Đݒ���C���|�[�g���� */
		Import();
		return TRUE;
	case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
		/* Keybind:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
		Export();
		return TRUE;
	case IDC_BUTTON_ASSIGN:	/* ���t */
		nIndex = List_GetCurSel( m_hwndKeyList );
		nIndex2 = Combo_GetCurSel( m_hwndCombo );
		nIndex3 = List_GetCurSel( m_hwndFuncList );
		if( nIndex == LB_ERR || nIndex2 == CB_ERR || nIndex3 == LB_ERR ){
			return TRUE;
		}
		nFuncCode = m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
		i = 0;
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
			i |= _SHIFT;
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
			i |= _CTRL;
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
			i |= _ALT;
		}
		m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)m_hwndKeyList );
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)m_hwndFuncList );
		return TRUE;
	case IDC_BUTTON_RELEASE:	/* ���� */
		nIndex = List_GetCurSel( m_hwndKeyList );
		if( nIndex == LB_ERR ){
			return TRUE;
		}
		nFuncCode = F_DEFAULT;
		i = 0;
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
			i |= _SHIFT;
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
			i |= _CTRL;
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
			i |= _ALT;
		}
		m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)m_hwndKeyList );
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)m_hwndFuncList );
		return TRUE;
	// 2014.10.03 ������
	case IDC_BUTTON_INITIALIZE:
		if( IDYES == ConfirmMessage( hwndDlg, LS(STR_PROPCOMKEYBIND_INIT) ) ){
			CShareData::InitKeyAssign( &m_Common );
			ChangeKeyList();
			::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)m_hwndFuncList );
		}
		return TRUE;
	}
	HWND hwndCtl = GetItemHwnd( wID );
	if( m_hwndCheckShift == hwndCtl
	 || m_hwndCheckCtrl == hwndCtl
	 || m_hwndCheckAlt == hwndCtl
		){
		ChangeKeyList();
		return TRUE;
	}
	return FALSE;
}


BOOL CDlgConfigChildKeybind::OnLbnSelChange( HWND hwndCtl, int wID )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	EFunctionCode	nFuncCode;

	switch( wID ){
	case IDC_LIST_KEY:
		{
			nIndex = List_GetCurSel( m_hwndKeyList );
			i = 0;
			if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
				i |= _SHIFT;
			}
			if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
				i |= _CTRL;
			}
			if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
				i |= _ALT;
			}
			nFuncCode = m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
			// Oct. 2, 2001 genta
			// 2007.11.02 ryoji F_DISABLE�Ȃ疢���t
			WCHAR szLabel[256];
			if( nFuncCode == F_DISABLE ){
				auto_strncpy( szLabel, LSW(STR_PROPCOMKEYBIND_UNASSIGN), _countof(szLabel) - 1 );
			}else{
				m_pDlgConfigArg->m_cLookup.Funccode2Name( nFuncCode, szLabel, 255 );
			}
			Wnd_SetText( m_hwndEDIT_KEYSFUNC, szLabel );
			return TRUE;
		}
	case IDC_LIST_FUNC:
		{
			nIndex = List_GetCurSel( m_hwndKeyList );
			nIndex2 = Combo_GetCurSel( m_hwndCombo );
			nIndex3 = List_GetCurSel( m_hwndFuncList );
			nFuncCode = m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
			/* �@�\�ɑΉ�����L�[���̎擾(����) */
			CNativeT**	ppcAssignedKeyList;
			int nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* �@�\�ɑΉ�����L�[���̎擾(����) */
				G_AppInstance(), m_Common.m_sKeyBind.m_nKeyNameArrNum, m_Common.m_sKeyBind.m_pKeyNameArr,
				&ppcAssignedKeyList, nFuncCode,
				FALSE	// 2007.02.22 ryoji �f�t�H���g�@�\�͎擾���Ȃ�
			);	
			/* ���蓖�ăL�[���X�g���N���A���Ēl�̐ݒ� */
			List_ResetContent( m_hwndAssignedkeyList );
			if( 0 < nAssignedKeyNum){
				int j;
				for( j = 0; j < nAssignedKeyNum; ++j ){
					/* �f�o�b�O���j�^�ɏo�� */
					const TCHAR* cpszString = ppcAssignedKeyList[j]->GetStringPtr();
					::List_AddString( m_hwndAssignedkeyList, cpszString );
					delete ppcAssignedKeyList[j];
				}
				delete [] ppcAssignedKeyList;
			}
			return TRUE;
		}
	case IDC_LIST_ASSIGNEDKEYS:
		{
			TCHAR	buff[1024], *p;
			int	ret;

			nIndex = List_GetCurSel( m_hwndAssignedkeyList );
			auto_memset(buff, 0, _countof(buff));
			ret = List_GetText( m_hwndAssignedkeyList, nIndex, buff);
			if( ret != LB_ERR )
			{
				i = 0;
				p = buff;
				//SHIFT
				if( auto_memcmp(p, STR_SHIFT_PLUS, _tcslen(STR_SHIFT_PLUS)) == 0 ){
					p += _tcslen(STR_SHIFT_PLUS);
					i |= _SHIFT;
				}
				//CTRL
				if( auto_memcmp(p, STR_CTRL_PLUS, _tcslen(STR_CTRL_PLUS)) == 0 ){
					p += _tcslen(STR_CTRL_PLUS);
					i |= _CTRL;
				}
				//ALT
				if( auto_memcmp(p, STR_ALT_PLUS, _tcslen(STR_ALT_PLUS)) == 0 ){
					p += _tcslen(STR_ALT_PLUS);
					i |= _ALT;
				}
				int j;
				for(j = 0; j < m_Common.m_sKeyBind.m_nKeyNameArrNum; j++)
				{
					if( _tcscmp(m_Common.m_sKeyBind.m_pKeyNameArr[j].m_szKeyName, p) == 0 )
					{
						List_SetCurSel( m_hwndKeyList, j);
						if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //�`�F�b�N
						else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //�`�F�b�N���͂���
						if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //�`�F�b�N
						else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //�`�F�b�N���͂���
						if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //�`�F�b�N
						else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //�`�F�b�N���͂���
						::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)m_hwndKeyList );

						// �L�[�ꗗ�̕�������ύX
						ChangeKeyList();
						break;
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDlgConfigChildKeybind::OnCbnSelChange( HWND hwndCtl, int wID )
{
	// �R���{�{�b�N�X�̑I��ύX
	switch( wID ){
	case IDC_COMBO_FUNCKIND:
		{
			int nIndex2 = Combo_GetCurSel( m_hwndCombo );
			/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
			m_pDlgConfigArg->m_cLookup.SetListItem( m_hwndFuncList, nIndex2 );	//	Oct. 2, 2001 genta
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CDlgConfigChildKeybind::OnTimer( WPARAM wParam )
{
	HWND hwndDlg = GetHwnd();

	// �{�^���̗L���^������؂�ւ���	// 2007.11.02 ryoji
	int nIndex = List_GetCurSel( m_hwndKeyList );
	int nIndex2 = Combo_GetCurSel( m_hwndCombo );
	int nIndex3 = List_GetCurSel( m_hwndFuncList );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ASSIGN ), !( LB_ERR == nIndex || nIndex2 == CB_ERR || nIndex3 == LB_ERR ) );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RELEASE ), !( LB_ERR == nIndex ) );
	return TRUE;
}


BOOL CDlgConfigChildKeybind::OnDestroy()
{
	::KillTimer( GetHwnd(), 1 );
	return CDlgConfigChild::OnDestroy();
}


/* �_�C�A���O�f�[�^�̐ݒ� Keybind */
void CDlgConfigChildKeybind::SetData()
{
	HWND hwndDlg = GetHwnd();

	int			i;

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	m_pDlgConfigArg->m_cLookup.SetCategory2Combo( m_hwndCombo );	//	Oct. 2, 2001 genta

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	Combo_SetCurSel( m_hwndCombo, 0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	m_hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		::List_AddString( m_hwndKeyList, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
	}

	return;
}



/* �_�C�A���O�f�[�^�̎擾 Keybind */
int CDlgConfigChildKeybind::GetData()
{
	return TRUE;
}
	
/*! Keybind: �L�[���X�g���`�F�b�N�{�b�N�X�̏�Ԃɍ��킹�čX�V���� */
void CDlgConfigChildKeybind::ChangeKeyList()
{
	HWND hwndDlg = GetHwnd();

	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	nIndex = List_GetCurSel( m_hwndKeyList );
	nIndexTop = List_GetTopIndex( m_hwndKeyList );
	szKeyState[0] = L'\0';
	i = 0;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
		i |= _SHIFT;
		wcscat( szKeyState, L"Shift+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
		i |= _CTRL;
		wcscat( szKeyState, L"Ctrl+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
		i |= _ALT;
		wcscat( szKeyState, L"Alt+" );
	}
	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	List_ResetContent( m_hwndKeyList );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		auto_sprintf( pszLabel, _T("%ls%ts"), szKeyState, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
		::List_AddString( m_hwndKeyList, pszLabel );
	}
	List_SetCurSel( m_hwndKeyList, nIndex );
	List_SetTopIndex( m_hwndKeyList, nIndexTop );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)m_hwndKeyList );
}

/* Keybind:�L�[���蓖�Đݒ���C���|�[�g���� */
void CDlgConfigChildKeybind::Import()
{
	HWND hwndDlg = GetHwnd();

	CImpExpKeybind	cImpExpKeybind( m_Common );

	// �C���|�[�g
	if (!cImpExpKeybind.ImportUI( G_AppInstance(), hwndDlg )) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}

	// �_�C�A���O�f�[�^�̐ݒ� Keybind
	// 2012.11.18 aroka �L�[�ꗗ�̍X�V�͑S�A�C�e�����X�V����B
	ChangeKeyList();
	//@@@ 2001.11.07 modify start MIK: �@�\�Ɋ��蓖�Ă��Ă���L�[���X�V����B// 2012.11.18 aroka �R�����g�C��
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
}


/* Keybind:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
void CDlgConfigChildKeybind::Export()
{
	HWND hwndDlg = GetHwnd();

	CImpExpKeybind	cImpExpKeybind( m_Common );

	// �G�N�X�|�[�g
	if (!cImpExpKeybind.ExportUI( G_AppInstance(), hwndDlg )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}


LPVOID CDlgConfigChildKeybind::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
