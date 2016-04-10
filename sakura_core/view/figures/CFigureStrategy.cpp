/*
	Copyright (C) 2008, kobake

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
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/CViewFont.h"
#include "CFigureStrategy.h"
#include "doc/layout/CLayout.h"
#include "charset/charcode.h"
#include "types/CTypeSupport.h"

bool CFigure_Text::DrawImp(SColorStrategyInfo* pInfo)
{
	int nIdx = pInfo->GetPosInLogic();
	int nLength =	CNativeW::GetSizeOfChar(	// �T���Q�[�g�y�A�΍�	2008.10.12 ryoji
						pInfo->m_pLineOfLogic,
						pInfo->GetDocLine()->GetLengthWithoutEOL(),
						nIdx
					);
	bool bTrans = pInfo->m_pcView->IsBkBitmap() && CTypeSupport(pInfo->m_pcView, COLORIDX_TEXT).GetBackColor() == GetBkColor(pInfo->m_gr);
	int fontNo = (nLength == 2 ? WCODE::GetFontNo2(pInfo->m_pLineOfLogic[nIdx], pInfo->m_pLineOfLogic[nIdx+1]):
			WCODE::GetFontNo(pInfo->m_pLineOfLogic[nIdx]));
	if( fontNo ){
		CTypeSupport cCurrentType(pInfo->m_pcView, pInfo->GetCurrentColor());	// ���ӂ̐F�i���݂̎w��F/�I��F�j
		CTypeSupport cCurrentType2(pInfo->m_pcView, pInfo->GetCurrentColor2());	// ���ӂ̐F�i���݂̎w��F�j
		bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor();
		SFONT sFont;
		sFont.m_sFontAttr.m_bBoldFont  = (blendColor ? cCurrentType2.IsBoldFont() : cCurrentType.IsBoldFont());
		sFont.m_sFontAttr.m_bUnderLine = (blendColor ? cCurrentType2.HasUnderLine() : cCurrentType.HasUnderLine());
		sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
		pInfo->m_gr.PushMyFont(sFont);
	}
	int nHeightMargin = pInfo->m_pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
	pInfo->m_pcView->GetTextDrawer().DispText(
		pInfo->m_gr,
		pInfo->m_pDispPos,
		nHeightMargin,
		&pInfo->m_pLineOfLogic[nIdx],
		nLength,
		bTrans
	);
	if( fontNo ){
		pInfo->m_gr.PopMyFont();
	}
	pInfo->m_nPosInLogic += nLength;
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�擝��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CFigureSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// ���݈ʒu���o���Ă���
	DispSpace(pInfo->m_gr, pInfo->m_pDispPos,pInfo->m_pcView, bTrans);	// �󔒕`��
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1�����O��
	pInfo->m_nPosInLogic += CNativeW::GetSizeOfChar(	// �s���ȊO�͂����ŃX�L�����ʒu���P���i�߂�
		pInfo->m_pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

ColorInfoBase CFigureSpace::SetColorInfoFromMarker(SColorStrategyInfo* pInfo, const ColorInfoBase& colorBase, int nColorIdx)
{
	ColorInfoBase color = colorBase;
	if( pInfo->GetEnableMarker() && !(COLORIDX_SEARCH <= nColorIdx && nColorIdx <= COLORIDX_SEARCHTAIL) ){
		const CMarkerItem& marker = pInfo->GetMarkerItem();
		if( marker.IsBoldSet() ){
			color.m_sFontAttr.m_bBoldFont = marker.IsBold();
		}
		if( marker.IsUnderLineSet() ){
			color.m_sFontAttr.m_bUnderLine = marker.IsUnderLine();
		}
		if( marker.m_cTEXT != -1 ){
			color.m_sColorAttr.m_cTEXT = marker.m_cTEXT;
		}
		if( marker.m_cBACK != -1 ){
			color.m_sColorAttr.m_cBACK = marker.m_cBACK;
		}
	}
	return color;
}

bool CFigureSpace::DrawImp_StyleSelect(SColorStrategyInfo* pInfo)
{
	// ���� DrawImp �͂����i��{�N���X�j�Ńf�t�H���g������������Ă��邪
	// ���z�֐��Ȃ̂Ŕh���N���X���̃I�[�o�[���C�h�ŌʂɎd�l�ύX�\
	CEditView* pcView = pInfo->m_pcView;
	const int		nColorIdx = ToColorInfoArrIndex(pInfo->GetCurrentColor());
	const int		nColorIdx2 = ToColorInfoArrIndex(pInfo->GetCurrentColor2());
	const int		nColorIdxBg = ToColorInfoArrIndex(pInfo->GetCurrentColorBg());

	const ColorInfoBase& colorType = m_pTypeData->m_ColorInfoArr[nColorIdx];		// ���ӂ̐F�i���݂̎w��F/�I��F�j
	const ColorInfoBase& colorType2 = m_pTypeData->m_ColorInfoArr[nColorIdx2];	// ���ӂ̐F�i���݂̎w��F�j
	const ColorInfoBase& colorText = m_pTypeData->m_ColorInfoArr[COLORIDX_TEXT];
	const ColorInfoBase& colorSpace = m_pTypeData->m_ColorInfoArr[GetDispColorIdx()];
	const ColorInfoBase& colorBg = m_pTypeData->m_ColorInfoArr[nColorIdxBg];
	const COLORREF crTextBack = colorText.GetBackColor();

	ColorInfoBase colorType3 = SetColorInfoFromMarker(pInfo, colorType2, nColorIdx2);
	const ColorInfoBase& colorType4 = (colorType3.GetBackColor() == crTextBack ? colorBg: colorType3);

	// �󔒋L���ނ͓��ɖ����w�肵�������ȊO�͂Ȃ�ׂ����ӂ̎w��ɍ��킹��悤�ɂ��Ă݂�	// 2009.05.30 ryoji
	// �Ⴆ�΁A�������w�肵�Ă��Ȃ��ꍇ�A���K�\���L�[���[�h���Ȃ琳�K�\���L�[���[�h���̉����w��ɏ]���ق������R�ȋC������B
	// �i���̂ق����󔒋L���́u�\���v���`�F�b�N���Ă��Ȃ��ꍇ�̕\���ɋ߂��j
	//
	// �O�i�F�E�w�i�F�̈���
	// �E�ʏ�e�L�X�g�Ƃ͈قȂ�F���w�肳��Ă���ꍇ�͋󔒋L���̑��̎w��F���g��
	// �E�ʏ�e�L�X�g�Ɠ����F���w�肳��Ă���ꍇ�͎��ӂ̐F�w��ɍ��킹��
	// �����̈���
	// �E�󔒋L�������ӂ̂ǂ��炩����ł������w�肳��Ă���΁u�O�i�F�E�w�i�F�̈����v�Ō��肵���O�i�F�ő����ɂ���
	// �����̈���
	// �E�󔒋L���ŉ����w�肳��Ă���΁u�O�i�F�E�w�i�F�̈����v�Ō��肵���O�i�F�ŉ���������
	// �E�󔒋L���ŉ����w�肳��Ă��炸���ӂŉ����w�肳��Ă���Ύ��ӂ̑O�i�F�ŉ���������
	// [�I��]�����_�����O��
	// �E�����F�̏ꍇ�͏]���ʂ�B
	COLORREF crText;
	COLORREF crBack;
	bool bBold;
	bool bItalic;
	const bool bNotSelecting = pInfo->GetCurrentColor() == pInfo->GetCurrentColor2();
	const bool blendColor = !bNotSelecting && colorType.GetTextColor() == colorType.GetBackColor(); // �I�������F
	const bool bText = colorSpace.GetTextColor() == colorText.GetTextColor();
	const bool bBack = colorSpace.GetBackColor() == crTextBack;
	if( bNotSelecting ){
		// �ʏ�F(���I��)
		crText = (bText ? colorType3 : colorSpace).GetTextColor();
		crBack = (bBack ? colorType4 : colorSpace).GetBackColor();
		bBold = colorType3.IsBoldFont();
		bItalic = colorType3.IsItalic();
	}else if( !blendColor ){
		// �w��I��F
		const ColorInfoBase& colorType5 = (colorType.GetBackColor() == crTextBack ? colorBg: colorType);
		crText = (bText ? colorType : colorSpace).GetTextColor();
		crBack = (bBack ? colorType5 : colorSpace).GetBackColor();
		bBold = colorType.IsBoldFont();
		bItalic = colorType.IsItalic();
	}else{
		// �A���t�@or���]
		const ColorInfoBase& cText = (bText ? colorType3 : colorSpace);
		const ColorInfoBase& cBack = (bBack ? colorType4 : colorSpace);
		crText = pcView->GetTextColorByColorInfo2(colorType, cText);
		crBack = pcView->GetBackColorByColorInfo2(colorType, cBack);
		bBold = colorType3.IsBoldFont();
		bItalic = colorType3.IsItalic();
	}

	pInfo->m_gr.PushTextForeColor(crText);
	pInfo->m_gr.PushTextBackColor(crBack);
	// Figure�������w��Ȃ炱����ŉ������w��B���̐F�̂ق��������w��Ȃ�ADrawImp_DrawUnderline�ŉ��������w��
	SFONT sFont;
	sFont.m_sFontAttr.m_bBoldFont = colorSpace.IsBoldFont() || bBold;
	sFont.m_sFontAttr.m_bUnderLine = colorSpace.HasUnderLine();
	sFont.m_sFontAttr.m_bItalic = colorSpace.IsItalic() || bItalic;
	sFont.m_sFontAttr.m_bStrikeOut = colorSpace.IsStrikeOut();
	sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle( 0, sFont.m_sFontAttr );
	pInfo->m_gr.PushMyFont(sFont);
	bool bTrans = pcView->IsBkBitmap() && colorText.GetBackColor() == crBack;
	return bTrans;
}

void CFigureSpace::DrawImp_StylePop(SColorStrategyInfo* pInfo)
{
	pInfo->m_gr.PopTextForeColor();
	pInfo->m_gr.PopTextBackColor();
	pInfo->m_gr.PopMyFont();
}

void CFigureSpace::DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos& sPos)
{
	CEditView* pcView = pInfo->m_pcView;

	const int		nColorIdx = ToColorInfoArrIndex(pInfo->GetCurrentColor());
	const int		nColorIdx2 = ToColorInfoArrIndex(pInfo->GetCurrentColor2());
	const ColorInfoBase& colorType = m_pTypeData->m_ColorInfoArr[nColorIdx];
	const ColorInfoBase& colorType2 = m_pTypeData->m_ColorInfoArr[nColorIdx2];

	ColorInfoBase colorType3 = SetColorInfoFromMarker(pInfo, colorType2, nColorIdx2);

	// �w��I��F
	bool bSelectColor = (pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && colorType.GetTextColor() != colorType.GetBackColor());

	const ColorInfoBase& colorStyle = (bSelectColor ? colorType : colorType3);

	CTypeSupport cSpaceType(pcView, GetDispColorIdx());	// �󔒂̎w��F

	if( (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine())
	 || (!cSpaceType.IsStrikeOut() && colorStyle.IsStrikeOut())
	)
	{
		int fontNo = WCODE::GetFontNo(' ');
		// ���������ӂ̑O�i�F�ŕ`�悷��
		SFONT sFont;
		sFont.m_sFontAttr.m_bBoldFont = false;
		sFont.m_sFontAttr.m_bUnderLine = colorStyle.HasUnderLine();
		sFont.m_sFontAttr.m_bItalic = false;
		sFont.m_sFontAttr.m_bStrikeOut = colorStyle.IsStrikeOut();
		sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle( fontNo, sFont.m_sFontAttr );
		pInfo->m_gr.PushMyFont(sFont);

		int nHeightMargin = pInfo->m_pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		CLayoutXInt nColLength = CLayoutXInt(pInfo->m_pDispPos->GetDrawCol() - sPos.GetDrawCol());
		int nSpWidth = pcView->GetTextMetrics().CalcTextWidth3(L" ", 1);
		int nLength = (Int)(nColLength + nSpWidth - 1) / nSpWidth;
		wchar_t* pszText = new wchar_t[nLength];
		std::vector<int> vDxArray(nLength);
		for( int i = 0; i < nLength; i++ ){
			pszText[i] = L' ';
			vDxArray[i] = nSpWidth;
		}
		RECT rcClip2;
		rcClip2.left = sPos.GetDrawPos().x;
		rcClip2.right = rcClip2.left + (Int)(nColLength); // �O������FCLayoutInt == px
		if( rcClip2.left < pcView->GetTextArea().GetAreaLeft() ){
			rcClip2.left = pcView->GetTextArea().GetAreaLeft();
		}
		rcClip2.top = sPos.GetDrawPos().y;
		rcClip2.bottom = sPos.GetDrawPos().y + sPos.GetCharHeight();
		::ExtTextOutW_AnyBuild(
			pInfo->m_gr,
			sPos.GetDrawPos().x,
			sPos.GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(ETO_OPAQUE),
			&rcClip2,
			pszText,
			nLength,
			&vDxArray[0]
		);
		delete []pszText;
		pInfo->m_gr.PopMyFont();
	}
}

