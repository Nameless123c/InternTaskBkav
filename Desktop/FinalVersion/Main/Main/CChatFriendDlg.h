#pragma once
#include "afxdialogex.h"
#include "pch.h"
#include <vector>
#include "Message.h"
#include "Friend.h"
#include "CFriendDlg.h"


class CChatFriendDlg : public CDialogEx{
	DECLARE_DYNAMIC(CChatFriendDlg)

public:
	CChatFriendDlg(CWnd* pParent = nullptr);  
	virtual ~CChatFriendDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATFRIEND_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);  

	DECLARE_MESSAGE_MAP()

public:
	void GetMessage();
	void DrawChatArea(CDC* pDC);
	int DrawSingleMessage(CDC* pDC, const Message& msg, int x, int y, int nContainerWidth);
	void SendMessage();
	void OnTimer(UINT_PTR nIDEvent);
	void UpdateLastMessageTime();

	CImage* m_pImgSend;
	CImage* m_pImgEmoji;
	CImage* m_pImgImage;
	CImage* m_pImgAttach;

	CRect m_rectSendBtn;
	CRect m_rectEmojiBtn;
	CRect m_rectImageBtn;
	CRect m_rectAttachBtn;

	int m_nScrollPos;   
	int m_nTotalHeight; 
	CRect m_rectChatArea;
	bool m_bIsFirstLoad;
	CFont m_fontTitle;
	std::vector<Message> m_vecLocalMessages;
	Friend m_currentFriend;
	CFriendDlg m_friendDlg;
	BOOL m_isFriendDlgCreated = FALSE;
	
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticChatfriendFullname();
	afx_msg void OnClose();
	afx_msg void OnStnClickedStaticChatfriendExit();
	afx_msg LRESULT OnNcHitTest(CPoint point);
};
