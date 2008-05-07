/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date   2005/09/02 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, �����, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, ���イ��, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "view/CEditView.h"
#include "debug/Debug.h"
#include "debug/CRunningTimer.h"
#include "charset/charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgCancel.h"
#include "doc/CLayout.h"/// 2002/2/3 aroka
#include "COpe.h"///
#include "COpeBlk.h"///
#include "CDropTarget.h"///
#include "window/CSplitBoxWnd.h"///
#include "CRegexKeyword.h"///	//@@@ 2001.11.17 add MIK
#include "CMarkMgr.h"///
#include "COsVersionInfo.h"
#include "io/CFileLoad.h" // 2002/08/30 Moca
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "CClipboard.h"
#include "types/CTypeSupport.h"
#include "parse/CWordParse.h"
#include "convert/CConvert.h"
#include "charset/CCodeMediator.h"
#include "charset/CShiftJis.h"
#include "util/input.h"
#include "util/os.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/module.h"
#include "util/tchar_template.h"
#include "util/os.h" //WM_MOUSEWHEEL,IMR_RECONVERTSTRING,WM_XBUTTON*,IMR_CONFIRMRECONVERTSTRING


CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT, DWORD );

#define IDT_ROLLMOUSE	1



/*
|| �E�B���h�E�v���V�[�W��
||
*/

LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
)
{
//	DBPRINT_W(L"EditViewWndProc(0x%08X): %ls\n", hwnd, GetWindowsMessageName(uMsg));

	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessageCmd( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
			}
			else{
				return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
			//	To Here
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


/*
||  �^�C�}�[���b�Z�[�W�̃R�[���o�b�N�֐�
||
||	���݂́A�}�E�X�ɂ��̈�I�����̃X�N���[�������̂��߂Ƀ^�C�}�[���g�p���Ă��܂��B
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#pragma warning(disable:4355) //�uthis�|�C���^�����������X�g�Ŏg�p����܂����v�̌x���𖳌���

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditView::CEditView(CEditWnd* pcEditWnd)
: CViewCalc(this)
, m_pcEditWnd(pcEditWnd)
, m_cParser(this)
, m_cTextDrawer(this)
, m_cCommander(this)
, m_cViewSelect(this)
, m_AT_ImmSetReconvertString(NULL)
, m_bActivateByMouse( FALSE )	// 2007.10.02 nasukoji
{
}


// 2007.10.23 kobake �R���X�g���N�^���̏��������ׂ�Create�Ɉڂ��܂����B(�������������s�K�v�ɕ��U���Ă�������)
BOOL CEditView::Create(
	HWND		hwndParent,	//!< �e
	CEditDoc*	pcEditDoc,	//!< �Q�Ƃ���h�L�������g
	int			nMyIndex,	//!< �r���[�̃C���f�b�N�X
	BOOL		bShow		//!< �쐬���ɕ\�����邩�ǂ���
)
{
	m_pcTextArea = new CTextArea(this);
	m_pcCaret = new CCaret(this, pcEditDoc);
	m_pcRuler = new CRuler(this, pcEditDoc);
	m_pcFontset = new CViewFont();

	m_cHistory = new CAutoMarkMgr;
	m_cRegexKeyword = NULL;				// 2007.04.08 ryoji

	SetDrawSwitch(true);
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	_SetDragMode(FALSE);					/* �I���e�L�X�g�̃h���b�O���� */
	m_bCurSrchKeyMark = false;				/* ���������� */
	//	Jun. 27, 2001 genta
	m_szCurSrchKey[0] = L'\0';

	m_sCurSearchOption.Reset();				// �����^�u�� �I�v�V����

	m_bExecutingKeyMacro = false;			/* �L�[�{�[�h�}�N���̎��s�� */
	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��CEditWnd�ֈڊ�

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_bCommandRunning = FALSE;	/* �R�}���h�̎��s�� */
	m_pcOpeBlk = NULL;			/* ����u���b�N */
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */
	m_pcsbwVSplitBox = NULL;	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = NULL;	/* ���������{�b�N�X */
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* �����X�N���[���o�[�̏k�� */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;

	m_ptSrchStartPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));	//����/�u���J�n���̃J�[�\���ʒu  (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	m_bSearch = FALSE;					// ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1)); // �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	m_bDrawBracketPairFlag = FALSE;	/* 03/02/18 ai */
	GetSelectionInfo().m_bDrawSelectArea = false;	/* �I��͈͂�`�悵���� */	// 02/12/13 ai

	m_crBack = -1;				/* �e�L�X�g�̔w�i�F */			// 2006.12.16 ryoji



	/* ���[���[�\�� */
	GetTextArea().SetAreaTop(GetTextArea().GetAreaTop()+GetDllShareData().m_Common.m_sWindow.m_nRulerHeight);	/* ���[���[���� */
	GetRuler().SetRedrawFlag();	// ���[���[�S�̂�`��������=true   2002.02.25 Add By KK
	m_hdcCompatDC = NULL;		/* �ĕ`��p�R���p�`�u���c�b */
	m_hbmpCompatBMP = NULL;		/* �ĕ`��p�������a�l�o */
	m_hbmpCompatBMPOld = NULL;	/* �ĕ`��p�������a�l�o(OLD) */
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	m_nCompatBMPWidth = -1;
	m_nCompatBMPHeight = -1;
	// To Here 2007.09.09 Moca

	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	2007.08.12 genta ��������ShareData�̒l���K�v�ɂȂ���
	m_CurRegexp.Init(GetDllShareData().m_Common.m_sSearch.m_szRegexpLib );

	// 2004.02.08 m_hFont_ZEN�͖��g�p�ɂ��폜
	m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	m_bInMenuLoop = FALSE;				/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
//	MYTRACE_A( "CEditView::CEditView()�����\n" );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	// from here  2002.04.09 minfu OS�ɂ���čĕϊ��̕�����ς���
	//	YAZAKI COsVersionInfo�̃J�v�Z�����͎��܂���B
	COsVersionInfo	cOs;
	if( cOs.OsDoesNOTSupportReconvert() ){
		// 95 or NT�Ȃ��
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage( RWM_RECONVERT );
		m_uATOKReconvertMsg = ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		m_hAtokModule = LoadLibrary(_T("ATOK10WC.DLL"));
		m_AT_ImmSetReconvertString = NULL;
		if ( NULL != m_hAtokModule ) {
			m_AT_ImmSetReconvertString =(BOOL (WINAPI *)( HIMC , int ,PRECONVERTSTRING , DWORD  ) ) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}
	else{ 
		// ����ȊO��OS�̂Ƃ���OS�W�����g�p����
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
	//2004.10.23 isearch
	m_nISearchMode = 0;
	m_pcmigemo = NULL;

	// 2007.10.02 nasukoji
	m_dwTripleClickCheck = 0;		// �g���v���N���b�N�`�F�b�N�p����������



	//�����܂ŃR���X�g���N�^�ł���Ă�����
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�����܂�Create�ł���Ă�����

	WNDCLASS	wc;
//	HDC			hdc;	???
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;

	//	2007.08.18 genta ��������ShareData�̒l���K�v�ɂȂ���
	m_cRegexKeyword = new CRegexKeyword( GetDllShareData().m_Common.m_sSearch.m_szRegexpLib );	//@@@ 2001.11.17 add MIK
	m_cRegexKeyword->RegexKeySetTypes(&(m_pcEditDoc->m_cDocType.GetDocumentAttribute()));	//@@@ 2001.11.17 add MIK

	GetTextArea().SetTopYohaku( GetDllShareData().m_Common.m_sWindow.m_nRulerBottomSpace ); 	/* ���[���[�ƃe�L�X�g�̌��� */
	GetTextArea().SetAreaTop( GetTextArea().GetTopYohaku() );								/* �\����̏�[���W */
	/* ���[���[�\�� */
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		GetTextArea().SetAreaTop( GetTextArea().GetAreaTop() + GetDllShareData().m_Common.m_sWindow.m_nRulerHeight);	/* ���[���[���� */
	}


	/* �E�B���h�E�N���X�̓o�^ */
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= (WNDPROC)EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG_PTR );
	wc.hInstance		= G_AppInstance();
	wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_IBEAM )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= GSTR_VIEWNAME;
	if( 0 == ::RegisterClass( &wc ) ){
	}

	/* �G�f�B�^�E�B���h�E�̍쐬 */
	g_m_pcEditView = this;
	SetHwnd(
		::CreateWindowEx(
			WS_EX_STATICEDGE,		// extended window style
			GSTR_VIEWNAME,			// pointer to registered class name
			GSTR_VIEWNAME,			// pointer to window name
			0						// window style
			| WS_VISIBLE
			| WS_CHILD
			| WS_CLIPCHILDREN
			,
			CW_USEDEFAULT,			// horizontal position of window
			0,						// vertical position of window
			CW_USEDEFAULT,			// window width
			0,						// window height
			hwndParent,				// handle to parent or owner window
			NULL,					// handle to menu or child-window identifier
			G_AppInstance(),		// handle to application instance
			(LPVOID)this			// pointer to window-creation data
		)
	);
	if( NULL == GetHwnd() ){
		return FALSE;
	}

	m_pcDropTarget->Register_DropTarget( GetHwnd() );

	/* ����Tip�\���E�B���h�E�쐬 */
	m_cTipWnd.Create( G_AppInstance(), GetHwnd()/*GetDllShareData().m_hwndTray*/ );

	/* �ĕ`��p�R���p�`�u���c�b */
	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// 2007.09.30 genta �֐���
	UseCompatibleDC( GetDllShareData().m_Common.m_sWindow.m_bUseCompotibleBMP );

	/* ���������{�b�N�X */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( G_AppInstance(), GetHwnd(), TRUE );
	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( G_AppInstance(), GetHwnd(), FALSE );

	/* �X�N���[���o�[�쐬 */
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();

	if( bShow ){
		ShowWindow( GetHwnd(), SW_SHOW );
	}

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption();

	/* �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );

	/* �^�C�}�[�N�� */
	if( 0 == ::SetTimer( GetHwnd(), IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc ) ){
		WarningMessage( GetHwnd(), _T("CEditView::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
	}

	return TRUE;
}


CEditView::~CEditView()
{
	if( GetHwnd() != NULL ){
		DestroyWindow( GetHwnd() );
	}

	/* �ĕ`��p�R���p�`�u���c�b */
	//	2007.09.30 genta �֐���
	//	m_hbmpCompatBMP�������ō폜�����D
	UseCompatibleDC(FALSE);

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;

	delete m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	
	//�ĕϊ� 2002.04.10 minfu
	if(m_hAtokModule)
		FreeLibrary(m_hAtokModule);

	delete m_pcTextArea;
	delete m_pcCaret;
	delete m_pcRuler;
	delete m_pcFontset;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//TCHAR��WCHAR�ϊ��B
inline wchar_t tchar_to_wchar(TCHAR tc)
{
#ifdef _UNICODE
	return tc;
#else
	WCHAR wc=0;
	mbtowc(&wc,&tc,sizeof(tc));
	return wc;
#endif
}

/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT CEditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	HDC			hdc;
	int			nPosX;
	int			nPosY;

	switch ( uMsg ){
	case WM_MOUSEWHEEL:
		if( m_pcEditDoc->m_pcEditWnd->DoMouseWheel( wParam, lParam ) ){
			return 0L;
		}
		return OnMOUSEWHEEL( wParam, lParam );

	case WM_CREATE:
		::SetWindowLongPtr( hwnd, 0, (LONG_PTR) this );

		return 0L;

		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		// �E�B���h�E��\���̍ĂɌ݊�BMP��p�����ă������[��ߖ񂷂�
		if( hwnd == GetHwnd() && (BOOL)wParam == FALSE ){
			DeleteCompatibleBitmap();
		}
		return 0L;
	// To Here 2007.09.09 Moca

	case WM_SIZE:
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;

	case WM_SETFOCUS:
		OnSetFocus();

		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_pcEditWnd->UpdateCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

		return 0L;
	case WM_CHAR:
		GetCommander().HandleCommand( F_WCHAR, TRUE, tchar_to_wchar((TCHAR)wParam), 0, 0, 0 );
		return 0L;

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			GetCaret().ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_IME_COMPOSITION:
		if( IsInsMode() && (lParam & GCS_RESULTSTR)){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//	maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			LPTSTR lptstr;
			lptstr = (LPTSTR)GlobalLock( hstr );
			if( lptstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lptstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lptstr, dwSize);

			/* �e�L�X�g��\��t�� */
			GetCommander().HandleCommand( F_INSTEXT_W, TRUE, (LPARAM)to_wchar(lptstr), TRUE, 0, 0 );

			ImmReleaseContext( hwnd, hIMC );

			// add this string into text buffer of application

			GlobalUnlock( hstr );
			GlobalFree( hstr );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
//			return 0;
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_IME_CHAR:
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){ /* �㏑�����[�h���H */
			GetCommander().HandleCommand( F_IME_CHAR, TRUE, wParam, 0, 0, 0 );
		}
		return 0L;

	case WM_KEYUP:
		/* �L�[���s�[�g��� */
		GetCommander().m_bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+x��ALT���������܂܂��ƃL�[���s�[�g��OFF�ɂȂ�Ȃ��΍�
	case WM_SYSKEYUP:
		GetCommander().m_bPrevCommand = 0;
		// �O�̂��ߌĂ�
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	// 2004.04.27 To Here

	case WM_LBUTTONDBLCLK:

		// 2007.10.02 nasukoji	��A�N�e�B�u�E�B���h�E�̃_�u���N���b�N���͂����ŃJ�[�\�����ړ�����
		// 2007.10.12 genta �t�H�[�J�X�ړ��̂��߁COnLBUTTONDBLCLK���ړ�
		if(m_bActivateByMouse){
			/* �A�N�e�B�u�ȃy�C����ݒ� */
			m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
			// �J�[�\�����N���b�N�ʒu�ֈړ�����
			OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );	
			// 2007.10.02 nasukoji
			m_bActivateByMouse = FALSE;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
		}
		//		MYTRACE_A( " WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/11 �}�E�X���{�^���Ή�
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		m_bActivateByMouse = FALSE;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
//		MYTRACE_A( " WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
//	case WM_MBUTTONUP:
	case WM_LBUTTONUP:

//		MYTRACE_A( " WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
//		MYTRACE_A( " WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE_A( " WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->m_pcEditWnd->GetActivePane() ){
//			/* �A�N�e�B�u�ȃy�C����ݒ� */
//			m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE_A( " WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
	case WM_XBUTTONDOWN:
		switch ( HIWORD(wParam) ){
		case XBUTTON1:
			OnXLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		case XBUTTON2:
			OnXRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		}

		return 0L;

	case WM_VSCROLL:
//		MYTRACE_A( "	WM_VSCROLL nPos=%d\n", GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			CLayoutInt Scroll = OnVScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) * m_nVScrollRate );

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if(!GetKeyState_Shift()){
				SyncScrollV( Scroll );
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE_A( "	WM_HSCROLL nPos=%d\n", GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			CLayoutInt Scroll = OnHScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) );

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if(!GetKeyState_Shift()){
				SyncScrollH( Scroll );
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */

		/* ����Tip���N������Ă��� */
		if( 0 == m_dwTipTimer ){
			/* ����Tip������ */
			m_cTipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
		}
		if( m_bHokan ){
			m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
		return 0L;


	case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			hdc = ::BeginPaint( hwnd, &ps );
			OnPaint( hdc, &ps, FALSE );
			::EndPaint(hwnd, &ps);
		}
		return 0L;

	case WM_CLOSE:
//		MYTRACE_A( "	WM_CLOSE\n" );
		DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
//		CDropTarget::Revoke_DropTarget();
		m_pcDropTarget->Revoke_DropTarget();
//		::RevokeDragDrop( GetHwnd() );
//		::OleUninitialize();

		/* �^�C�}�[�I�� */
		::KillTimer( GetHwnd(), IDT_ROLLMOUSE );


//		MYTRACE_A( "	WM_DESTROY\n" );
		/*
		||�q�E�B���h�E�̔j��
		*/
		if( NULL != m_hwndVScrollBar ){	// Aug. 20, 2005 Aroka
			DestroyWindow( m_hwndVScrollBar );
			m_hwndVScrollBar = NULL;
		}
		if( NULL != m_hwndHScrollBar ){
			DestroyWindow( m_hwndHScrollBar );
			m_hwndHScrollBar = NULL;
		}
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		if( NULL != m_pcsbwVSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}

		if( NULL != m_pcsbwHSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}


		SetHwnd(NULL);
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE_A( "MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );
		::SendMessageAny( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
		::PostMessageAny( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;

	case MYWM_IME_REQUEST:  /* �ĕϊ�  by minfu 2002.03.27 */ // 20020331 aroka
		
		// 2002.04.09 switch case �ɕύX  minfu 
		switch ( wParam ){
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		}
		
		return 0L;
	
	// 2007.10.02 nasukoji	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ�
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc( hwnd, uMsg, wParam, lParam );	// �e�ɐ�ɏ���������
		if( nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT ){
			return nRes;
		}

		// �}�E�X�N���b�N�ɂ��o�b�N�O���E���h�E�B���h�E���A�N�e�B�x�[�g���ꂽ
		//	2007.10.08 genta �I�v�V�����ǉ�
		if( GetDllShareData().m_Common.m_sGeneral.m_bNoCaretMoveByActivation &&
		   (! m_pcEditDoc->m_pcEditWnd->IsActiveApp()))
		{
			m_bActivateByMouse = TRUE;		// �}�E�X�ɂ��A�N�e�B�x�[�g
			return MA_ACTIVATEANDEAT;		// �A�N�e�B�x�[�g��C�x���g��j��
		}

		/* �A�N�e�B�u�ȃy�C����ݒ� */
		m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );

		return nRes;

	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))){
		// 2002.04.08 switch case �ɕύX minfu 
			switch ( wParam ){
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
				
			}
			return 0L;
		}
// >> by aroka

		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �E�B���h�E�C�x���g                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( GetHwnd(), x, y, nWidth, nHeight, TRUE );
	return;
}


/* �E�B���h�E�T�C�Y�̕ύX���� */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == GetHwnd() 
		|| ( cx == 0 && cy == 0 ) ){
		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		// �E�B���h�E�������ɂ��݊�BMP��j������
		DeleteCompatibleBitmap();
		// To Here 2007.09.09 Moca
		return;
	}

	int	nVSplitHeight = 0;	/* ���������{�b�N�X�̍��� */
	int	nHSplitWidth  = 0;	/* ���������{�b�N�X�̕� */

	//�X�N���[���o�[�̃T�C�Y��l���擾
	int nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	/* ���������{�b�N�X */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->GetHwnd(), cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* ���������{�b�N�X */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->GetHwnd(),0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE );
	}

	/* �T�C�Y�{�b�N�X */
	if( NULL != m_hwndSizeBox ){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}

	// �G���A���X�V
	GetTextArea().TextArea_OnSize(
		CMySize(cx,cy),
		nCxVScroll,
		m_hwndHScrollBar?nCyHScroll:0
	);

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �ĕ`��p�������a�l�o */
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( m_hdcCompatDC != NULL ){
		CreateOrUpdateCompatibleBitmap( cx, cy );
 	}
	// To Here 2007.09.09 Moca

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption(); // [Q] genta �{���ɕK�v�H

	return;
}



/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
void CEditView::OnSetFocus( void )
{
	// 2004.04.02 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	{
		CLayoutPoint ptPos = GetCaret().GetCaretLayoutPos();
		if( GetCaret().GetAdjustCursorPos( &ptPos ) ){
			GetCaret().MoveCursor( ptPos, FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	GetCaret().ShowEditCaret();

	SetIMECompFormFont();

	/* ���[���̃J�[�\�����O���[���獕�ɕύX���� */
	HDC hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	// 03/02/18 �Ί��ʂ̋����\��(�`��) ai
	m_bDrawBracketPairFlag = TRUE;
	DrawBracketPair( true );
}


