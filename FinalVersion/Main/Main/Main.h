#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"
#include <vector>
#include "User.h"
#include "Friend.h"
#include "Message.h"

class CMainApp : public CWinApp{
public:
	CMainApp();
	User m_userData;
	std::vector <Friend> m_vecFriend;
	Friend m_selectedFriend;
	std::vector<Message> m_vecMessage;

public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();
	void Logout();

	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;
