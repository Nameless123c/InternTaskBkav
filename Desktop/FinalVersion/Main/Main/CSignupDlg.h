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
	virtual BOOL OnInitDialog();
	CFont m_fontTitle;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	CMFCButton m_btnSignup;
	afx_msg void OnStnClickedStaticSignupGotoLogin();
	afx_msg void OnStnClickedStaticSignupExit();
	afx_msg LRESULT OnNcHitTest(CPoint point);
};
