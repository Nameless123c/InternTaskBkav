#pragma once
#include "afxdialogex.h"
#include "Friend.h"
#include "CChatFriendDlg.h"
#include <map>
#include <string>
#include <vector>



class CHomeChatDlg : public CDialogEx{
	DECLARE_DYNAMIC(CHomeChatDlg)
	

public:
	CHomeChatDlg(CWnd* pParent = nullptr); 
	virtual ~CHomeChatDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HOMECHAT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	void GetFiendList();
	void GetUserData();
	void DrawFriendList(CDC* pDC);

	CRect m_rectFriendArea;
	int m_nFriendScrollPos = 0;
	int m_nTotalFriendHeight = 0;
	std::vector<Friend> m_vecFriendDisplay;
	CFont m_fontTitle;
	std::map<std::string, CChatFriendDlg*> m_mapChatWindows;

	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChangeEditHomechatSearch();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
