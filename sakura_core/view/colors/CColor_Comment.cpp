#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_LineComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// �s�R�����g
	if( m_pTypeData->m_cLineComment.Match( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		return true;
	}
	return false;
}

bool CColor_LineComment::EndColor(const CStringRef& cStr, int nPos)
{
	//������I�[
	if( nPos >= cStr.GetLength() ){
		return true;
	}

	//���s
	if( WCODE::IsLineDelimiter(cStr.At(nPos), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
		return true;
	}

	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CLayoutColorBlockCommentInfo : public CLayoutColorInfo{
public:
	int m_nNest;

	~CLayoutColorBlockCommentInfo(){}
	bool IsEqual(const CLayoutColorInfo* p) const{
		if( !p ){
			return false;
		}
		const CLayoutColorBlockCommentInfo* info = dynamic_cast<const CLayoutColorBlockCommentInfo*>(p);
		if( info == NULL ){
			return false;
		}
		return info->m_nNest == this->m_nNest;
	}
};

void CColor_BlockComment::SetStrategyColorInfo(const CLayoutColorInfo* colorInfo)
{
	if( colorInfo ){
		const CLayoutColorBlockCommentInfo* info = dynamic_cast<const CLayoutColorBlockCommentInfo*>(colorInfo);
		if( info == NULL ){
			return;
		}
		m_nNest = info->m_nNest;
	}else{
		m_nNest = 0; // 1���߂̒��Ȃ̂�
	}
}

CLayoutColorInfo* CColor_BlockComment::GetStrategyColorInfo() const
{
	if( 0 < m_nNest ){
		CLayoutColorBlockCommentInfo* info = new CLayoutColorBlockCommentInfo();
		info->m_nNest = m_nNest;
		return info;
	}
	return NULL;
}

bool CColor_BlockComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// �u���b�N�R�����g
	if( m_pcBlockComment->Match_CommentFrom( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		if( m_pcBlockComment->m_bCommentNest ){
			this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo2(
				nPos + m_pcBlockComment->getBlockFromLen(),
				cStr,
				m_nNest
			);
			return true;
		}
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */	//@@@ 2002.09.22 YAZAKI
		this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
			nPos + m_pcBlockComment->getBlockFromLen(),
			cStr
		);

		return true;
	}
	return false;
}

bool CColor_BlockComment::EndColor(const CStringRef& cStr, int nPos)
{
	if( 0 == this->m_nCOMMENTEND ){
		if( m_pcBlockComment->m_bCommentNest ){
			this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo2(
				nPos,
				cStr,
				m_nNest
			);
		}else{
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
				nPos,
				cStr
			);
		}
	}
	else if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

