/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�S�ʁv�y�[�W

	@author Uchi
	@date 2010/5/9 CPropCommon.c��蕪��
*/
/*
	Copyright (C) 2010, Uchi
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildGeneral.h"
#include "prop/CDlgConfig.h"
#include "typeprop/CPropTypes.h" // TYPE_NAME_ID
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
TYPE_NAME_ID<int> SpecialScrollModeArr[] = {
	{ 0,						STR_SCROLL_WITH_NO_KEY },		//_T("�g�ݍ��킹�Ȃ�") },
	{ MOUSEFUNCTION_CENTER,		STR_SCROLL_WITH_MID_BTN },		//_T("�}�E�X���{�^��") },
	{ MOUSEFUNCTION_LEFTSIDE,	STR_SCROLL_WITH_SIDE_1_BTN },	//_T("�}�E�X�T�C�h�{�^��1") },
	{ MOUSEFUNCTION_RIGHTSIDE,	STR_SCROLL_WITH_SIDE_2_BTN },	//_T("�}�E�X�T�C�h�{�^��2") },
	{ VK_CONTROL,				STR_SCROLL_WITH_CTRL_KEY },	//_T("CONTROL�L�[") },
	{ VK_SHIFT,					STR_SCROLL_WITH_SHIFT_KEY },	//_T("SHIFT�L�[") },
};

static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		HIDC_BUTTON_CLEAR_MRU_FILE,			//�������N���A�i�t�@�C���j
	IDC_BUTTON_CLEAR_MRU_FOLDER,	HIDC_BUTTON_CLEAR_MRU_FOLDER,		//�������N���A�i�t�H���_�j
	IDC_CHECK_FREECARET,			HIDC_CHECK_FREECARET,				//�t���[�J�[�\��
//DEL	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,					//�����C���f���g �F�^�C�v�ʂֈړ�
//DEL	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,			//�S�p�󔒂��C���f���g �F�^�C�v�ʂֈړ�
	IDC_CHECK_USETRAYICON,			HIDC_CHECK_USETRAYICON,				//�^�X�N�g���C���g��
	IDC_CHECK_STAYTASKTRAY,			HIDC_CHECK_STAYTASKTRAY,			//�^�X�N�g���C�ɏ풓
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	HIDC_CHECK_REPEATEDSCROLLSMOOTH,	//�������炩�ɂ���
	IDC_CHECK_CLOSEALLCONFIRM,		HIDC_CHECK_CLOSEALLCONFIRM,			//[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����	// 2006.12.25 ryoji
	IDC_CHECK_EXITCONFIRM,			HIDC_CHECK_EXITCONFIRM,				//�I���̊m�F
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, HIDC_CHECK_STOPS_WORD, //�P��P�ʂňړ�����Ƃ��ɒP��̗��[�Ɏ~�܂�
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, HIDC_CHECK_STOPS_PARAGRAPH, // �i���P�ʂňړ�����Ƃ��ɒi���̗��[�Ɏ~�܂�
	IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, HIDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE,	// �}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ� 2007.10.08 genta
	IDC_HOTKEY_TRAYMENU,			HIDC_HOTKEY_TRAYMENU,				//���N���b�N���j���[�̃V���[�g�J�b�g�L�[
	IDC_EDIT_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,	//�X�N���[���s��
	IDC_EDIT_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,				//�t�@�C�������̍ő吔
	IDC_EDIT_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,			//�t�H���_�����̍ő吔
	IDC_RADIO_CARETTYPE0,			HIDC_RADIO_CARETTYPE0,				//�J�[�\���`��iWindows���j
	IDC_RADIO_CARETTYPE1,			HIDC_RADIO_CARETTYPE1,				//�J�[�\���`��iMS-DOS���j
	IDC_SPIN_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,
	IDC_SPIN_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,
	IDC_SPIN_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,
	IDC_CHECK_MEMDC,				HIDC_CHECK_MEMDC,					//��ʃL���b�V�����g��
	IDC_COMBO_WHEEL_PAGESCROLL,		HIDC_COMBO_WHEEL_PAGESCROLL,		// �g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������		// 2009.01.17 nasukoji
	IDC_COMBO_WHEEL_HSCROLL,		HIDC_COMBO_WHEEL_HSCROLL,			// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������			// 2009.01.17 nasukoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildGeneral::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_GENERAL, 0, SW_SHOW );
}


BOOL CDlgConfigChildGeneral::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	return result;
}


BOOL CDlgConfigChildGeneral::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();

	switch( wID ){
	case IDC_CHECK_USETRAYICON:	/* �^�X�N�g���C���g�� */
	// From Here 2001.12.03 hor
	//		���삵�ɂ������ĕ]���������̂Ń^�X�N�g���C�֌W��Enable�������߂܂���
	//@@@ YAZAKI 2001.12.31 IDC_CHECKSTAYTASKTRAY�̃A�N�e�B�u�A��A�N�e�B�u�̂ݐ���B
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
		}
	// To Here 2001.12.03 hor
		return TRUE;

	case IDC_CHECK_STAYTASKTRAY:	/* �^�X�N�g���C�ɏ풓 */
		return TRUE;

	case IDC_BUTTON_CLEAR_MRU_FILE:
		/* �t�@�C���̗������N���A */
		if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
			LS(STR_PROPCOMGEN_FILE1) ) ){
			return TRUE;
		}
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
//				m_pShareData->m_sHistory.m_nMRUArrNum = 0;
		{
			CMRUFile cMRU;
			cMRU.ClearAll();
		}
		InfoMessage( hwndDlg, LS(STR_PROPCOMGEN_FILE2) );
		return TRUE;
	case IDC_BUTTON_CLEAR_MRU_FOLDER:
		/* �t�H���_�̗������N���A */
		if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
			LS(STR_PROPCOMGEN_DIR1) ) ){
			return TRUE;
		}
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
//				m_pShareData->m_sHistory.m_nOPENFOLDERArrNum = 0;
		{
			CMRUFolder cMRUFolder;	//	MRU���X�g�̏������B���x�������Ɩ�肠��H
			cMRUFolder.ClearAll();
		}
		InfoMessage( hwndDlg, LS(STR_PROPCOMGEN_DIR2) );
		return TRUE;
	}
	return FALSE;
}



