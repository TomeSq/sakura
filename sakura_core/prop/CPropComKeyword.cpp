/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����L�[���[�h�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, KEITA
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildKeyword.h"
#include "prop/CDlgConfig.h"
#include "env/CShareData.h"
#include "env/CDocTypeManager.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "dlg/CDlgInput1.h"
#include "util/shell.h"
#include <memory>
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				HIDC_BUTTON_ADDSET,			//�L�[���[�h�Z�b�g�ǉ�
	IDC_BUTTON_DELSET,				HIDC_BUTTON_DELSET,			//�L�[���[�h�Z�b�g�폜
	IDC_BUTTON_ADDKEYWORD,			HIDC_BUTTON_ADDKEYWORD,		//�L�[���[�h�ǉ�
	IDC_BUTTON_EDITKEYWORD,			HIDC_BUTTON_EDITKEYWORD,	//�L�[���[�h�ҏW
	IDC_BUTTON_DELKEYWORD,			HIDC_BUTTON_DELKEYWORD,		//�L�[���[�h�폜
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYWORD,	//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYWORD,	//�G�N�X�|�[�g
	IDC_CHECK_KEYWORDCASE,			HIDC_CHECK_KEYWORDCASE,		//�L�[���[�h�̉p�啶�����������
	IDC_COMBO_SET,					HIDC_COMBO_SET,				//�����L�[���[�h�Z�b�g��
	IDC_LIST_KEYWORD,				HIDC_LIST_KEYWORD,			//�L�[���[�h�ꗗ
	IDC_BUTTON_KEYCLEAN		,		HIDC_BUTTON_KEYCLEAN,		//�L�[���[�h����	// 2006.08.06 ryoji
	IDC_BUTTON_KEYSETRENAME,		HIDC_BUTTON_KEYSETRENAME,	//�Z�b�g�̖��̕ύX	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildKeyword::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_KEYWORD, 0, SW_SHOW );
}


BOOL CDlgConfigChildKeyword::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	/* �R���g���[���̃n���h�����擾 */
	HWND hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	RECT		rc;
	LV_COLUMN	lvc;
	::GetWindowRect( hwndLIST_KEYWORD, &rc );
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = rc.right - rc.left;
	lvc.pszText = const_cast<TCHAR*>(_T(""));
	lvc.iSubItem = 0;
	ListView_InsertColumn( hwndLIST_KEYWORD, 0, &lvc );

	LONG_PTR lStyle = ::GetWindowLongPtr( hwndLIST_KEYWORD, GWL_STYLE );
	::SetWindowLongPtr( hwndLIST_KEYWORD, GWL_STYLE, lStyle | LVS_SHOWSELALWAYS );

	/* �R���g���[���X�V�̃^�C�~���O�p�̃^�C�}�[���N�� */
	::SetTimer( hwndDlg, 1, 300, NULL );

	return result;
}


BOOL CDlgConfigChildKeyword::OnNotify( WPARAM wParam, LPARAM lParam )
{
	HWND hwndDlg = GetHwnd();

	NMHDR*	pNMHDR = (NMHDR*)lParam;
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	if( hwndLIST_KEYWORD == pNMHDR->hwndFrom ){
		switch( pNMHDR->code ){
		case NM_DBLCLK:
//				MYTRACE( _T("NM_DBLCLK     \n") );
			/* ���X�g���őI������Ă���L�[���[�h��ҏW���� */
			Edit_List_KeyWord( hwndLIST_KEYWORD );
			return TRUE;
		case LVN_BEGINLABELEDIT:
		{
#ifdef _DEBUG
			LV_DISPINFO*	plvdi = (LV_DISPINFO*)lParam;
			LV_ITEM*		plvi = &plvdi->item;
			MYTRACE( _T("LVN_BEGINLABELEDIT\n") );
											MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
											MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
											MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
			if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
											MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
			if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText );
											MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
			if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
			if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
			return TRUE;
		}
		case LVN_ENDLABELEDIT:
		{
			LV_DISPINFO*	plvdi = (LV_DISPINFO*)lParam;
			LV_ITEM*		plvi = &plvdi->item;
#ifdef _DEBUG
			MYTRACE( _T("LVN_ENDLABELEDIT\n") );
											MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
											MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
											MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
			if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
											MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
			if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText  );
											MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
			if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
			if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
			if( NULL == plvi->pszText ){
				return TRUE;
			}
			if( plvi->pszText[0] != _T('\0') ){
				if( MAX_KEYWORDLEN < _tcslen( plvi->pszText ) ){
					InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_ERR_LEN), MAX_KEYWORDLEN );
					return TRUE;
				}
				/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW */
				m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
					plvi->lParam,
					to_wchar(plvi->pszText)
				);
			}else{
				/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
				m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam );
			}
			/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
			SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

			ListView_SetItemState( hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

			return TRUE;
		}
		case LVN_KEYDOWN:
//				MYTRACE( _T("LVN_KEYDOWN\n") );
		{
			LV_KEYDOWN*		pnkd = (LV_KEYDOWN*)lParam;
			switch( pnkd->wVKey ){
			case VK_DELETE:
				/* ���X�g���őI������Ă���L�[���[�h���폜���� */
				Delete_List_KeyWord( hwndLIST_KEYWORD );
				break;
			case VK_SPACE:
				/* ���X�g���őI������Ă���L�[���[�h��ҏW���� */
				Edit_List_KeyWord( hwndLIST_KEYWORD );
				break;
			}
			return TRUE;
		}
		}
	}
	return TRUE;
}


BOOL CDlgConfigChildKeyword::OnCbnSelChange( HWND hwndCtl, int wID )
{
	if( IDC_COMBO_SET == wID ){
		int nIndex1 = Combo_GetCurSel( hwndCtl );
		/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
		SetKeyWordSet( nIndex1 );
	}
	return TRUE;
}

BOOL CDlgConfigChildKeyword::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	int					nIndex1;
	wchar_t				szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1			cDlgInput1;
	
	switch( wID ){
	case IDC_BUTTON_ADDSET:	/* �Z�b�g�ǉ� */
		if( MAX_SETNUM <= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
			InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_SETMAX), MAX_SETNUM );
			return TRUE;
		}
		/* ���[�h���X�_�C�A���O�̕\�� */
		szKeyWord[0] = L'\0';
		//	Oct. 5, 2002 genta ���������̐ݒ���C���D�o�b�t�@�I�[�o�[�������Ă����D
		if( !cDlgInput1.DoModal(
			G_AppInstance(),
			hwndDlg,
			LS(STR_PROPCOMKEYWORD_SETNAME1),
			LS(STR_PROPCOMKEYWORD_SETNAME2),
			MAX_SETNAMELEN,
			szKeyWord
			)
		){
			return TRUE;
		}
		if( szKeyWord[0] != L'\0' ){
			/* �Z�b�g�̒ǉ� */
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( szKeyWord, false );

			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum - 1;

			/* �_�C�A���O�f�[�^�̐ݒ� Keyword */
			SetData();
		}
		return TRUE;
	case IDC_BUTTON_DELSET:	/* �Z�b�g�폜 */
		nIndex1 = Combo_GetCurSel( hwndCOMBO_SET );
		if( CB_ERR == nIndex1 ){
			return TRUE;
		}
		{
			/* �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v��� */
			std::tstring strLabel;
			for( size_t i = 0; i < m_pDlgConfigArg->m_Types_nKeyWordSetIdx.size() ; ++i ){
				// 2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ����Am_pShareData�𒼐ڌ��Ă����Ȃ��B
				for( int k = 0; k < MAX_KEYWORDSET_PER_TYPE; k++ ){
					if( nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[k] ){
						std::tstring name;
						std::tstring exts;
						bool bAdd = false;
						if( m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].typeId == -1 ){
							// �^�C�v�ʈꎞ�\��
							name = _T("(Temp)");
							name += m_pDlgConfigArg->m_tempTypeName;
							exts = m_pDlgConfigArg->m_tempTypeExts;
							bAdd = true;
						}else{
							const STypeConfigMini* type = NULL;
							CTypeConfig typeConfig = CDocTypeManager().GetDocumentTypeOfId( m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].typeId );
							if( CDocTypeManager().GetTypeConfigMini( typeConfig, &type ) ){
								name = type->m_szTypeName;
								exts = type->m_szTypeExts;
								bAdd = true;
							}
						}
						if( bAdd ){
							strLabel += _T("�E");
							strLabel += name;
							strLabel +=  _T("(");
							strLabel += exts;
							strLabel += _T(")\n");
						}
						break;
					}
				}
			}
			if( IDCANCEL == ::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
				LS(STR_PROPCOMKEYWORD_SETDEL),
				m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIndex1 ),
				strLabel.c_str()
			) ){
				return TRUE;
			}
			/* �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v�̃Z�b�g���N���A */
			for( size_t i = 0; i < m_pDlgConfigArg->m_Types_nKeyWordSetIdx.size(); ++i ){
				// 2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
				for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
					if( nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] ){
						m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] = -1;
					}
					else if( nIndex1 < m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] ){
						m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j]--;
					}
				}
			}
			/* ���Ԗڂ̃Z�b�g���폜 */
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
			/* �_�C�A���O�f�[�^�̐ݒ� Keyword */
			SetData();
		}
		return TRUE;
	case IDC_BUTTON_KEYSETRENAME: // �L�[���[�h�Z�b�g�̖��̕ύX
		// ���[�h���X�_�C�A���O�̕\��
		wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
		{
			BOOL bDlgInputResult = cDlgInput1.DoModal(
				G_AppInstance(),
				hwndDlg,
				LS(STR_PROPCOMKEYWORD_RENAME1),
				LS(STR_PROPCOMKEYWORD_RENAME2),
				MAX_SETNAMELEN,
				szKeyWord
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		if( szKeyWord[0] != L'\0' ){
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord );

			// �_�C�A���O�f�[�^�̐ݒ� Keyword
			SetData();
		}
		return TRUE;
	case IDC_CHECK_KEYWORDCASE:	/* �L�[���[�h�̉p�啶����������� */
//					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_bKEYWORDCASEArr[ m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ] = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );	//MIK 2000.12.01 case sense
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE ));			//MIK 2000.12.01 case sense
		return TRUE;
	case IDC_BUTTON_ADDKEYWORD:	/* �L�[���[�h�ǉ� */
		/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
		if( !m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CanAddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
			InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_KEYMAX) );
			return TRUE;
		}
		/* ���[�h���X�_�C�A���O�̕\�� */
		szKeyWord[0] = L'\0';
		if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYADD1), LS(STR_PROPCOMKEYWORD_KEYADD2), MAX_KEYWORDLEN, szKeyWord ) ){
			return TRUE;
		}
		if( szKeyWord[0] != L'\0' ){
			/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
			if( 0 == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord ) ){
				// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
				SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
			}
		}
		return TRUE;
	case IDC_BUTTON_EDITKEYWORD:	/* �L�[���[�h�ҏW */
		/* ���X�g���őI������Ă���L�[���[�h��ҏW���� */
		Edit_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	case IDC_BUTTON_DELKEYWORD:	/* �L�[���[�h�폜 */
		/* ���X�g���őI������Ă���L�[���[�h���폜���� */
		Delete_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	// From Here 2005.01.26 Moca
	case IDC_BUTTON_KEYCLEAN:
		Clean_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	// To Here 2005.01.26 Moca
	case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
		/* ���X�g���̃L�[���[�h���C���|�[�g���� */
		Import_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
		/* ���X�g���̃L�[���[�h���G�N�X�|�[�g���� */
		Export_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	}
	return FALSE;
}


