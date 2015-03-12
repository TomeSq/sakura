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
#ifndef SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_
#define SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_

class CImageListMgr;
class CMenuDrawer;

/*! �v���p�e�B�V�[�g�ԍ�
	@date 2008.6.22 Uchi #define -> enum �ɕύX	
	@date 2008.6.22 Uchi�����ύX Win,Toolbar,Tab,Statusbar�̏��ɁAFile,FileName ����
	@date 2014.11.02 �ꕔ���̕ύX
*/
enum PropComSheetOrder {
	ID_PROPCOM_PAGENUM_GENERAL = 0,		//!< �S��
	ID_PROPCOM_PAGENUM_WINDOW,				//!< �E�B���h�E
	ID_PROPCOM_PAGENUM_MAINMENU,		//!< ���C�����j���[
	ID_PROPCOM_PAGENUM_TOOLBAR,			//!< �c�[���o�[
	ID_PROPCOM_PAGENUM_TABS,				//!< �^�u�o�[
	ID_PROPCOM_PAGENUM_STATUSBAR,		//!< �X�e�[�^�X�o�[
	ID_PROPCOM_PAGENUM_EDIT,			//!< �ҏW
	ID_PROPCOM_PAGENUM_FILE,			//!< �t�@�C��
	ID_PROPCOM_PAGENUM_FILENAME,		//!< �t�@�C�����\��
	ID_PROPCOM_PAGENUM_BACKUP,			//!< �o�b�N�A�b�v
	ID_PROPCOM_PAGENUM_FORMAT,			//!< ����
	ID_PROPCOM_PAGENUM_SEARCH,			//!< ����
	ID_PROPCOM_PAGENUM_KEYBOARD,		//!< �L�[���蓖��
	ID_PROPCOM_PAGENUM_CUSTMENU,		//!< �J�X�^�����j���[
	ID_PROPCOM_PAGENUM_KEYWORD,			//!< �����L�[���[�h
	ID_PROPCOM_PAGENUM_SUPPORT,			//!< �x��
	ID_PROPCOM_PAGENUM_MACRO,			//!< �}�N��
	ID_PROPCOM_PAGENUM_PLUGIN,			//!< �v���O�C��
	ID_PROPCOM_PAGENUM_MAX,
	ID_PROPCOM_PAGENUM_NONE = -1		//!< ���w��
};

class CPropertyManager{
public:
	void Create( HWND, CImageListMgr*, CMenuDrawer* );

	/*
	|| ���̑�
	*/
	bool OpenPropertySheet( HWND hWnd, int nPageNum, bool bTrayProc );	/* ���ʐݒ� */
	bool OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType );	/* �^�C�v�ʐݒ� */

private:
	HWND			m_hwndOwner;
	CImageListMgr*	m_pImageList;
	CMenuDrawer*	m_pMenuDrawer;

	int				m_nPropComPageNum;
	int				m_nPropTypePageNum;
};

#endif /* SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_ */
/*[EOF]*/