BOOL CDlgConfigChildGeneral::OnCbnSelEndOk( HWND hwndCtl, int wID )
{
	// 2009.01.12 nasukoji	�R���{�{�b�N�X�̃��X�g�̍��ڂ��I�����ꂽ
	HWND hwndDlg = GetHwnd();
	HWND	hwndCombo;
	int		nSelPos;

	switch( wID ){
	// �g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
	case IDC_COMBO_WHEEL_PAGESCROLL:
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL );
		nSelPos = Combo_GetCurSel( hwndCombo );
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_HSCROLL );
		if( nSelPos && nSelPos == Combo_GetCurSel( hwndCombo ) ){
			Combo_SetCurSel( hwndCombo, 0 );
		}
		return TRUE;
	// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
	case IDC_COMBO_WHEEL_HSCROLL:
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_HSCROLL );
		nSelPos = Combo_GetCurSel( hwndCombo );
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL );
		if( nSelPos && nSelPos == Combo_GetCurSel( hwndCombo ) ){
			Combo_SetCurSel( hwndCombo, 0 );
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CDlgConfigChildGeneral::OnNotify( WPARAM wParam, LPARAM lParam )
{
	HWND hwndDlg = GetHwnd();
	int			idCtrl = (int)wParam;
	NMHDR*		pNMHDR = (NMHDR*)lParam;
	NM_UPDOWN*	pMNUD  = (NM_UPDOWN*)lParam;
	int nVal;

	switch( idCtrl ){
	case IDC_SPIN_REPEATEDSCROLLLINENUM:
		/* �L�[���s�[�g���̃X�N���[���s�� */
//			MYTRACE( _T("IDC_SPIN_REPEATEDSCROLLLINENUM\n") );
		nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nVal;
		}else
		if( pMNUD->iDelta > 0 ){
			--nVal;
		}
		if( nVal < 1 ){
			nVal = 1;
		}
		if( nVal > 10 ){
			nVal = 10;
		}
		::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE );
		return TRUE;
	case IDC_SPIN_MAX_MRU_FILE:
		/* �t�@�C���̗���MAX */
//			MYTRACE( _T("IDC_SPIN_MAX_MRU_FILE\n") );
		nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nVal;
		}else
		if( pMNUD->iDelta > 0 ){
			--nVal;
		}
		if( nVal < 0 ){
			nVal = 0;
		}
		if( nVal > MAX_MRU ){
			nVal = MAX_MRU;
		}
		::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, nVal, FALSE );
		return TRUE;
	case IDC_SPIN_MAX_MRU_FOLDER:
		/* �t�H���_�̗���MAX */