/* ���̓t�H�[�J�X���������Ƃ��̏��� */
void CEditView::OnKillFocus( void )
{
	// 03/02/18 �Ί��ʂ̋����\��(����) ai
	DrawBracketPair( false );
	m_bDrawBracketPairFlag = FALSE;

	GetCaret().DestroyCaret();

	/* ���[���[�`�� */
	/* ���[���̃J�[�\����������O���[�ɕύX���� */
	HDC	hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}

	if( m_bHokan ){
		m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ݒ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �t�H���g�̕ύX */
void CEditView::SetFont( void )
{
	// ���g���N�X�X�V
	this->GetTextMetrics().Update(GetFontset().GetFontHan());

	// �G���A�����X�V
	HDC hdc = ::GetDC( GetHwnd() );
	GetTextArea().UpdateAreaMetrics(hdc);
	::ReleaseDC( GetHwnd(), hdc );

	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	GetTextArea().DetectWidthOfLineNumberArea( false );

	// ����ԍĕ`��
	::InvalidateRect( GetHwnd(), NULL, TRUE );

	//	Oct. 11, 2002 genta IME�̃t�H���g���ύX
	SetIMECompFormFont();
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �L�����b�g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief �I�����l�������s���w��ɂ��J�[�\���ړ�

	�I����ԃ`�F�b�N���J�[�\���ړ����I��̈�X�V�Ƃ���������
	���������̃R�}���h�ɂ���̂ł܂Ƃ߂邱�Ƃɂ����D
	�܂��C�߂�l�͂قƂ�ǎg���Ă��Ȃ��̂�void�ɂ����D

	�I����Ԃ��l�����ăJ�[�\�����ړ�����D
	��I�����w�肳�ꂽ�ꍇ�ɂ͊����I��͈͂��������Ĉړ�����D
	�I�����w�肳�ꂽ�ꍇ�ɂ͑I��͈͂̊J�n�E�ύX�𕹂��čs���D
	�C���^���N�e�B�u�����O��Ƃ��邽�߁C�K�v�ɉ������X�N���[�����s���D
	�J�[�\���ړ���͏㉺�ړ��ł��J�����ʒu��ۂ悤�C
	GetCaret().m_nCaretPosX_Prev�̍X�V�������čs���D

	@date 2006.07.09 genta �V�K�쐬
*/
void CEditView::MoveCursorSelecting(
	CLayoutPoint	ptWk_CaretPos,		//!< [in] �ړ��惌�C�A�E�g�ʒu
	bool			bSelect,			//!< true: �I������  false: �I������
	int				nCaretMarginRate	//!< �c�X�N���[���J�n�ʒu�����߂�l
)
{
	if( bSelect ){
		if( !GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	GetCaret().MoveCursor( ptWk_CaretPos, TRUE, nCaretMarginRate );	// 2007.08.22 ryoji nCaretMarginRate���g���Ă��Ȃ�����
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
	if( bSelect ){
		/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
		
			2004.04.02 Moca 
			�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
			�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
			nPosX, nPosY�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
		*/
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
	}
	
}






// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�w��J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�

	2007.01.18 kobake URL������̎󂯎���wstring�ōs���悤�ɕύX
*/
bool CEditView::IsCurrentPositionURL(
	const CLayoutPoint&	ptCaretPos,		//!< [in]  �J�[�\���ʒu
	CLogicRange*		pUrlRange,		//!< [out] URL�͈́B���W�b�N�P�ʁB
	std::wstring*		pwstrURL		//!< [out] URL������󂯎���BNULL���w�肵���ꍇ��URL��������󂯎��Ȃ��B
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::IsCurrentPositionURL" );


	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptXY;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		ptCaretPos,
		&ptXY
	);
	pUrlRange->SetLine(ptXY.GetY2());
//	*pnUrlLine = ptXY.GetY2();
	CLogicInt		nLineLen;
	const wchar_t*	pLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen); //2007.10.09 kobake ���C�A�E�g�E���W�b�N���݃o�O�C��

	int			nUrlLen;
	CLogicInt	i = ptXY.GetX2() - CLogicInt(200);
	if( i < CLogicInt(0) ){
		i = CLogicInt(0);
	}
	for( ; i <= ptXY.GetX2() && i < nLineLen && i < ptXY.GetX2() + CLogicInt(200); ){
	/* �J�[�\���ʒu����O����250�o�C�g�܂ł͈͓̔��ōs���Ɍ������ăT�[�` */
		/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
		if( !IsURL( &pLine[i], (Int)(nLineLen - i), &nUrlLen ) ){
			++i;
		}
		else{
			if( i <= ptXY.GetX2() && ptXY.GetX2() < i + CLogicInt(nUrlLen) ){
				/* URL��Ԃ��ꍇ */
				if( pwstrURL ){
					pwstrURL->assign(&pLine[i],nUrlLen);
				}
				pUrlRange->SetLine(ptXY.GetY2());
				pUrlRange->SetXs(i, i+CLogicInt(nUrlLen));
				return true;
			}else{
				i += CLogicInt(nUrlLen);
			}
		}
	}
	return false;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VOID CEditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
	)
{
	POINT		po;
	RECT		rc;

	if( GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		if( m_bDragSource ){
			return;
		}
	}
	/* �͈͑I�𒆂łȂ��ꍇ */
	if(!GetSelectionInfo().IsMouseSelecting()){
		if(TRUE == KeyWordHelpSearchDict( LID_SKH_ONTIMER, &po, &rc ) ){	// 2006.04.10 fon
			/* ����Tip��\�� */
			m_cTipWnd.Show( po.x, po.y + GetTextMetrics().GetHankakuHeight(), NULL );
		}
	}
	else{
		::GetCursorPos( &po );
		::GetWindowRect(GetHwnd(), &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, GetSelectionInfo().m_ptMouseRollPosOld.x, GetSelectionInfo().m_ptMouseRollPosOld.y );
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ϊ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
void CEditView::ConvSelectedArea( EFunctionCode nFuncCode )
{
	CNativeW	cmemBuf;

	CLayoutPoint sPos;

	CLogicInt	nIdxFrom;
	CLogicInt	nIdxTo;
	CLayoutInt	nLineNum;
	CLogicInt	nDelLen;
	CLogicInt	nDelPosNext;
	CLogicInt	nDelLenNext;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;
	CWaitCursor cWaitCursor( GetHwnd() );

	BOOL		bBeginBoxSelectOld;

	/* �e�L�X�g���I������Ă��邩 */
	if( !GetSelectionInfo().IsTextSelected() ){
		return;
	}

	CLayoutRange sSelectOld = GetSelectionInfo().m_sSelect;		// �͈͑I��

	bBeginBoxSelectOld	= GetSelectionInfo().IsBoxSelecting();

	/* ��`�͈͑I�𒆂� */
	if( GetSelectionInfo().IsBoxSelecting() ){

		/* 2�_��Ίp�Ƃ����`�����߂� */
		CLayoutRect	rcSelLayout;
		TwoPointToRect(
			&rcSelLayout,
			GetSelectionInfo().m_sSelect.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().m_sSelect.GetTo()		// �͈͑I���I��
		);

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		GetSelectionInfo().DisableSelectArea( TRUE );

		nIdxFrom = CLogicInt(0);
		nIdxTo = CLogicInt(0);
		for( nLineNum = rcSelLayout.bottom; nLineNum >= rcSelLayout.top - 1; nLineNum-- ){
			const CLayout* pcLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSelLayout.left );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSelLayout.right );

				for( CLogicInt i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == WCODE::CR || pLine[i] == WCODE::LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= CLogicInt(0);
				nIdxTo		= CLogicInt(0);
			}
			CLogicInt	nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSelLayout.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + CLayoutInt(1), &nLineLen2, &pcLayout );
				sPos.Set(
					LineIndexToColmn( pcLayout, nDelPos ),
					nLineNum + 1
				);

				//2007.10.18 COpe����
				// �w��ʒu�̎w�蒷�f�[�^�폜
				DeleteData2(
					sPos,
					nDelLen,
					&cmemBuf
				);
				
				{
					/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
					CConvertMediator::ConvMemory( &cmemBuf, nFuncCode, (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );

					/* ���݈ʒu�Ƀf�[�^��}�� */
					CLayoutPoint ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
					InsertData_CEditView(
						sPos,
						cmemBuf.GetStringPtr(),
						cmemBuf.GetStringLength(),
						&ptLayoutNew,
						true
					);

					/* �J�[�\�����ړ� */
					GetCaret().MoveCursor( ptLayoutNew, FALSE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}
		}
		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
		GetCaret().MoveCursor( rcSelLayout.UpperLeft(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
	}
	else{
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy );

		/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
		CConvertMediator::ConvMemory( &cmemBuf, nFuncCode, (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );

//		/* �I���G���A���폜 */
//		DeleteData( FALSE );

		CLayoutInt nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();

		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			GetSelectionInfo().m_sSelect,
			NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			cmemBuf.GetStringPtr(),		/* �}������f�[�^ */ // 2002/2/10 aroka CMemory�ύX
			cmemBuf.GetStringLength(),	/* �}������f�[�^�̒��� */ // 2002/2/10 aroka CMemory�ύX
			false,
			m_bDoing_UndoRedo?NULL:m_pcOpeBlk
		);

		// From Here 2001.12.03 hor
		//	�I���G���A�̕���
		GetSelectionInfo().m_sSelect.SetFrom(sSelectOld.GetFrom());	// �͈͑I���J�n�ʒu
		GetSelectionInfo().m_sSelect.SetTo(GetCaret().GetCaretLayoutPos());	// �͈͑I���I���ʒu
		if(nCaretPosYOLD==GetSelectionInfo().m_sSelect.GetFrom().y) {
			GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), TRUE );
		}else{
			GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetTo(), TRUE );
		}
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		RedrawAll();
		// To Here 2001.12.03 hor
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���j���[                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
	HMENU		hMenu;
	POINT		po;
	RECT		rc;
	CMemory		cmemCurText;
	int			i;
	int			nMenuIdx;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];
	UINT		uFlags;


	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();

	/* �E�N���b�N���j���[�̒�`�̓J�X�^�����j���[�z���0�Ԗ� */
	nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	//�}�W�b�N�i���o�[�r��	//@@@ 2003.06.13 MIK

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
	for( i = 0; i < GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( L'\0' == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				auto_strcpy( szLabel2, szLabel );
			}else{
				auto_sprintf( szLabel2, LTEXT("%ls (&%lc)"),
					szLabel,
					GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* �@�\�����p�\�����ׂ� */
			if( IsFuncEnable( m_pcEditDoc, &GetDllShareData(), GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->GetMenuDrawer().MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );

		}
	}

	if( !GetSelectionInfo().IsMouseSelecting() ){	/* �͈͑I�� */
		if( TRUE == KeyWordHelpSearchDict( LID_SKH_POPUPMENU_R, &po, &rc ) ){	// 2006.04.10 fon
			const TCHAR*	pszWork;
			pszWork = m_cTipWnd.m_cInfo.GetStringPtr();
			// 2002.05.25 Moca &�̍l����ǉ� 
			TCHAR*	pszShortOut = new TCHAR[160 + 1];
			if( 80 < _tcslen( pszWork ) ){
				TCHAR*	pszShort = new TCHAR[80 + 1];
				auto_memcpy( pszShort, pszWork, 80 );
				pszShort[80] = _T('\0');
				dupamp( pszShort, pszShortOut );
				delete [] pszShort;
			}else{
				dupamp( pszWork, pszShortOut );
			}
			::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, _T("�L�[���[�h�̐������N���b�v�{�[�h�ɃR�s�[(&K)") );	// 2006.04.10 fon ToolTip���e�𒼐ڕ\������̂���߂�
			delete [] pszShortOut;
			::InsertMenu( hMenu, 1, MF_BYPOSITION, IDM_JUMPDICT, _T("�L�[���[�h�������J��(&J)") );	// 2006.04.10 fon
			::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		}
	}
	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent( m_hwndParent )/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �ݒ蔽�f                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �ݒ�ύX�𔽉f������ */
void CEditView::OnChangeSetting()
{
	RECT		rc;

	GetTextArea().SetTopYohaku( GetDllShareData().m_Common.m_sWindow.m_nRulerBottomSpace ); 		/* ���[���[�ƃe�L�X�g�̌��� */
	GetTextArea().SetAreaTop( GetTextArea().GetTopYohaku() );									/* �\����̏�[���W */

	/* ���[���[�\�� */
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		GetTextArea().SetAreaTop(GetTextArea().GetAreaTop() + GetDllShareData().m_Common.m_sWindow.m_nRulerHeight);	/* ���[���[���� */
	}

	// �t�H���g�X�V
	GetFontset().UpdateFont();

	/* �t�H���g�̕ύX */
	SetFont();

	/* �t�H���g���ς���Ă��邩������Ȃ��̂ŁA�J�[�\���ړ� */
	GetCaret().MoveCursor( GetCaret().GetCaretLayoutPos(), TRUE );

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	//	2007.09.30 genta ��ʃL���b�V���pCompatibleDC��p�ӂ���
	UseCompatibleDC( GetDllShareData().m_Common.m_sWindow.m_bUseCompotibleBMP );

	/* �E�B���h�E�T�C�Y�̕ύX���� */
	::GetClientRect( GetHwnd(), &rc );
	OnSize( rc.right, rc.bottom );

	/* �ĕ`�� */
	::InvalidateRect( GetHwnd(), NULL, TRUE );
}


/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
void CEditView::CopyViewStatus( CEditView* pView ) const
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	// ���͏��
	GetCaret().CopyCaretStatus(&pView->GetCaret());

	// �I�����
	GetSelectionInfo().CopySelectStatus(&pView->GetSelectionInfo());

	/* ��ʏ�� */
	GetTextArea().CopyTextAreaStatus(&pView->GetTextArea());

	/* �\�����@ */
	GetTextMetrics().CopyTextMetricsStatus(&pView->GetTextMetrics());
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �����{�b�N�X                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( m_pcsbwVSplitBox == NULL ){	/* ���������{�b�N�X */
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( G_AppInstance(), GetHwnd(), TRUE );
		}
	}
	else{
		SAFE_DELETE(m_pcsbwVSplitBox);	/* ���������{�b�N�X */
	}
	if( bHorz ){
		if( m_pcsbwHSplitBox == NULL ){	/* ���������{�b�N�X */
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( G_AppInstance(), GetHwnd(), FALSE );
		}
	}
	else{
		SAFE_DELETE(m_pcsbwHSplitBox);	/* ���������{�b�N�X */
	}

	if( bSizeBox ){
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,													/* no extended styles */
			_T("SCROLLBAR"),										/* scroll bar control class */
			NULL,												/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			GetHwnd(),												/* handle of main window */
			(HMENU) NULL,										/* no menu for a scroll bar */
			G_AppInstance(),										/* instance owning this window */
			(LPVOID) NULL										/* pointer not needed */
		);
	}else{
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,														/* no extended styles */
			_T("STATIC"),											/* scroll bar control class */
			NULL,													/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			GetHwnd(),													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			G_AppInstance(),											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( GetHwnd(), &rc );
	OnSize( rc.right, rc.bottom );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �e�L�X�g�I��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �I��͈͂̃f�[�^���擾 */
/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
bool CEditView::GetSelectedData(
	CNativeW*		cmemBuf,
	BOOL			bLineOnly,
	const wchar_t*	pszQuote,			/* �擪�ɕt������p�� */
	BOOL			bWithLineNumber,	/* �s�ԍ���t�^���� */
	BOOL			bAddCRLFWhenCopy,	/* �܂�Ԃ��ʒu�ŉ��s�L�������� */
	enumEOLType		neweol				//	�R�s�[��̉��s�R�[�h EOL_NONE�̓R�[�h�ۑ�
)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayoutInt		nLineNum;
	CLogicInt		nIdxFrom;
	CLogicInt		nIdxTo;
	int				nRowNum;
	int				nLineNumCols;
	wchar_t*		pszLineNum;
	wchar_t*		pszSpaces = L"                    ";
	const CLayout*	pcLayout;
	CEol			appendEol( neweol );
	bool			addnl = false;

	/* �͈͑I��������Ă��Ȃ� */
	if( !GetSelectionInfo().IsTextSelected() ){
		return false;
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		nLineNumCols = GetTextArea().DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new wchar_t[nLineNumCols + 1];
	}

	CLayoutRect			rcSel;

	if( GetSelectionInfo().IsBoxSelecting() ){	/* ��`�͈͑I�� */
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().m_sSelect.GetTo()		// �͈͑I���I��
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		// �T�C�Y�������v�̂��Ƃ��Ă����B
		// ���\��܂��Ɍ��Ă��܂��B
		CLayoutInt i = rcSel.bottom - rcSel.top;

		// �ŏ��ɍs�����̉��s�ʂ��v�Z���Ă��܂��B
		int nBufSize = wcslen(WCODE::CRLF) * (Int)i;

		// ���ۂ̕����ʁB
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
		for(; i != CLayoutInt(0) && pcLayout != NULL; i--, pcLayout = pcLayout->GetNextLayout())
		{
			pLine = pcLayout->GetPtr() + pcLayout->GetLogicOffset();
			nLineLen = CLogicInt(pcLayout->GetLengthWithEOL());
			if( NULL != pLine )
			{
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				nBufSize += nIdxTo - nIdxFrom;
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ��܂��Ɍ����e�ʂ����ɃT�C�Y�����炩���ߊm�ۂ��Ă����B
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );
				//2002.02.08 hor
				// pLine��NULL�̂Ƃ�(��`�G���A�̒[��EOF�݂̂̍s���܂ނƂ�)�͈ȉ����������Ȃ�
				if( nIdxTo - nIdxFrom > 0 ){
					if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
					}else{
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					}
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf->AppendString( WCODE::CRLF );
				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
					break;
				}
//			}
		}
	}
	else{
		cmemBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		//  ���ꂩ��\��t���Ɏg���̈�̑�܂��ȃT�C�Y���擾����B
		//  ��܂��Ƃ������x���ł��̂ŁA�T�C�Y�v�Z�̌덷���i�e�ʂ𑽂����ς�����Ɂj���\�o��Ǝv���܂����A
		// �܂��A�����D��Ƃ������ƂŊ��ق��Ă��������B
		//  ���ʂȗe�ʊm�ۂ��o�Ă��܂��̂ŁA�����������x���グ�����Ƃ���ł����E�E�E�B
		//  �Ƃ͂����A�t�ɏ��������ς��邱�ƂɂȂ��Ă��܂��ƁA���Ȃ葬�x���Ƃ���v���ɂȂ��Ă��܂��̂�
		// �����Ă��܂��Ƃ���ł����E�E�E�B
		m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetSelectionInfo().m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		int nBufSize = 0;

		int i = (Int)(GetSelectionInfo().m_sSelect.GetTo().y - GetSelectionInfo().m_sSelect.GetFrom().y);

		// �擪�Ɉ��p����t����Ƃ��B
		if ( NULL != pszQuote )
		{
			nBufSize += wcslen(pszQuote);
		}

		// �s�ԍ���t����B
		if ( bWithLineNumber )
		{
			nBufSize += nLineNumCols;
		}

		// ���s�R�[�h�ɂ��āB
		if ( neweol == EOL_UNKNOWN )
		{
			nBufSize += wcslen(WCODE::CRLF);
		}
		else
		{
			nBufSize += appendEol.GetLen();
		}

		// ���ׂĂ̍s�ɂ��ē��l�̑��������̂ŁA�s���{����B
		nBufSize *= (Int)i;

		// ���ۂ̊e�s�̒����B
		for (; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->GetNextLayout() )
		{
			nBufSize += pcLayout->GetLengthWithEOL() + appendEol.GetLen();
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ���ׂ������������o�b�t�@������Ă����B
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for( nLineNum = GetSelectionInfo().m_sSelect.GetFrom().GetY2(); nLineNum <= GetSelectionInfo().m_sSelect.GetTo().y; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == GetSelectionInfo().m_sSelect.GetFrom().y ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetFrom().GetX2() );
			}else{
				nIdxFrom = CLogicInt(0);
			}
			if( nLineNum == GetSelectionInfo().m_sSelect.GetTo().y ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxTo = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetTo().GetX2() );
			}else{
				nIdxTo = nLineLen;
			}
			if( nIdxTo - nIdxFrom == CLogicInt(0) ){
				continue;
			}

			if( NULL != pszQuote && 0 < wcslen( pszQuote ) ){	/* �擪�ɕt������p�� */
				cmemBuf->AppendString( pszQuote );
			}
			if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
				auto_sprintf( pszLineNum, L" %d:" , nLineNum + 1 );
				cmemBuf->AppendString( pszSpaces, nLineNumCols - wcslen( pszLineNum ) );
				cmemBuf->AppendString( pszLineNum );
			}


			if( EOL_NONE != pcLayout->GetLayoutEol() ){
//			if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
//				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
//				cmemBuf.AppendSz( CRLF );

				if( nIdxTo >= nLineLen ){
					cmemBuf->AppendString( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf->AppendString( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->GetLayoutEol()).GetValue2() :	//	�R�[�h�ۑ�
						appendEol.GetValue2() );			//	�V�K���s�R�[�h
				}
				else {
					cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				if( nIdxTo - nIdxFrom >= nLineLen ){
					if( bAddCRLFWhenCopy ||  /* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
						NULL != pszQuote || /* �擪�ɕt������p�� */
						bWithLineNumber 	/* �s�ԍ���t�^���� */
					){
//						cmemBuf.Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf->AppendString(( neweol == EOL_UNKNOWN ) ?
							WCODE::CRLF :						//	�R�[�h�ۑ�
							appendEol.GetValue2() );		//	�V�K���s�R�[�h
					}
				}
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		delete [] pszLineNum;
	}
	return true;
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int CEditView::IsCurrentPositionSelected(
	CLayoutPoint	ptCaretPos		// �J�[�\���ʒu
)
{
	if( !GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}
	CLayoutRect		rcSel;
	CLayoutPoint	po;

	/* ��`�͈͑I�𒆂� */
	if( GetSelectionInfo().IsBoxSelecting() ){
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().m_sSelect.GetTo()		// �͈͑I���I��
		);
		++rcSel.bottom;
		po = ptCaretPos;
		if( m_bDragSource ){
			if( GetKeyState_Control() ){ /* Ctrl�L�[��������Ă����� */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( rcSel.PtInRect( po ) ){
			return 0;
		}
		if( rcSel.top > ptCaretPos.y ){
			return -1;
		}
		if( rcSel.bottom < ptCaretPos.y ){
			return 1;
		}
		if( rcSel.left > ptCaretPos.x ){
			return -1;
		}
		if( rcSel.right < ptCaretPos.x ){
			return 1;
		}
	}else{
		if( GetSelectionInfo().m_sSelect.GetFrom().y > ptCaretPos.y ){
			return -1;
		}
		if( GetSelectionInfo().m_sSelect.GetTo().y < ptCaretPos.y ){
			return 1;
		}
		if( GetSelectionInfo().m_sSelect.GetFrom().y == ptCaretPos.y ){
			if( m_bDragSource ){
				if( GetKeyState_Control() ){	/* Ctrl�L�[��������Ă����� */
					if( GetSelectionInfo().m_sSelect.GetFrom().x >= ptCaretPos.x ){
						return -1;
					}
				}else{
					if( GetSelectionInfo().m_sSelect.GetFrom().x > ptCaretPos.x ){
						return -1;
					}
				}
			}else
			if( GetSelectionInfo().m_sSelect.GetFrom().x > ptCaretPos.x ){
				return -1;
			}
		}
		if( GetSelectionInfo().m_sSelect.GetTo().y == ptCaretPos.y ){
			if( m_bDragSource ){
				if( GetKeyState_Control() ){	/* Ctrl�L�[��������Ă����� */
					if( GetSelectionInfo().m_sSelect.GetTo().x <= ptCaretPos.x ){
						return 1;
					}
				}else{
					if( GetSelectionInfo().m_sSelect.GetTo().x < ptCaretPos.x ){
						return 1;
					}
				}
			}else
			if( GetSelectionInfo().m_sSelect.GetTo().x <= ptCaretPos.x ){
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 (�e�X�g)
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
//2007.09.01 kobake ����
int CEditView::IsCurrentPositionSelectedTEST(
	const CLayoutPoint& ptCaretPos,      //�J�[�\���ʒu
	const CLayoutRange& sSelect //
) const
{
	if( !GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}

	if( PointCompare( ptCaretPos, sSelect.GetFrom() ) < 0 )return -1;
	if( PointCompare( ptCaretPos, sSelect.GetTo() ) >= 0 )return 1;

	return 0;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �N���b�v�{�[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
void CEditView::CopySelectedAllLines(
	const wchar_t*	pszQuote,		//!< �擪�ɕt������p��
	BOOL			bWithLineNumber	//!< �s�ԍ���t�^����
)
{
	RECT		rcSel;
	CNativeW	cmemBuf;

	if( !GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	/* ��`�͈͑I�𒆂� */
	if( GetSelectionInfo().IsBoxSelecting() ){
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().m_sSelect.GetTo()		// �͈͑I���I��
		);

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		GetSelectionInfo().DisableSelectArea( TRUE );

		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
		GetSelectionInfo().m_sSelect.SetFrom(CLayoutPoint(0,rcSel.top     )); // �͈͑I���J�n
		GetSelectionInfo().m_sSelect.SetTo  (CLayoutPoint(0,rcSel.bottom+1)); // �͈͑I���I��
	}
	else{
		CLayoutRange sSelectOld;
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelectionInfo().m_sSelect.GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelectionInfo().m_sSelect.GetTo().y  ));
		if( GetSelectionInfo().m_sSelect.GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		GetSelectionInfo().DisableSelectArea( TRUE );
		GetSelectionInfo().m_sSelect = sSelectOld;		//�͈͑I��
	}
	/* �ĕ`�� */
	//	::UpdateWindow();
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, false);
	// To Here 2007.09.09 Moca
	/* �I��͈͂��N���b�v�{�[�h�ɃR�s�[ */
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ�͏I������ */
	if( !GetSelectedData(
		&cmemBuf,
		FALSE,
		pszQuote, /* ���p�� */
		bWithLineNumber, /* �s�ԍ���t�^���� */
		GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy /* �܂�Ԃ��ʒu�ɉ��s�L�������� */
	) ){
		ErrorBeep();
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), false );
}

/*! �N���b�v�{�[�h����f�[�^���擾
	@date 2005/05/29 novice UNICODE TEXT �Ή�������ǉ�
	@date 2007.10.04 ryoji MSDEVLineSelect�Ή�������ǉ�
*/
bool CEditView::MyGetClipboardData( CNativeW& cmemBuf, bool* pbColmnSelect, bool* pbLineSelect /*= NULL*/ )
{
	if(pbColmnSelect)
		*pbColmnSelect = false;

	if(pbLineSelect)
		*pbLineSelect = false;

	if(!CClipboard::HasValidData())
		return false;
	
	CClipboard cClipboard(GetHwnd());
	if(!cClipboard)
		return false;

	return cClipboard.GetText(&cmemBuf,pbColmnSelect,pbLineSelect);
}

/* �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	@date 2004.02.17 Moca �G���[�`�F�b�N����悤��
 */
bool CEditView::MySetClipboardData( const ACHAR* pszText, int nTextLen, bool bColmnSelect, bool bLineSelect /*= false*/ )
{
	//WCHAR�ɕϊ�
	vector<wchar_t> buf;
	mbstowcs_vector(pszText,nTextLen,&buf);
	return MySetClipboardData(&buf[0],buf.size()-1,bColmnSelect,bLineSelect);
}
bool CEditView::MySetClipboardData( const WCHAR* pszText, int nTextLen, bool bColmnSelect, bool bLineSelect /*= false*/ )
{
	/* Windows�N���b�v�{�[�h�ɃR�s�[ */
	CClipboard cClipboard(GetHwnd());
	if(!cClipboard){
		return false;
	}
	cClipboard.Empty();
	return cClipboard.SetText(pszText,nTextLen,bColmnSelect,bLineSelect);
}






// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �A���_�[���C��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �J�[�\���s�A���_�[���C����ON */
void CEditView::CaretUnderLineON( bool bDraw )
{
	BOOL bCursorVLine = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp;

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

	if( GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( bCursorVLine ){
		// �J�[�\���ʒu�c���B-1���ăL�����b�g�̍��ɗ���悤�ɁB
		m_nOldCursorLineX = GetTextArea().GetAreaLeft() + (Int)(GetCaret().GetCaretLayoutPos().GetX2() - GetTextArea().GetViewLeftCol())
			* (m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace + GetTextMetrics().GetHankakuWidth() ) - 1;
		if( -1 == m_nOldCursorLineX ){
			m_nOldCursorLineX = -2;
		}
	}else{
		m_nOldCursorLineX = -1;
	}

	if( bDraw
	 && m_bDrawSWITCH
	 && GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX
	 && m_nOldCursorLineX <= GetTextArea().GetAreaRight()
	 && m_bDoing_UndoRedo == FALSE
	){
		// �J�[�\���ʒu�c���̕`��
		// �A���_�[���C���Əc���̌�_�ŁA��������ɂȂ�悤�ɐ�ɏc���������B
		HDC		hdc;
		HPEN	hPen, hPenOld;
		int     nROP_Old = 0;
		hdc = ::GetDC( GetHwnd() );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, m_nOldCursorLineX, GetTextArea().GetAreaTop(), NULL );
		::LineTo(   hdc, m_nOldCursorLineX, GetTextArea().GetAreaBottom() );
		// �u�����v�̂Ƃ���2dot�̐��ɂ���B���̍ۃJ�[�\���Ɋ|����Ȃ��悤�ɍ����𑾂�����
		if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bFatFont &&
			GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX - 1 ){
			::MoveToEx( hdc, m_nOldCursorLineX - 1, GetTextArea().GetAreaTop(), NULL );
			::LineTo(   hdc, m_nOldCursorLineX - 1, GetTextArea().GetAreaBottom() );
		}
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( GetHwnd(), hdc );
		hdc= NULL;
	}
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		m_nOldUnderLineY = GetTextArea().GetAreaTop() + (Int)(GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine())
			 * (m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace + GetTextMetrics().GetHankakuHeight()) + GetTextMetrics().GetHankakuHeight();
		if( -1 == m_nOldUnderLineY ){
			m_nOldUnderLineY = -2;
		}
	}else{
		m_nOldUnderLineY = -1;
	}
	// To Here 2007.09.09 Moca

	if( bDraw
	 && GetDrawSwitch()
	 && m_nOldUnderLineY >=GetTextArea().GetAreaTop()
	 && m_bDoing_UndoRedo == FALSE	/* �A���h�D�E���h�D�̎��s���� */
	){
//		MYTRACE_A( "���J�[�\���s�A���_�[���C���̕`��\n" );
		/* ���J�[�\���s�A���_�[���C���̕`�� */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( GetHwnd() );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			GetTextArea().GetAreaLeft(),
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			GetTextArea().GetAreaRight(),
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( GetHwnd(), hdc );
		hdc= NULL;
	}
}

/* �J�[�\���s�A���_�[���C����OFF */
void CEditView::CaretUnderLineOFF( bool bDraw )
{
	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp &&
			FALSE == m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp ){
		return;
	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && GetDrawSwitch()
		 && m_nOldUnderLineY >=GetTextArea().GetAreaTop()
		 && !m_bDoing_UndoRedo	/* �A���h�D�E���h�D�̎��s���� */
		){
			// -- -- �J�[�\���s�A���_�[���C���̏����i�������j -- -- //

			GetCaret().m_cUnderLine.Lock();

			PAINTSTRUCT ps;
			ps.rcPaint.left = GetTextArea().GetAreaLeft();
			ps.rcPaint.right = GetTextArea().GetAreaRight();
			ps.rcPaint.top = m_nOldUnderLineY;
			ps.rcPaint.bottom = m_nOldUnderLineY + 1; // 2007.09.09 Moca +1 ����悤��

			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			CLayoutRange sSelectBackup = GetSelectionInfo().m_sSelect;
			GetSelectionInfo().m_sSelect.Clear(-1);

			// �`��
			HDC hdc = this->GetDC();
			// �\�Ȃ�݊�BMP����R�s�[���čč��
			OnPaint( hdc, &ps, TRUE );
			this->ReleaseDC( hdc );

			//	�I�����𕜌�
			GetSelectionInfo().m_sSelect = sSelectBackup;
			GetCaret().m_cUnderLine.UnLock();
		}
		m_nOldUnderLineY = -1;
	}
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �J�[�\���ʒu�c��
	if( -1 != m_nOldCursorLineX ){
		if( bDraw
		 && m_bDrawSWITCH
		 && GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX
		 && m_nOldCursorLineX <= GetTextArea().GetAreaRight()
		 && m_bDoing_UndoRedo == FALSE
		){
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nOldCursorLineX;
			ps.rcPaint.right = m_nOldCursorLineX + 1;
			ps.rcPaint.top = GetTextArea().GetAreaTop();
			ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
			if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bFatFont ){
				ps.rcPaint.left += -1;
			}
			HDC hdc = ::GetDC( GetHwnd() );
			GetCaret().m_cUnderLine.Lock();
			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			CLayoutRange sSelectBackup = this->GetSelectionInfo().m_sSelect;
			this->GetSelectionInfo().m_sSelect.Clear(-1);
			// �\�Ȃ�݊�BMP����R�s�[���čč��
			OnPaint( hdc, &ps, TRUE );
			//	�I�����𕜌�
			this->GetSelectionInfo().m_sSelect = sSelectBackup;
			GetCaret().m_cUnderLine.UnLock();
			ReleaseDC( hdc );
		}
		m_nOldCursorLineX = -1;
	};
	// To Here 2007.09.09 Moca
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ��ԕ\��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������

	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta ���̂�CEditWnd�ֈړ�
*/
void CEditView::SendStatusMessage( const TCHAR* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �ҏW���[�h                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�}�����[�h�擾

	@date 2005.10.02 genta �Ǘ����@�ύX�̂��ߊ֐���
*/
bool CEditView::IsInsMode(void) const
{
	return m_pcEditDoc->m_cDocEditor.IsInsMode();
}

void CEditView::SetInsMode(bool mode)
{
	m_pcEditDoc->m_cDocEditor.SetInsMode( mode );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CEditView::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// -- -- �� InitAllView�ł���Ă����� -- -- //
	pcDoc->m_nCommandExecNum=0;

	m_cHistory->Flush();

	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	GetSelectionInfo().DisableSelectArea( FALSE );

	OnChangeSetting();
	GetCaret().MoveCursor( CLayoutPoint(0, 0), TRUE );
	GetCaret().m_nCaretPosX_Prev = CLayoutInt(0);
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	2004.05.13 Moca ���s�R�[�h�̐ݒ�����炱���Ɉړ�
	m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//!	���݂̃J�[�\���s�ʒu�𗚗��ɓo�^����
void CEditView::AddCurrentLineToHistory( void )
{
	CLogicPoint ptPos;
	//int PosX, PosY;	//	�����ʒu�i���s�P�ʂ̌v�Z�j

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptPos );

	CMarkMgr::CMark m( ptPos );
	m_cHistory->Add( m );
}


//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
bool  CEditView::ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin)
{
	CNativeW	cmemCurText;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
		if( m_bInMenuLoop == FALSE	&&	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
			0 != m_dwTipTimer			/* ����Tip��\�����Ă��Ȃ� */
		){
			cmemCurText.SetString( pszHelp );

			/* ���Ɍ����ς݂� */
			if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* �Y������L�[���Ȃ����� */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* �������s */
				if(!KeySearchCore(&m_cTipWnd.m_cKey))	// 2006.04.10 fon
					return FALSE;
			}
			m_dwTipTimer = 0;	/* ����Tip��\�����Ă��� */

		// 2001/06/19 Start by asa-o: ����Tip�̕\���ʒu����
			// ����Tip�̃T�C�Y���擾
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta �}���`���j�^�Ή�
			::GetMonitorWorkRect( m_cTipWnd.GetHwnd(), &rcDesktop );

			// �E�ɓ���
			if(prcHokanWin->right + rcTipWin.right < rcDesktop.right){
				// ���̂܂�
			}else
			// ���ɓ���
			if(rcDesktop.left < prcHokanWin->left - rcTipWin.right ){
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}else
			// �ǂ�����X�y�[�X�������Ƃ��L���ق��ɕ\��
			if(rcDesktop.right - prcHokanWin->right > prcHokanWin->left ){
				// �E�ɕ\�� ���̂܂�
			}else{
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			/* ����Tip��\�� */
			m_cTipWnd.Show( po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End