BOOL CDlgConfigChildKeyword::OnDestroy()
{
	::KillTimer( GetHwnd(), 1 );
	return CDlgConfigChild::OnDestroy();
}

/* ���X�g���őI������Ă���L�[���[�h��ҏW���� */
void CDlgConfigChildKeyword::Edit_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex1;
	LV_ITEM		lvi;
	wchar_t		szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1	cDlgInput1;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );

	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
	wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam ) );

	/* ���[�h���X�_�C�A���O�̕\�� */
	if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYEDIT1), LS(STR_PROPCOMKEYWORD_KEYEDIT2), MAX_KEYWORDLEN, szKeyWord ) ){
		return;
	}
	if( szKeyWord[0] != L'\0' ){
		/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
			lvi.lParam,
			szKeyWord
		);
	}else{
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}



/* ���X�g���őI������Ă���L�[���[�h���폜���� */
void CDlgConfigChildKeyword::Delete_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex1;
	LV_ITEM		lvi;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );
	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	//�L�[���[�h����\������B
	DispKeywordCount();

	return;
}


/* ���X�g���̃L�[���[�h���C���|�[�g���� */
void CDlgConfigChildKeyword::Import_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	bool	bCase = false;
	int		nIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase( nIdx, bCase );
	CImpExpKeyWord	cImpExpKeyWord( m_Common, nIdx, bCase );

	// �C���|�[�g
	if (!cImpExpKeyWord.ImportUI( G_AppInstance(), hwndDlg )) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}

	/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	return;
}


