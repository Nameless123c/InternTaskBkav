#pragma once
#include "afxdialogex.h"


class CSignupDlg : public CDialogEx{
	DECLARE_DYNAMIC(CSignupDlg)

public:
	CSignupDlg(CWnd* pParent = nullptr);   
	virtual ~CSignupDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIGNUP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);   

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSignupSubmit();
};
