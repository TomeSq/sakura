/*!	@file
@brief CViewCommanderクラスのコマンド(マクロ系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, Moca
	Copyright (C) 2005, novice
	Copyright (C) 2006, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "macro/CSMacroMgr.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgOpenFile.h"
#include "CEditApp.h"
#include "recent/CRecentCurDir.h"
#include "util/module.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"


/* キーマクロの記録開始／終了 */
void CViewCommander::Command_RECKEYMACRO( void )
{
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro ){									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* キーボードマクロを記録中のウィンドウ */
		//@@@ 2002.1.24 YAZAKI キーマクロをマクロ用フォルダに「RecKey.mac」という名で保存
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca 記録用キーマクロのフルパスをCShareData経由で取得
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet );
			return;
		}else{
			_tcscpy( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		int nSaveResult = m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		if( GetEditWindow()->m_bExecKeyMacro ){
			// 2014.12.24 キーマクロ実行中は不可
			return;
		}
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		/* キーマクロのバッファをクリアする */
		//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	/* 親ウィンドウのタイトルを更新 */
	GetEditWindow()->UpdateCaption();

	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}



/* キーマクロの保存 */
void CViewCommander::Command_SAVEKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	//	Jun. 16, 2002 genta
	if( !m_pcSMacroMgr->IsSaveOk() ){
		//	保存不可
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26) );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	_tcscpy( szPath, _T("") );
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	wcscat( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, L"\\" );

	/* キーボードマクロの保存 */
	//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
	//@@@ 2002.1.24 YAZAKI
	if ( !m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath );
	}
	return;
}



/*! キーマクロの読み込み
	@date 2005.02.20 novice デフォルトの拡張子変更
 */
void CViewCommander::Command_LOADKEYMACRO( void )
{
	if( GetEditWindow()->m_bExecKeyMacro ){
		// 2014.12.24 キーマクロ実行中は不可
		return;
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	_tcscpy( szPath, _T("") );
	pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice デフォルトの拡張子変更
// 2005/07/13 novice 多様なマクロをサポートしているのでデフォルトは全て表示にする
		_T("*.*"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* キーボードマクロの読み込み */
	//@@@ 2002.1.24 YAZAKI 読み込みといいつつも、ファイル名をコピーするだけ。実行直前に読み込む
	_tcscpy(GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro( G_AppInstance(), m_pCommanderView->GetHwnd(), szPath );
	return;
}



/* キーマクロの実行 */
void CViewCommander::Command_EXECKEYMACRO( void )
{
	if( GetEditWindow()->m_bExecKeyMacro ){
		// 2014.12.24 マクロ内メニュー表示で再入の可能性。再入不可
		return;
	}
	GetEditWindow()->m_bExecKeyMacro = true;
	//@@@ 2002.1.24 YAZAKI 記録中は終了してから実行
	if (GetDllShareData().m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	/* キーボードマクロの実行 */
	//@@@ 2002.1.24 YAZAKI
	if ( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	ファイルが保存されていたら
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		BOOL bLoadResult = m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flagsオプション追加
			m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
		}
	}
	GetEditWindow()->m_bExecKeyMacro = false;
	return;
}



/*! 名前を指定してマクロ実行
	@param pszPath	マクロのファイルパス、またはマクロのコード。
	@param pszType	種別。NULLの場合ファイル指定、それ以外の場合は言語の拡張子を指定

	@date 2008.10.23 syat 新規作成
	@date 2008.12.21 syat 引数「種別」を追加
 */
void CViewCommander::Command_EXECEXTMACRO( const WCHAR* pszPathW, const WCHAR* pszTypeW, CMacro* pMacroParam, bool bRecKeyMacro )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//ファイル選択ダイアログの初期フォルダ
	const TCHAR*	pszFolder;					//マクロフォルダ
	const TCHAR*	pszPath = NULL;				//第1引数をTCHAR*に変換した文字列
	const TCHAR*	pszType = NULL;				//第2引数をTCHAR*に変換した文字列
	HWND			hwndRecordingKeyMacro = NULL;
	std::tstring	tstrPathTemp;
	std::tstring	tstrTypeTemp;
	std::wstring	wstrPath;
	std::wstring	wstrType;
	const WCHAR*	pszTypeMacro = NULL;

	if ( pszPathW != NULL ) {
		//to_tchar()で取得した文字列はdeleteしないこと。
		tstrPathTemp = to_tchar( pszPathW );
		pszPath = tstrPathTemp.c_str();
		tstrTypeTemp = to_tchar( pszTypeW );
		pszType = tstrTypeTemp.c_str();
		wstrPath = pszPathW;
		pszTypeMacro = pszTypeW;

	} else {
		// ファイルが指定されていない場合、ダイアログを表示する
		_tcscpy( szPath, _T("") );
		pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
		}
		/* ファイルオープンダイアログの初期化 */
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
			if( GetSaveResultParam() ){
				GetMacroResultVal().SetIntParam( -1 );
			}
			return;
		}
		pszPath = szPath;
		pszType = NULL;
		wstrPath = to_wchar(szPath);
	}

	//キーマクロ記録中
	CMacro cMacroParent(F_0);
	cMacroParent.Move(GetEditWindow()->m_cRecMacroParam);
	CMacro cMacroParamOld(F_0);
	cMacroParamOld.Move(GetEditWindow()->m_cExecMacroParam);
	if( pMacroParam ){
		GetEditWindow()->m_cExecMacroParam.Move(*pMacroParam);
	}
	if( bRecKeyMacro ){
		//キーマクロの記録を一時停止する
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
	}

	//古い一時マクロの退避
	CMacroManagerBase* oldMacro = m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_MACROERR1), pszPath );
		if( GetSaveResultParam() ){
			GetMacroResultVal().SetIntParam( -2 );
		}
	}
	else {
		if( GetSaveResultParam() ){
			GetMacroResultVal().SetIntParam( 0 );
		}
		m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// 終わったら開放
	m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	if( bLoadResult ){
		if( GetSaveResultParam() ){
			GetMacroResultParam().AddStringParam( pszPath );
			GetMacroResultParam().AddStringParam( L"" ); //ext
			// マクロ自身のパラメータをコピー
			GetMacroResultParam().AddParamCopyFromCMacro( &GetEditWindow()->m_cRecMacroParam );
			// GetMacroResultVal().SetIntParam( 0 ); // ここでは設定しない
		}
	}

	// キーマクロ記録中だった場合は再開する
	if ( hwndRecordingKeyMacro != NULL ) {
		// キーマクロの記録を登録
		LPARAM lparams[] = {(LPARAM)wstrPath.c_str(), (LPARAM)pszTypeMacro, (LPARAM)&GetEditWindow()->m_cRecMacroParam, 0};
		m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, lparams, m_pCommanderView );

		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
	}
	GetEditWindow()->m_cRecMacroParam.Move(cMacroParent);
	GetEditWindow()->m_cExecMacroParam.Move(cMacroParamOld);
	return;
}



