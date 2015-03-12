#ifndef SAKURA_CDLGCONFIGCHILDMACRO_H_
#define SAKURA_CDLGCONFIGCHILDMACRO_H_

#include "CDlgConfigChild.h"

//IDD_PROP_MACRO

class CDlgConfigChildMacro : public CDlgConfigChild
{
public:
	CDlgConfigChildMacro( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	BOOL OnEnKillFocus( HWND hwndCtl, int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void InitDialog( HWND hwndDlg );	//!< Macro�y�[�W�̏�����
	void SetMacro2List_Macro();			//!< Macro�f�[�^�̐ݒ�
	void SelectBaseDir_Macro();			//!< Macro�f�B���N�g���̑I��
	void OnFileDropdown_Macro();		//!< �t�@�C���h���b�v�_�E�����J�����Ƃ�
	void CheckListPosition_Macro();		//!< ���X�g�r���[��Focus�ʒu�m�F

	int				m_nLastPos; //!< �O��t�H�[�J�X�̂������ꏊ
};

#endif
