//	$Id$
/************************************************************************

	CMacro.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CMacro;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include "debug.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro();
	~CMacro();

	/*
	||  Attributes & Operations
	*/
	static char* CMacro::GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* 機能ID→関数名，機能名日本語 */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* 関数名→機能ID，機能名日本語 */
	static BOOL CanFuncIsKeyMacro( int );	/* キーマクロに記録可能な機能かどうかを調べる */
	

protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */

/*[EOF]*/

