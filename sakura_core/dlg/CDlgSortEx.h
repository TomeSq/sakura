/*
	Copyright (C) 2016, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "dlg/CDialog.h"

#ifndef SAKURA_CDLGSORTEX_H_
#define SAKURA_CDLGSORTEX_H_

class CDlgSortEx : public CDialog
{
public:
	CDlgSortEx();
	int DoModal(HINSTANCE, HWND, LPARAM);

	bool m_bOrderAsc; // false == Desc

	enum ESortExMode{
		ESortExMode_String,
		ESortExMode_API1,
		ESortExMode_API2,
		ESortExMode_API3,
		ESortExMode_Dec,
		ESortExMode_Hex,
	};
	ESortExMode m_eMode;

	bool m_bCaseIgnore;
	bool m_bHiraKataIgnore;
	bool m_bWidthIgnore;
	bool m_bSpaceIgnore;
	bool m_bStringSort;
	bool m_bNumSort;

protected:
	int GetData();
	void SetData();
	void SetEnableIgnores(BOOL bEnable);
	void SetEnableStirngCompareOption(BOOL bEnable);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);
};


#endif /* SAKURA_CDLGSORTEX_H_ */