/*! 外部コマンド実行ダイアログ表示
	@date 2002.02.02 YAZAKI.
*/
void CViewCommander::Command_EXECCOMMAND_DIALOG( EFunctionFlags flags )
{
	CDlgExec cDlgExec;

	/* モードレスダイアログの表示 */
	if( !cDlgExec.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		if( GetSaveResultParam() ){
			GetMacroResultVal().SetIntParam( 0 );
		}
		return;
	}

	m_pCommanderView->AddToCmdArr( cDlgExec.m_szCommand );
	std::wstring strCmd = to_wchar(cDlgExec.m_szCommand);
	std::wstring strDir = to_wchar(cDlgExec.m_szCurDir);
	const WCHAR* cmd_string = strCmd.c_str();
	const WCHAR* curDir = strDir.c_str();
	const WCHAR* pszDir = curDir;
	if( curDir[0] == L'\0' ){
		pszDir = NULL;
	}else{
		CRecentCurDir cRecentCurDir;
		cRecentCurDir.AppendItem( cDlgExec.m_szCurDir );
		cRecentCurDir.Terminate();
	}

	int nOpt = GetDllShareData().m_nExecFlgOpt;
	//HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	//	外部コマンド実行コマンドの発行
	HandleCommand( static_cast<EFunctionCode>(F_EXECMD | flags), true, (LPARAM)cmd_string, (LPARAM)(nOpt), (LPARAM)pszDir, 0);	//	外部コマンド実行コマンドの発行
	if( GetSaveResultParam() ){
		GetMacroResultParam().AddStringParam( strCmd.c_str() );
		GetMacroResultParam().AddIntParam( nOpt );
		GetMacroResultParam().AddStringParam( curDir );
		GetMacroResultVal().SetIntParam( 1 );
	}
}



//外部コマンド実行
//	Sept. 20, 2000 JEPRO  名称CMMANDをCOMMANDに変更
//	Oct. 9, 2001   genta  マクロ対応のため引数追加
//  2002.2.2       YAZAKI ダイアログ呼び出し部とコマンド実行部を分離
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CViewCommander::Command_EXECCOMMAND( LPCWSTR cmd_string, const int nFlgOpt, LPCWSTR pszCurDir)	//	2006.12.03 maru 引数の拡張
{
	//	From Here Aug. 21, 2001 genta
	//	パラメータ置換 (超暫定)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	CSakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);

	// 子プロセスの標準出力をリダイレクトする
	std::tstring buf2 = to_tchar(buf);
	std::tstring buf3;
	if( pszCurDir ){
		buf3 = to_tchar(pszCurDir);
	}
	m_pCommanderView->ExecCmd( buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : NULL) );
	//	To Here Aug. 21, 2001 genta
	return;
}
