#pragma once
#include "afxdialogex.h"


class CLoginDlg : public CDialogEx{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = nullptr);  
	virtual ~CLoginDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnLoginSubmit();
	afx_msg void OnStnClickedStaticLoginGotoSignup();
};