/* ���X�g���̃L�[���[�h���G�N�X�|�[�g���� */
void CDlgConfigChildKeyword::Export_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	bool	bCase;
	CImpExpKeyWord	cImpExpKeyWord( m_Common, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, bCase );

	// �G�N�X�|�[�g
	if (!cImpExpKeyWord.ExportUI( G_AppInstance(), hwndDlg )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}


//! �L�[���[�h�𐮓ڂ���
void CDlgConfigChildKeyword::Clean_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	if( IDYES == ::MessageBox( hwndDlg, LS(STR_PROPCOMKEYWORD_DEL),
			GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){	// 2009.03.26 ryoji MB_ICONSTOP->MB_ICONQUESTION
		if( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CleanKeyWords( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
		}
		SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}
}

/* �_�C�A���O�f�[�^�̐ݒ� Keyword */
void CDlgConfigChildKeyword::SetData()
{
	HWND hwndDlg = GetHwnd();

	int		i;
	HWND	hwndWork;


	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	Combo_ResetContent( hwndWork );  /* �R���{�{�b�N�X����ɂ��� */
	if( 0 < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
		Combo_SetCurSel( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

		/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
		SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}else{
		/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
		SetKeyWordSet( -1 );
	}

	return;
}


/* �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ� */
void CDlgConfigChildKeyword::SetKeyWordSet( int nIdx )
{
	HWND hwndDlg = GetHwnd();

	int		i;
	int		nNum;
	HWND	hwndList;
	LV_ITEM	lvi;

	ListView_DeleteAllItems( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ) );
	if( 0 <= nIdx ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), TRUE );
		//	Jan. 29, 2005 genta �L�[���[�h�Z�b�g�؂�ւ�����̓L�[���[�h�͖��I��
		//	���̂��ߗL���ɂ��Ă����Ƀ^�C�}�[�Ŗ����ɂȂ�D
		//	�Ȃ̂ł����Ŗ����ɂ��Ă����D
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), FALSE );
		return;
	}

	/* �L�[���[�h�̉p�啶����������� */
	if( true == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase(nIdx) ){		//MIK 2000.12.01 case sense
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );
	}

	/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
	nNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( nIdx );
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	// 2005.01.25 Moca/genta ���X�g�ǉ����͍ĕ`���}�����Ă��΂₭�\��
	::SendMessageAny( hwndList, WM_SETREDRAW, FALSE, 0 );

	for( i = 0; i < nNum; ++i ){
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
		const TCHAR* pszKeyWord = to_tchar(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( nIdx, i ));

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = const_cast<TCHAR*>(pszKeyWord);
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem( hwndList, &lvi );

	}
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nIdx;

	// 2005.01.25 Moca/genta ���X�g�ǉ������̂��ߍĕ`�拖��
	::SendMessageAny( hwndList, WM_SETREDRAW, TRUE, 0 );

	//�L�[���[�h����\������B
	DispKeywordCount();

	return;
}



/* �_�C�A���O�f�[�^�̎擾 Keyword */
int CDlgConfigChildKeyword::GetData()
{
	return TRUE;
}

//�L�[���[�h����\������B
void CDlgConfigChildKeyword::DispKeywordCount()
{
	HWND hwndDlg = GetHwnd();

	HWND	hwndList;
	int		n;
	TCHAR szCount[ 256 ];

	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	n = ListView_GetItemCount( hwndList );
	if( n < 0 ) n = 0;

	int		nAlloc;
	nAlloc = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetAllocSize( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc -= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc += m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetFreeSize();
	
	auto_sprintf( szCount, LS(STR_PROPCOMKEYWORD_INFO), MAX_KEYWORDLEN, n, nAlloc );
	::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_KEYWORD_COUNT ), szCount );
}


LPVOID CDlgConfigChildKeyword::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
