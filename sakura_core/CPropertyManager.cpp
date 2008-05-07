#include "stdafx.h"
#include "CPropertyManager.h"
#include "mymessage.h"

CPropertyManager::CPropertyManager()
{
	/* �ݒ�v���p�e�B�V�[�g�̏������P */
	m_cPropCommon.Create( CEditWnd::Instance()->GetHwnd(), &CEditApp::Instance()->GetIcons(), CEditApp::Instance()->m_pcSMacroMgr, &CEditWnd::Instance()->GetMenuDrawer() );
	m_cPropTypes.Create( G_AppInstance(), CEditWnd::Instance()->GetHwnd() );
}

/*! ���ʐݒ� �v���p�e�B�V�[�g */
BOOL CPropertyManager::OpenPropertySheet( int nPageNum )
{
	// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	m_cPropCommon.InitData();
	
	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropCommon.DoPropertySheet( nPageNum ) ){

		// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
		// ShareData �� �ݒ��K�p�E�R�s�[����
		// 2007.06.20 ryoji �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		BOOL bGroup = (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin);
		m_cPropCommon.ApplyData();
		CEditApp::Instance()->m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �t�H���g���ς���� */
		for( int i = 0; i < 4; ++i ){
			CEditApp::Instance()->m_pcEditWnd->m_pcEditViewArr[i]->m_cTipWnd.ChangeFont( &(GetDllShareData().m_Common.m_sHelper.m_lf_kh) );
		}

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
BOOL CPropertyManager::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	STypeConfig& types = GetDllShareData().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��
	//m_cPropTypes.m_CKeyWordSetMgr = GetDllShareData().m_CKeyWordSetMgr;

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		m_cPropTypes.GetTypeData( types );

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}