//			MYTRACE( _T("IDC_SPIN_MAX_MRU_FOLDER\n") );
		nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nVal;
		}else
		if( pMNUD->iDelta > 0 ){
			--nVal;
		}
		if( nVal < 0 ){
			nVal = 0;
		}
		if( nVal > MAX_OPENFOLDER ){
			nVal = MAX_OPENFOLDER;
		}
		::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, nVal, FALSE );
		return TRUE;
	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� General */
void CDlgConfigChildGeneral::SetData()
{
	HWND hwndDlg = GetHwnd();

	/* �J�[�\���̃^�C�v 0=win 1=dos  */
	if( 0 == m_Common.m_sGeneral.GetCaretType() ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, TRUE );
	}


	/* �t���[�J�[�\�����[�h */
	::CheckDlgButton( hwndDlg, IDC_CHECK_FREECARET, m_Common.m_sGeneral.m_bIsFreeCursorMode ? 1 : 0 );

	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord );

	/* �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph );

	//	2007.10.08 genta �}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ� (2007.10.02 by nasukoji)
	::CheckDlgButton( hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, m_Common.m_sGeneral.m_bNoCaretMoveByActivation );

	/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_CLOSEALLCONFIRM, m_Common.m_sGeneral.m_bCloseAllConfirm );

	/* �I�����̊m�F������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_EXITCONFIRM, m_Common.m_sGeneral.m_bExitConfirm );

	/* �L�[���s�[�g���̃X�N���[���s�� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, (Int)m_Common.m_sGeneral.m_nRepeatedScrollLineNum, FALSE );

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH, m_Common.m_sGeneral.m_nRepeatedScroll_Smooth );

	// 2009.01.17 nasukoji	�g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
	HWND	hwndCombo;
	int		nSelPos;
	int		i;

	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL );
	Combo_ResetContent( hwndCombo );
	nSelPos = 0;
	for( i = 0; i < _countof( SpecialScrollModeArr ); ++i ){
		Combo_InsertString( hwndCombo, i, LS( SpecialScrollModeArr[i].nNameId ) );
		if( SpecialScrollModeArr[i].nMethod == m_Common.m_sGeneral.m_nPageScrollByWheel ){	// �y�[�W�X�N���[���Ƃ���g�ݍ��킹����
			nSelPos = i;
		}
	}
	Combo_SetCurSel( hwndCombo, nSelPos );

	// 2009.01.12 nasukoji	�g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_HSCROLL );
	Combo_ResetContent( hwndCombo );
	nSelPos = 0;
	for( i = 0; i < _countof( SpecialScrollModeArr ); ++i ){
		Combo_InsertString( hwndCombo, i, LS( SpecialScrollModeArr[i].nNameId ) );
		if( SpecialScrollModeArr[i].nMethod == m_Common.m_sGeneral.m_nHorizontalScrollByWheel ){	// ���X�N���[���Ƃ���g�ݍ��킹����
			nSelPos = i;
		}
	}
	Combo_SetCurSel( hwndCombo, nSelPos );

	// 2007.09.09 Moca ��ʃL���b�V���ݒ�ǉ�
	// ��ʃL���b�V�����g��
	::CheckDlgButton( hwndDlg, IDC_CHECK_MEMDC, m_Common.m_sWindow.m_bUseCompatibleBMP );

	/* �t�@�C���̗���MAX */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, m_Common.m_sGeneral.m_nMRUArrNum_MAX, FALSE );

	/* �t�H���_�̗���MAX */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX, FALSE );

	/* �^�X�N�g���C���g�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USETRAYICON, m_Common.m_sGeneral.m_bUseTaskTray );
// From Here 2001.12.03 hor
//@@@ YAZAKI 2001.12.31 �����͐��䂷��B
	if( m_Common.m_sGeneral.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
// To Here 2001.12.03 hor
	/* �^�X�N�g���C�ɏ풓 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STAYTASKTRAY, m_Common.m_sGeneral.m_bStayTaskTray );

	/* �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g */
	HotKey_SetHotKey( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ), m_Common.m_sGeneral.m_wTrayMenuHotKeyCode, m_Common.m_sGeneral.m_wTrayMenuHotKeyMods );

	return;
}





