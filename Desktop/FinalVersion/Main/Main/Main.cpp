#include "pch.h"
#include "framework.h"
#include "Main.h"
#include "CLoginDlg.h"
#include "CSignupDlg.h"
#include "CHomeChatDlg.h"
#include "CChatFriendDlg.h"
#include "DatabaseService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMainApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()



CMainApp::CMainApp(){
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}


CMainApp theApp;

void CMainApp::Logout() {
	m_userData = User();
	m_vecFriend.clear();
    m_mapNickname.clear();
}


BOOL CMainApp::InitInstance(){
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	
    bool running = TRUE;
    INT curDlg = IDD_LOGIN_DIALOG;

    while (running) {
        if (curDlg == IDD_LOGIN_DIALOG) {
            CLoginDlg dlg;
            INT_PTR nResponse = dlg.DoModal(); 

            if (nResponse == ID_SIGNUP_TRIGGER) {
                curDlg = IDD_SIGNUP_DIALOG;
            }
            else if (nResponse == ID_HOMECHAT_TRIGGER) {

                bool success = DatabaseService::InitializeUserDB(theApp.m_userData.username, theApp.m_userData.password);

                if (success) {
                    DatabaseService::SaveUserInfo(theApp.m_userData);

                    curDlg = IDD_HOMECHAT_DIALOG;
                }
            }   
            else {
                running = FALSE;
            }
        }
        else if (curDlg == IDD_SIGNUP_DIALOG) {
            CSignupDlg dlg;
            INT_PTR nResponse = dlg.DoModal();

            if (nResponse == ID_LOGIN_TRIGGER) {
                curDlg = IDD_LOGIN_DIALOG;
            }
            else {
                running = FALSE;
            }
        }
        else if (curDlg == IDD_HOMECHAT_DIALOG) {
            CHomeChatDlg dlg;
            INT_PTR nResponse = dlg.DoModal();

            if (nResponse == ID_LOGIN_TRIGGER) {
                curDlg = IDD_LOGIN_DIALOG;
            }
            else {
                running = FALSE;
            }
        }
    }

	if (pShellManager != nullptr){
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();T
#endif

	return FALSE;
}

int CMainApp::ExitInstance() {
	DatabaseService::CloseDB();
	return CWinApp::ExitInstance();
}

