// $Id$
//
/*!	@file
	現在行のマークを管理する

	@author genta
	@version $Revision$

	Copyright (C) 1998-2000, genta
*/

#include "global.h"
#include "CMarkMgr.h"

//-----------------------------------
// CMarkMgr
//-----------------------------------
/*!
	@brief 保管する最大件数を指定する。
	
	現在より小さい値を設定したときは余分な要素は削除される。
	
	@param max 設定する最大件数
*/
void CMarkMgr::SetMax(int max)
{
	maxitem = max;
	Expire();	//	指定した数に要素を減らす
}

/*!
	@brief 現在位置の要素が有効かどうかの判定
	
	@retval true	有効
	@retval false	無効
*/
bool CMarkMgr::CheckCurrent(void) const
{
	if( curpos < Count() )
		return dat[ curpos ].IsValid();

	return false;
}

/*!
	@brief 現在位置の前に有効な要素があるかどうかを調べる

	@retval true	有る
	@retval false	無い
*/
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief 現在位置の後に有効な要素があるかどうかを調べる

	@retval true	有る
	@retval false	無い
*/
bool CMarkMgr::CheckNext(void) const
{
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief 現在位置を前の有効な位置まで進める
	
	@retval true	正常終了。現在位置は1つ前の有効な要素に移動した。
	@retval false	有効な要素が見つからなかった。現在位置は移動していない。
*/
bool CMarkMgr::PrevValid(void)
{
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() ){
			curpos = i;
			return true;
		}
	}
	return false;
}
/*!
	@brief 現在位置を後の有効な位置まで進める
	
	@retval true	正常終了。現在位置は1つ後の有効な要素に移動した。
	@retval false	有効な要素が見つからなかった。現在位置は移動していない。
*/
bool CMarkMgr::NextValid(void)
{
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() ){
			curpos = i;
			return true;
		}
	}
	return false;
}

//	From Here Apr. 1, 2001 genta 
/*!
	現在のデータを全て消去し、現在位置のポインタをリセットする。
	
	@par history
	Apr. 1, 2001 genta 新規追加
*/
void CMarkMgr::Flush(void)
{
	dat.erase( dat.begin(), dat.end());
	curpos = 0;
}
//	To Here

//-----------------------------------
// CAutoMarkMgr
//-----------------------------------

/*!
	現在位置に要素を追加する．現在位置より後ろは全て削除する。
	要素番号が大きい方が新しいデータ。
	
	@param m 追加する要素
*/
void CAutoMarkMgr::Add(const CMark& m)
{
	//	現在位置が途中の時
	if( curpos < dat.size()){
		//	現在位置まで要素を削除
		dat.erase( dat.begin() + curpos, dat.end() );
	}
	
	//	要素の追加
	dat.push_back(m);
	++curpos;
	
	//	規定数を超えてしまうときの対応
	Expire();
}

/*!
	要素数が最大値を超えている場合に要素数が範囲内に収まるよう、
	古い方(番号の若い方)から削除する。
*/
void CAutoMarkMgr::Expire(void)
{
	int range = dat.size() - GetMax();
	
	if( range <= 0 )	return;
	
	//	最大値を超えている場合
	dat.erase( dat.begin(), dat.begin() + range );
	curpos -= range;
	if( curpos < 0 )
		curpos = 0;
}
