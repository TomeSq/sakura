#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo

#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanSpace                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanSpace::Match(const wchar_t* pText) const
{
	if( pText[0] == L' ' ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ���p�X�y�[�X�`��
void CFigure_HanSpace::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	CMyRect rcClip;
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	const int Dx = pcView->GetTextMetrics().CalcTextWidth3(L" ", 1);
	const CLayoutXInt nCol = CLayoutXInt(Dx);
#else
	const int Dx = pcView->GetTextMetrics().GetDxArray_AllHankaku()[0];
	const CLayoutXInt nCol = CLayoutXInt(1);
#endif
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,nCol))
	{
		//������"o"�̉��������o��
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipBottom,
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
//FIXME:�����Ⴄ
			L"o",
#else
			L"o",
#endif
			1,
			&Dx
		);
		
		//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipTop,
			L" ",
			1,
			&Dx
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(nCol);
}
