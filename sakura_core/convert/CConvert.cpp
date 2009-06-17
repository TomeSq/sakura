#include "stdafx.h"
#include "CConvert.h"
#include "func/Funccode.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "charset/CCodeMediator.h"
#include "charset/CShiftJis.h"
#include "charset/CJis.h"
#include "charset/CEuc.h"
#include "charset/CUnicodeBe.h"
#include "charset/CUtf8.h"
#include "charset/CUtf7.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkanaToHankana.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankanaToZenkana.h"
#include "CConvert_HankanaToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkana.h"
#include "CConvert_Trim.h"

#include "window/CEditWnd.h"

/* 機能種別によるバッファの変換 */
void CConvertMediator::ConvMemory( CNativeW* pCMemory, EFunctionCode nFuncCode, int nTabWidth )
{
	// コード変換はできるだけANSI版のsakuraと互換の結果が得られるように実装する	// 2009.03.26 ryoji
	// xxx2SJIS:
	//   1. バッファの内容がANSI版相当になるよう Unicode→SJIS 変換する
	//   2. xxx→SJIS 変換後にバッファ内容をUNICODE版相当に戻す（SJIS→Unicode）のと等価な結果を得るために xxx→Unicode 変換する
	// SJIS2xxx:
	//   1. バッファ内容をANSI版相当に変換（Unicode→SJIS）後に SJIS→xxx 変換するのと等価な結果を得るために Unicode→xxx 変換する
	//   2. バッファ内容をUNICODE版相当に戻すために SJIS→Unicode 変換する

	switch( nFuncCode ){
	//コード変換(xxx2SJIS)
	case F_CODECNV_AUTO2SJIS:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
		CShiftJis::UnicodeToSJIS(pCMemory->_GetMemory());
		break;
	//コード変換(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:		CJis::UnicodeToJIS(pCMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2EUC:		CEuc::UnicodeToEUC(pCMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2UTF8:		CUtf8::UnicodeToUTF8(pCMemory->_GetMemory());		break;
	case F_CODECNV_SJIS2UTF7:		CUtf7::UnicodeToUTF7(pCMemory->_GetMemory());		break;
	}

	if( nFuncCode == F_CODECNV_AUTO2SJIS ){
		ECodeType ecode;
		CCodeMediator ccode( CEditWnd::Instance()->GetDocument() );
		ecode = ccode.CheckKanjiCode(
			reinterpret_cast<const char*>(pCMemory->_GetMemory()->GetRawPtr()),
			pCMemory->_GetMemory()->GetRawLength() );
		switch( ecode ){
		case CODE_JIS:			nFuncCode = F_CODECNV_EMAIL;			break;
		case CODE_EUC:			nFuncCode = F_CODECNV_EUC2SJIS;			break;
		case CODE_UNICODE:		nFuncCode = F_CODECNV_UNICODE2SJIS;		break;
		case CODE_UNICODEBE:	nFuncCode = F_CODECNV_UNICODEBE2SJIS;	break;
		case CODE_UTF8:			nFuncCode = F_CODECNV_UTF82SJIS;		break;
		case CODE_UTF7:			nFuncCode = F_CODECNV_UTF72SJIS;		break;
		}
	}

	switch( nFuncCode ){
	//文字種変換、整形
	case F_TOLOWER:					CConvert_ToLower().CallConvert(pCMemory);			break;	// 小文字
	case F_TOUPPER:					CConvert_ToUpper().CallConvert(pCMemory);			break;	// 大文字
	case F_TOHANKAKU:				CConvert_ToHankaku().CallConvert(pCMemory);			break;	// 全角→半角
	case F_TOHANKATA:				CConvert_ZenkanaToHankana().CallConvert(pCMemory);	break;	// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					CConvert_HaneisuToZeneisu().CallConvert(pCMemory);	break;	// 半角英数→全角英数
	case F_TOHANEI:					CConvert_ZeneisuToHaneisu().CallConvert(pCMemory);	break;	// 全角英数→半角英数
	case F_TOZENKAKUKATA:			CConvert_ToZenkana().CallConvert(pCMemory);			break;	// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			CConvert_ToZenhira().CallConvert(pCMemory);			break;	// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		CConvert_HankanaToZenkana().CallConvert(pCMemory);	break;	// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		CConvert_HankanaToZenhira().CallConvert(pCMemory);	break;	// 半角カタカナ→全角ひらがな
	//文字種変換、整形
	case F_TABTOSPACE:				CConvert_TabToSpace(nTabWidth).CallConvert(pCMemory);break;	// TAB→空白
	case F_SPACETOTAB:				CConvert_SpaceToTab(nTabWidth).CallConvert(pCMemory);break;	// 空白→TAB
	case F_LTRIM:					CConvert_Trim(true).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	case F_RTRIM:					CConvert_Trim(false).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	//コード変換(xxx2SJIS)
	case F_CODECNV_EMAIL:			CJis::JISToUnicode(pCMemory->_GetMemory(), true);	break;
	case F_CODECNV_EUC2SJIS:		CEuc::EUCToUnicode(pCMemory->_GetMemory());			break;
	case F_CODECNV_UNICODE2SJIS:	/* 無変換 */										break;
	case F_CODECNV_UNICODEBE2SJIS:	CUnicodeBe::UnicodeBEToUnicode(pCMemory->_GetMemory());	break;
	case F_CODECNV_UTF82SJIS:		CUtf8::UTF8ToUnicode(pCMemory->_GetMemory());		break;
	case F_CODECNV_UTF72SJIS:		CUtf7::UTF7ToUnicode(pCMemory->_GetMemory());		break;
	//コード変換(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_SJIS2UTF7:
		CShiftJis::SJISToUnicode(pCMemory->_GetMemory());
		break;
	}

	return;
}


