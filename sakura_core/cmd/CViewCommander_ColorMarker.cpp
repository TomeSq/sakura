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

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"
#include "dlg/CDlgColorMarker.h"
#include "dlg/CDlgOpenFile.h"
#include "env/CSakuraEnvironment.h"


/*! �J���[�}�[�J�[�ݒ�
	�I��͈͓����}�[�J�[�ł���
	nIndex:0 �Ō�Ɏg������
	nIndex:1-10 �Z�b�g1-10
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SET(int nIndex)
{
	CMarkerItem item;
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	if( nIndex == 0 ){
		item = markerSet.m_ColorItemLast;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		item = markerSet.m_ColorItems[nIndex - 1];
		markerSet.m_ColorItemLast = item;
	}else{
		ErrorBeep();
		return FALSE;
	}
	Command_SETCOLORMARKER(item);
	return TRUE;
}

/*! �J���[�}�[�J�[�Z�b�g������(�}�N��)
	�I��͈͓����}�[�N����
*/
BOOL CViewCommander::Command_SETCOLORMARKER_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		Command_SETCOLORMARKER(item);
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}

/*! �J���[�}�[�J�[�Z�b�g������(Sub)
	�I��͈͓����}�[�N����
*/
void CViewCommander::Command_SETCOLORMARKER(const CMarkerItem& marker)
{
	CMarkerItem item = marker;
	bool bLineMode = ((item.m_nGyouLine & 0x03) != 0);
	if( bLineMode ){
		item.m_nBegin = 0;
		item.m_nEnd = 0;
		// ���I���ł��s���[�h�͋���
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			// �J�[�\���s��o�^
			Command_SETCOLORMARKER_ITEM(item, GetCaret().GetCaretLogicPos().y, false);
			GetDocument()->m_pcEditWnd->Views_Redraw();
			return;
		}
	}else{
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			return; // ���I��
		}
	}
	CLayoutRange rgSelect = GetSelect();
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		CLayoutRect rcSelect;
		TwoPointToRect(&rcSelect, rgSelect.GetFrom(), rgSelect.GetTo());
		CLogicYInt nLinePrev = CLogicYInt(-1);
		for( CLayoutYInt nLineNum = rcSelect.top; nLineNum <= rcSelect.bottom; nLineNum++ ){
			CLogicPoint ptLogic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.left, nLineNum), &ptLogic);
			if( !bLineMode ){
				item.m_nBegin = ptLogic.x;
			}
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.right, nLineNum), &ptLogic);
			if( !bLineMode ){
				item.m_nEnd = ptLogic.x;
			}
			if( item.m_nBegin != item.m_nEnd ){
				if( nLinePrev == ptLogic.y ){
					continue; // �܂�Ԃ��s�̓������W�b�N�s�͏d�������Ȃ�
				}
				nLinePrev = ptLogic.y;
				Command_SETCOLORMARKER_ITEM(item, ptLogic.y, false);
			}
		}
	}else{
		CLayoutRect rc;
		TwoPointToRect(&rc, rgSelect.GetFrom(), rgSelect.GetTo());
		CLayoutRange rgFixedSelect(CLayoutPoint(rc.left, rc.top), CLayoutPoint(rc.right, rc.bottom));
		CLogicRange rgLogicRange;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(rgFixedSelect, &rgLogicRange);
		const CLogicYInt nBegin = rgLogicRange.GetFrom().y;
		const CLogicYInt nEnd = rgLogicRange.GetTo().y;
		CLogicYInt nEnd2 = nEnd;
		if( rgLogicRange.GetFrom().y != rgLogicRange.GetTo().y
			 && rgLogicRange.GetTo().x == CLogicXInt(0) ){
			nEnd2--; // ���̍s���܂őI���������ꍇ�́A�O�̍s�܂łɂ���
		}
		for( CLogicYInt nLineNum = nBegin; nLineNum <= nEnd2; nLineNum++ ){
			if( !bLineMode ){
				CLogicXInt nFromX = CLogicXInt(0);
				CLogicXInt nToX = CLogicXInt(-1);
				if( nLineNum == nBegin ){
					nFromX = rgLogicRange.GetFrom().x;
				}
				if( nLineNum == nEnd ){
					nToX = rgLogicRange.GetTo().x;
				}
				if( CLogicXInt(-1) == nToX ){
					const CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
					if( docLine ){
						nToX = docLine->GetLengthWithEOL();
					}else{
						nToX = 0;
					}
				}
				if( nFromX == nToX ){
					// �I��͈͂�EOL���E�ȂǂŔ͈͂��Ȃ��ꍇ�͖���
					continue;
				}
				item.m_nBegin = nFromX;
				item.m_nEnd = nToX;
			}
			Command_SETCOLORMARKER_ITEM(item, nLineNum, false);
		}
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

BOOL CViewCommander::Command_SETCOLORMARKER_DIRECT(LPCWSTR pszMarker)
{
	CMarkerItem item;
	CLogicYInt nLineNum;
	if( CColorMarkerVisitor().StrToMarkerItem(1, pszMarker, item, nLineNum) ){
		return Command_SETCOLORMARKER_ITEM(item, nLineNum, true);
	}
	ErrorBeep();
	return FALSE;
}

/*! �J���[�}�[�J�[�Z�b�g(Sub)
	�w�肳�ꂽ�ʒu���w��F�Ń}�[�N����
*/
BOOL CViewCommander::Command_SETCOLORMARKER_ITEM(const CMarkerItem& item, CLogicYInt nLineNum, bool bDirect)
{
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
	if( docLine ){
		if( CColorMarkerVisitor().AddColorMarker(docLine, item) ){
			if( bDirect ){
				GetDocument()->m_pcEditWnd->Views_Redraw();
			}
			return TRUE;
		}
	}
	ErrorBeep();
	return FALSE;
}

/*! �J���[�}�[�J�[�����ݒ�(�Z�b�gn)
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SEARCH_SET(int nIndex)
{
	CMarkerItem item;
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	if( nIndex == 0 ){
		item = markerSet.m_ColorItemLast;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		item = markerSet.m_ColorItems[nIndex - 1];
		markerSet.m_ColorItemLast = item;
	}else{
		ErrorBeep();
		return FALSE;
	}
	Command_SETCOLORMARKER_SEARCH(item);
	return TRUE;
}

/*! �J���[�}�[�J�[�����ݒ�(�}�[�J�[)
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SEARCH_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		Command_SETCOLORMARKER_SEARCH(item);
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}

/*! �J���[�}�[�J�[�����ݒ�
*/
void CViewCommander::Command_SETCOLORMARKER_SEARCH(CMarkerItem& marker)
{
	// m_sSearchOption�I���̂��߂̐�ɓK�p
	if( !m_pCommanderView->ChangeCurRegexp() ){
		return;
	}
	const bool bLineMode = ((marker.m_nGyouLine & 0x03) != 0);

	if( !bLineMode ){
		// �����}�[�N�ƃ}�[�J�[���_�u�邽�߃I�t�ɂ���
		m_pCommanderView->m_bCurSrchKeyMark = false;
	}

	bool bDisplayUpdate = false;

	CDocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;
	CLogicRange cSelectLogic;
	int nRet = CSearchAgent(&rDocLineMgr).SearchWord(
		CLogicPoint(CLogicXInt(0), CLogicYInt(0)), SEARCH_FORWARD,
		&cSelectLogic, m_pCommanderView->m_sSearchPattern
	);

	while( nRet != 0 ){
		CLogicPoint ptLogicCursor = cSelectLogic.GetTo();
		// ���K�\����0�����}�b�`�̏ꍇ1�����ɂ���
		if( cSelectLogic.GetFrom() == cSelectLogic.GetTo() ){
			const CDocLine* docLine = rDocLineMgr.GetLine(cSelectLogic.GetTo().y);
			CLogicXInt nPosAdd = CLogicXInt(0);
			if( docLine ){
				const wchar_t* pLine = docLine->GetPtr();
				nPosAdd = CNativeW::GetSizeOfChar(pLine, docLine->GetLengthWithEOL(), cSelectLogic.GetTo().x);
				if( nPosAdd < CLogicXInt(1) ){
					nPosAdd = CLogicXInt(1);
				}
			}
			ptLogicCursor.x += nPosAdd;
		}

		CDocLine* docLine = rDocLineMgr.GetLine(cSelectLogic.GetFrom().y);
		if( docLine ){
			CMarkerItem item = marker;
			if( bLineMode ){
				item.m_nBegin = CLogicXInt(0);
				item.m_nEnd = CLogicXInt(0);
			}else{
				item.m_nBegin = cSelectLogic.GetFrom().x;
				item.m_nEnd = ptLogicCursor.x;
			}
			CColorMarkerVisitor().AddColorMarker(docLine, item);
			if( bLineMode ){
				ptLogicCursor.y++;
				ptLogicCursor.x = CLogicXInt(0);
			}
		}
		
		nRet = CSearchAgent(&rDocLineMgr).SearchWord(
			ptLogicCursor, SEARCH_FORWARD,
			&cSelectLogic, m_pCommanderView->m_sSearchPattern
		);
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

static void CViewCommander_ColorMarkerDelItemPos(CDocLine* docLine, CLogicXInt nFromX, CLogicXInt nToX)
{
	const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
	std::vector<CMarkerItem> vecIns; // �\�[�g���̊֌W�Ō�ł܂Ƃ߂ēo�^
	int nIndexLast = -1;
	for( int i = nCount - 1; 0 <= i; i-- ){
		const CMarkerItem& item = *CColorMarkerVisitor().GetColorMarker(docLine, i);
		bool bDelete = false;
		if( nFromX <= item.m_nBegin ){
			if( item.m_nEnd <= nToX ){
				// [<>] :�폜 []�I��͈� <>�}�[�J�[�͈�
				bDelete = true;
			}else if( item.m_nBegin < nToX ){
				//[<] > :[]<> �E�ɏk��
				CMarkerItem ins = item;
				ins.m_nBegin = nToX;
				vecIns.push_back(ins);
				bDelete = true;
			}else{
				//[]<>:�͈͊O
			}
		}else if( nFromX < item.m_nEnd ){
			if( item.m_nEnd <= nToX ){
				//< [>] : <>[ ] ���ɏk��
				CMarkerItem ins = item;
				ins.m_nEnd= nFromX;
				vecIns.push_back(ins);
				bDelete = true;
			}else{
				//< [] > : <>[]<> ���E����
				CMarkerItem insBefore = item;
				CMarkerItem insAfter = item;
				insAfter.m_nBegin = nToX;
				insBefore.m_nEnd = nFromX;
				vecIns.push_back(insAfter);
				vecIns.push_back(insBefore);
				bDelete = true;
			}
		}else{
			// <>[] :�͈͊O
		}
		if( bDelete ){
			if( nIndexLast == -1 ){
				nIndexLast = i + 1;
			}
		}else if( nIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nIndexLast);
			nIndexLast = -1;
		}
	}
	if( nIndexLast != -1 ){
		CColorMarkerVisitor().DelColorMarker(docLine, 0, nIndexLast);
	}
	const int nInsCount = static_cast<int>(vecIns.size());
	// ��납��ǉ������̂ŋt���ő}���B����œ����͈͂̃A�C�e�����o�^����ۂĂ�
	for( int i = nInsCount - 1; 0 <= i; i-- ){
		CColorMarkerVisitor().AddColorMarker(docLine, vecIns[i]);
	}
}

/*! �J���[�}�[�J�[�폜
*/
void CViewCommander::Command_DELCOLORMARKER(void)
{
	CLayoutRange rgSelect = GetSelect();
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		CLayoutRect rcSelect;
		TwoPointToRect(&rcSelect, rgSelect.GetFrom(), rgSelect.GetTo());
		CLogicYInt nLinePrev = CLogicYInt(-1);
		for( CLayoutYInt nLineNum = rcSelect.top; nLineNum <= rcSelect.bottom; nLineNum++ ){
			CLogicPoint ptLogic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.left, nLineNum), &ptLogic);
			CLogicPoint ptLogicTo;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.right, nLineNum), &ptLogicTo);
			CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(ptLogic.y);
			if( docLine ){
				CViewCommander_ColorMarkerDelItemPos(docLine, ptLogic.x, ptLogicTo.x);
			}
		}
	}else{
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			rgSelect.SetFrom(GetCaret().GetCaretLayoutPos());
			rgSelect.SetTo(GetCaret().GetCaretLayoutPos());
		}
		CLayoutRect rc;
		TwoPointToRect(&rc, rgSelect.GetFrom(), rgSelect.GetTo());
		CLayoutRange rgFixedSelect(CLayoutPoint(rc.left, rc.top), CLayoutPoint(rc.right, rc.bottom));
		CLogicRange rgLogicRange;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(rgFixedSelect, &rgLogicRange);
		const CLogicYInt nBegin = rgLogicRange.GetFrom().y;
		const CLogicYInt nEnd = rgLogicRange.GetTo().y;
		CLogicYInt nEnd2 = nEnd;
		if( rgLogicRange.GetFrom().y != rgLogicRange.GetTo().y
			 && rgLogicRange.GetTo().x == CLogicXInt(0) ){
			nEnd2--; // ���̍s���܂őI���������ꍇ�́A�O�̍s�܂łɂ���
		}
		for( CLogicYInt nLineNum = nBegin; nLineNum <= nEnd2; nLineNum++ ){
			CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
			if( docLine ){
				CLogicXInt nFromX = CLogicXInt(0);
				CLogicXInt nToX = CLogicXInt(-1);
				if( nLineNum == nBegin ){
					nFromX = rgLogicRange.GetFrom().x;
				}
				if( nLineNum == nEnd ){
					nToX = rgLogicRange.GetTo().x;
				}
				if( CLogicXInt(-1) == nToX ){
					nToX = CLogicXInt(INT_MAX);
				}
				CViewCommander_ColorMarkerDelItemPos(docLine, nFromX, nToX); 
			}
		}
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

