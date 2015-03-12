/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�X�e�[�^�X�o�[�v�y�[�W

	@author Uchi
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta
	Copyright (C) 2007, Uchi
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildStatusbar.h"
#include "prop/CDlgConfig.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJIS�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JIS�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUC�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UTF8_CODEPOINT,		HIDC_CHECK_DISP_UTF8_CODEPOINT,			// UTF-8���R�[�h�|�C���g�ŕ\������
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\������
	IDC_CHECK_DISP_SELCOUNT_BY_BYTE,	HIDC_CHECK_DISP_SELCOUNT_BY_BYTE,		// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	0, 0
};


HWND CDlgConfigChildStatusbar::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_STATUSBAR, 0, SW_SHOW );
}


BOOL CDlgConfigChildStatusbar::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	return result;
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgConfigChildStatusbar::SetData()
{
	HWND hwndDlg = GetHwnd();

	// �������R�[�h�̎w��
	// SJIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, m_Common.m_sStatusbar.m_bDispUniInSjis );
	// JIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  m_Common.m_sStatusbar.m_bDispUniInJis );
	// EUC�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  m_Common.m_sStatusbar.m_bDispUniInEuc );
	// UTF-8�ŕ\�����o�C�g�R�[�h�ōs��
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT,  m_Common.m_sStatusbar.m_bDispUtf8Codepoint );
	// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\��
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    m_Common.m_sStatusbar.m_bDispSPCodepoint );
	// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE,m_Common.m_sStatusbar.m_bDispSelCountByByte );
	return;
}


/* �_�C�A���O�f�[�^�̎擾 */
int CDlgConfigChildStatusbar::GetData()
{
	HWND hwndDlg = GetHwnd();

	// �������R�[�h�̎w��
	// SJIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInSjis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS );
	// JIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInJis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS );
	// EUC�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInEuc		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC );
	// UTF-8�ŕ\�����o�C�g�R�[�h�ōs��
	m_Common.m_sStatusbar.m_bDispUtf8Codepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT );
	// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\��
	m_Common.m_sStatusbar.m_bDispSPCodepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT );
	// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	m_Common.m_sStatusbar.m_bDispSelCountByByte	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE );

	return TRUE;
}


LPVOID CDlgConfigChildStatusbar::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