/* �_�C�A���O�f�[�^�̎擾 General */
int CDlgConfigChildGeneral::GetData()
{
	HWND hwndDlg = GetHwnd();

	/* �J�[�\���̃^�C�v 0=win 1=dos  */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE0 ) ){
		m_Common.m_sGeneral.SetCaretType(0);
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE1 ) ){
		m_Common.m_sGeneral.SetCaretType(1);
	}

	/* �t���[�J�[�\�����[�h */
	m_Common.m_sGeneral.m_bIsFreeCursorMode = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FREECARET ) != 0;

	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */
	m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD );
	//	2007.10.08 genta �}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ� (2007.10.02 by nasukoji)
	m_Common.m_sGeneral.m_bNoCaretMoveByActivation = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE );

	/* �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩 */
	m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH );

	/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
	m_Common.m_sGeneral.m_bCloseAllConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CLOSEALLCONFIRM );

	/* �I�����̊m�F������ */
	m_Common.m_sGeneral.m_bExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXITCONFIRM );

	/* �L�[���s�[�g���̃X�N���[���s�� */
	m_Common.m_sGeneral.m_nRepeatedScrollLineNum = (CLayoutInt)::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nRepeatedScrollLineNum < CLayoutInt(1) ){
		m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(1);
	}
	if( m_Common.m_sGeneral.m_nRepeatedScrollLineNum > CLayoutInt(10) ){
		m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(10);
	}

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH );

	// 2009.01.17 nasukoji	�g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
	HWND	hwndCombo;
	int		nSelPos;

	// 2007.09.09 Moca ��ʃL���b�V���ݒ�ǉ�
	// ��ʃL���b�V�����g��
	m_Common.m_sWindow.m_bUseCompatibleBMP = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_MEMDC );

	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL );
	nSelPos = Combo_GetCurSel( hwndCombo );
	m_Common.m_sGeneral.m_nPageScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;		// �y�[�W�X�N���[���Ƃ���g�ݍ��킹����

	// 2009.01.17 nasukoji	�g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WHEEL_HSCROLL );
	nSelPos = Combo_GetCurSel( hwndCombo );
	m_Common.m_sGeneral.m_nHorizontalScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;	// ���X�N���[���Ƃ���g�ݍ��킹����

	/* �t�@�C���̗���MAX */
	m_Common.m_sGeneral.m_nMRUArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nMRUArrNum_MAX < 0 ){
		m_Common.m_sGeneral.m_nMRUArrNum_MAX = 0;
	}
	if( m_Common.m_sGeneral.m_nMRUArrNum_MAX > MAX_MRU ){
		m_Common.m_sGeneral.m_nMRUArrNum_MAX = MAX_MRU;
	}

	{	//�����̊Ǘ�	//@@@ 2003.04.09 MIK
		CRecentFile	cRecentFile;
		cRecentFile.UpdateView();
		cRecentFile.Terminate();
	}

	/* �t�H���_�̗���MAX */
	m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX < 0 ){
		m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 0;
	}
	if( m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX > MAX_OPENFOLDER ){
		m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = MAX_OPENFOLDER;
	}

	{	//�����̊Ǘ�	//@@@ 2003.04.09 MIK
		CRecentFolder	cRecentFolder;
		cRecentFolder.UpdateView();
		cRecentFolder.Terminate();
	}

	/* �^�X�N�g���C���g�� */
	m_Common.m_sGeneral.m_bUseTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON );
//@@@ YAZAKI 2001.12.31 m_bUseTaskTray�Ɉ����Â���悤�ɁB
	if( m_Common.m_sGeneral.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
	/* �^�X�N�g���C�ɏ풓 */
	m_Common.m_sGeneral.m_bStayTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STAYTASKTRAY );

	/* �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g */
	LRESULT	lResult;
	lResult = HotKey_GetHotKey( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ) );
	m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = LOBYTE( lResult );
	m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HIBYTE( lResult );

	return TRUE;
}


LPVOID CDlgConfigChildGeneral::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