/*! �J���[�}�[�J�[�ڍאݒ�(�_�C�A���O)
*/
void CViewCommander::Command_DLGCOLORMARKER(void)
{
	CDlgColorMarker cDlg;
	cDlg.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0, GetDocument(), false);
}

/*! ���̃J���[�}�[�J�[
*/
void CViewCommander::Command_COLORMARKER_NEXT(void)
{
	CLogicPoint	ptXY = GetCaret().GetCaretLogicPos();
	CLogicPoint	ptOld = ptXY;

	for( int n = 0; n < 2; n++ ){
		if( CColorMarkerVisitor().SearchColorMarker(&GetDocument()->m_cDocLineMgr, ptXY, SEARCH_FORWARD, ptXY) ){
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting(ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
			if( ptXY < ptOld ){
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if( !GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
			break;
		}
		ptXY.x = 0;
		ptXY.y = 0;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_MARKER_NEXT_NOT_FOUND));
	return;
}

/*! �O�̃J���[�}�[�J�[
*/
void CViewCommander::Command_COLORMARKER_PREV(void)
{
	CLogicPoint	ptXY = GetCaret().GetCaretLogicPos();
	CLogicPoint	ptOld = ptXY;

	for( int n = 0; n < 2; n++ ){
		if( CColorMarkerVisitor().SearchColorMarker(&GetDocument()->m_cDocLineMgr, ptXY, SEARCH_BACKWARD, ptXY) ){
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting(ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
			if( ptOld <= ptOld ){
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if( !GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
			break;
		}
		ptXY.x = INT_MAX;
		ptXY.y = GetDocument()->m_cDocLineMgr.GetLineCount() - 1;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_MARKER_PREV_NOT_FOUND));
	return;
}

/*! �J���[�}�[�J�[�w��폜
*/
BOOL CViewCommander::Command_DELCOLORMARKER_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( !CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		ErrorBeep();
		return FALSE;
	}
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetDocLineTop();
	while( docLine ){
		const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
		int nDelIndexLast = -1;
		for( int i = nCount - 1; 0 <= i; i-- ){
			CMarkerItem& data = *CColorMarkerVisitor().GetColorMarker(docLine, i);
			
			if( item.m_nBold == data.m_nBold
				&& item.m_nUnderLine == data.m_nUnderLine
				&& item.m_cTEXT == data.m_cTEXT
				&& item.m_cBACK == data.m_cBACK
				&& item.m_nGyouLine == data.m_nGyouLine
				&& item.m_nExtValue == data.m_nExtValue
			){
				if( nDelIndexLast == -1 ){
					nDelIndexLast = i + 1;
				}
			}else if( nDelIndexLast != -1 ){
				CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nDelIndexLast);
				nDelIndexLast = -1;
			}
		}
		if( nDelIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, 0, nDelIndexLast);
		}
		docLine = docLine->GetNextLine();
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
	return TRUE;
}

/*! �J���[�}�[�J�[�S�폜
*/
void CViewCommander::Command_DELCOLORMARKER_ALL(void)
{
	CColorMarkerVisitor().ResetAllColorMarker(&GetDocument()->m_cDocLineMgr);
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

/*! �J���[�}�[�J�[�폜���ʒl
*/
void CViewCommander::Command_DELCOLORMARKER_EXTVALUE(UINT nExtValue, UINT nExtValue2)
{
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetDocLineTop();
	while( docLine ){
		int nDelIndexLast = -1;
		const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
		for( int i = nCount - 1; 0 <= i; i-- ){
			CMarkerItem& data = *CColorMarkerVisitor().GetColorMarker(docLine, i);
			if( nExtValue <= data.m_nExtValue && data.m_nExtValue <= nExtValue2 ){
				if( nDelIndexLast == -1 ){
					nDelIndexLast = i + 1;
				}
			}else if( nDelIndexLast != -1 ){
				CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nDelIndexLast);
				nDelIndexLast = -1;
			}
		}
		if( nDelIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, 0, nDelIndexLast);
		}
		docLine = docLine->GetNextLine();
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

static bool GetSCMFileName(CEditDoc* pcEditDoc, LPCWSTR pszFileName, bool bSave, std::tstring& tstr)
{
	if( pszFileName == NULL || pszFileName[0] == L'\0' ){
		TCHAR szPath[_MAX_PATH];
		CDlgOpenFile cDlgOpenFile;
		cDlgOpenFile.Create(
			G_AppInstance(),
			pcEditDoc->m_pcEditWnd->GetHwnd(),
			_T("*.scm"),
			CSakuraEnvironment::GetDlgInitialDir().c_str()
		);
		auto_strcpy(szPath, pcEditDoc->m_cDocFile.GetFilePath());
		if( szPath[0] ){
			if( lstrlen(szPath) + 4 < _countof(szPath) ){
				auto_strcat(szPath, _T(".scm"));
			}
		}
		if( bSave ){
			if( false == cDlgOpenFile.DoModal_GetSaveFileName(szPath) ){
				return false; // Cancel
			}
		}else{
			if( false == cDlgOpenFile.DoModal_GetOpenFileName(szPath) ){
				return false; // Cancel
			}
		}
		tstr = szPath;
	}else{
		tstr = to_tchar(pszFileName);
	}
	return true;
}

/*! �J���[�}�[�J�[�ǂݍ���
*/
void CViewCommander::Command_COLORMARKER_LOAD(LPCWSTR pszFileName)
{
	std::tstring tstr;
	if( GetSCMFileName(GetDocument(), pszFileName, false, tstr) ){
		if( CColorMarkerVisitor().LoadColorMarker(&GetDocument()->m_cDocLineMgr, tstr.c_str()) ){
			GetDocument()->m_pcEditWnd->Views_Redraw();
		}else{
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_LOADCOLORMAKER), tstr.c_str());
		}
	}
}

/*! �J���[�}�[�J�[��������
*/
void CViewCommander::Command_COLORMARKER_SAVE(LPCWSTR pszFileName)
{
	std::tstring tstr;
	if( GetSCMFileName(GetDocument(), pszFileName, true, tstr) ){
		if( CColorMarkerVisitor().SaveColorMarker(&GetDocument()->m_cDocLineMgr, tstr.c_str()) ){
		}else{
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_SAVECOLORMAKER), tstr.c_str());
		}
	}
}

/*! �J���[�}�[�J�[�v���Z�b�g�E����ݒ�
*/
BOOL CViewCommander::Command_COLORMARKER_SETPRESET(int nIndex, LPCWSTR pszMarker, LPCWSTR pszName)
{
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	CMarkerItem item;
	CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item);
	if( nIndex == 0 ){
		markerSet.m_ColorItemLast = item;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		markerSet.m_ColorItems[nIndex - 1] = item;
		wcscpyn(markerSet.m_szSetNames[nIndex - 1], pszName, _countof(markerSet.m_szSetNames[0]));
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}
