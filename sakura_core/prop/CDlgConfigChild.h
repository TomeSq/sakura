#ifndef SAKURA_CDLGCONFIGCHILD_H_
#define SAKURA_CDLGCONFIGCHILD_H_

#include "dlg/CDialog.h"

struct SDlgConfigArg;

class CDlgConfigChild : public CDialog
{
public:
	CDlgConfigChild(CommonSetting& common): m_Common(common){}

	//! ���[�h���X�_�C�A���O�̕\��
	//@note �K���I�[�o�[���C�h���邱��
	virtual HWND DoModeless( HINSTANCE hInstance,			//!< �A�v���P�[�V�����C���X�^���X�n���h��
	                         HWND hwndParent,				//!< �e�E�B���h�E�n���h��
	                         SDlgConfigArg* pDlgConfigArg,	//!< ��Ɨp���L�f�[�^�\����
	                         int nTypeIndex					//!< �^�C�vindex
	) = 0;

	virtual void CloseDialog()
	{
		if( NULL != GetHwnd() ){
			GetData();
			::DestroyWindow( GetHwnd() );
			_SetHwnd(NULL);
		}
		return;
	}

protected:
	/*! �_�C�A���O�̏�����
	    @note �I�[�o�[���C�h�����ꍇ�͂��̊֐����Ăяo���C�܂��͓����e���������邱��
	*/
	virtual BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
	{
		_SetHwnd(hwndDlg);
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		// �_�C�A���O�f�[�^�̐ݒ�
		SetData();
		//�E�B���h�E�ʒu�C�T�C�Y���L�����Ȃ��̂�CDialog::OnInitDialog()���Ă΂Ȃ�
		return TRUE;
	}

	/*! WM_DESTROY ���b�Z�[�W������
	    @note �I�[�o�[���C�h�����ꍇ�͂��̊֐����Ăяo���C�܂��͓����e���������邱��
	*/
	virtual BOOL OnDestroy()
	{
		//// �_�C�A���O�f�[�^�̎擾
		//GetData();
		//_SetHwnd(NULL);
		//�E�B���h�E�ʒu�C�T�C�Y���L�����Ȃ��̂�CDialog::OnDestroy()���Ă΂Ȃ�
		return TRUE;
	}

	/*! WM_SIZE ���b�Z�[�W������
	    @note �_�C�A���O�̈ʒu�͋L�����Ȃ�
	*/
	virtual BOOL OnMove( WPARAM, LPARAM ){ return FALSE; }
	/*! WM_MOVE ���b�Z�[�W������
	    @note �_�C�A���O�̃T�C�Y�͋L�����Ȃ�
	*/
	virtual BOOL OnSize( WPARAM, LPARAM ){ return FALSE; }
	/*! WM_COMMAND ���b�Z�[�W�� BN_CLICKED ������
	    @note OK/�L�����Z�� �{�^���͖���
	*/
	virtual BOOL OnBnClicked( int ){ return FALSE; }

	HFONT SetCtrlFont( int idc_static, const LOGFONT& lf );			//!< �R���g���[���Ƀt�H���g�ݒ肷��		// 2013/4/24 Uchi
	HFONT SetFontLabel( int idc_static, const LOGFONT& lf, int nps, bool bUse = true );	//!< �t�H���g���x���Ƀt�H���g�ƃt�H���g���ݒ肷��	// 2013/4/24 Uchi
protected:
	int				m_nCurrentTypeIndex;
	SDlgConfigArg*	m_pDlgConfigArg;
	CommonSetting&	m_Common;		//!< ��Ɨp���L�f�[�^�\���̂̃|�C���^

};

#endif
