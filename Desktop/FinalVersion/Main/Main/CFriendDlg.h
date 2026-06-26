#pragma once
#include "afxdialogex.h"
#include "Friend.h"


class CFriendDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFriendDlg)

public:
	CFriendDlg(CWnd* pParent = nullptr);   
	virtual ~CFriendDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRIEND_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	Friend m_friendData;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CMFCButton m_btnNickName;
	afx_msg void OnBnClickedBtnFriendSubmit();
	CFont m_fontTitle;
	afx_msg void OnStnClickedStaticFriendExit();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnStnClickedStaticFriendDelete();
};
