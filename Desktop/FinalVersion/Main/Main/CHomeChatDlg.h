#pragma once
#include "afxdialogex.h"
#include "Friend.h"
#include "CChatFriendDlg.h"
#include <map>
#include <string>
#include <vector>
#include "CUserDlg.h"
#include <map>
#include "Nickname.h"


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
	bool GetFiendList();
	void GetUserData();
	void DrawFriendList(CDC* pDC);
	bool GetNickname();

	CRect m_rectFriendArea;
	int m_nFriendScrollPos = 0;
	int m_nTotalFriendHeight = 0;
	std::vector<Friend> m_vecFriendDisplay;
	CFont m_fontTitle;
	std::map<std::string, CChatFriendDlg*> m_mapChatWindows;
	CRect m_rectUserAvatar;
	CUserDlg* m_pUserDlg = nullptr;
	bool m_isUserCreate = false;
	std::vector<Friend> m_tempFriendList;
	std::map<std::string, NicknameInfo> m_tempNickname;

	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChangeEditHomechatSearch();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnLogoutSignal(WPARAM wParam, LPARAM lParam);
};
