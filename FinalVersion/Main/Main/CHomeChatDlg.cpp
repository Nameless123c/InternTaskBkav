#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CHomeChatDlg.h"
#include "ApiService.h"
#include "ChangeFormat.h"
#include "Friend.h"
#include "FileService.h"
#include "PaintService.h"

IMPLEMENT_DYNAMIC(CHomeChatDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CHomeChatDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_EN_CHANGE(IDC_EDIT_HOMECHAT_SEARCH, &CHomeChatDlg::OnChangeEditHomechatSearch)
END_MESSAGE_MAP()

CHomeChatDlg::CHomeChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOMECHAT_DIALOG, pParent)
{

}

CHomeChatDlg::~CHomeChatDlg(){

}

void CHomeChatDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
}

BOOL CHomeChatDlg::OnInitDialog(){
	CDialogEx::OnInitDialog();

	GetFiendList();
	GetUserData();
	m_vecFriendDisplay = theApp.m_vecFriend;

	CWnd* pArea = GetDlgItem(IDC_STATIC_HOMECHAT_FRIENDAREA);
	if (pArea) {
		pArea->GetWindowRect(&m_rectFriendArea);
		ScreenToClient(&m_rectFriendArea);
		pArea->ShowWindow(SW_HIDE);
	}

	m_nFriendScrollPos = 0;
	m_nTotalFriendHeight = 0;

	return TRUE;
}

void CHomeChatDlg::OnPaint() {
	CPaintDC dc(this);

	GetDlgItem(IDC_STATIC_HOMECHAT_FULLNAME)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_HOMECHAT_FULLNAME)->SetWindowText(ChangeFormat::UTF8ToCString(theApp.m_userData.fullName));

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HOMECHAT_SEARCH);
	if (pEdit) {
		pEdit->SetCueBanner(ChangeFormat::UTF8ToCString("Tìm kiếm"));
	}

	CWnd* pArea = GetDlgItem(IDC_STATIC_HOMECHAT_AVT);
	if (pArea) {
		CRect rectAvt;
		pArea->GetWindowRect(&rectAvt);
		ScreenToClient(&rectAvt);
		pArea->ShowWindow(SW_HIDE);


		PaintService::DrawAvatar(&dc, theApp.m_userData.pAvatar,
			rectAvt.left, rectAvt.top,
			rectAvt.Width(), rectAvt.Height());
	}

	DrawFriendList(&dc);
}

void CHomeChatDlg::GetFiendList() {
	std::string url = "http://localhost:8888/api/message/list-friend";
	std::string token = theApp.m_userData.token;

	std::string res = ApiService::SendGetRequest(url, token);

	if (res == "") {
		GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Lỗi kết nối mạng"));
	}
	else {
		nlohmann::json jsonRes = nlohmann::json::parse(res);

		if (jsonRes["status"] == 1) {
			for (auto& item : jsonRes["data"]) {
				Friend f;

				f.friendId = item["FriendID"];
				f.fullName = item["FullName"];
				f.isOnline = item["isOnline"];
				
				if (item.contains("Avatar")) {
					f.avatar = item["Avatar"];
				}

				f.pAvatar = FileService::LoadImageFromFile(f.avatar);

				theApp.m_vecFriend.push_back(f);
			}
		}
		else if (jsonRes["status"] == 0) {
			std::string msg = jsonRes["message"];

			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowText(ChangeFormat::UTF8ToCString(msg));
		}
		else {
			std::string msg = jsonRes["error"];

			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowText(ChangeFormat::UTF8ToCString(msg));
		}
	}
}

void CHomeChatDlg::GetUserData() {
	std::string url = "http://localhost:8888/api/user/info";
	std::string token = theApp.m_userData.token;

	std::string res = ApiService::SendGetRequest(url, token);

	if (res == "") {
		GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Lỗi kết nối mạng"));
	}
	else {
		nlohmann::json jsonRes = nlohmann::json::parse(res);
		
		if (jsonRes["status"] == 1) {
			if (jsonRes["data"].contains("Avatar")) {
				theApp.m_userData.avatar = jsonRes["data"]["Avatar"];
			}
			theApp.m_userData.pAvatar = FileService::LoadImageFromFile(theApp.m_userData.avatar);
		}
		else if (jsonRes["status"] == 0) {
			std::string msg = jsonRes["message"];

			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowText(ChangeFormat::UTF8ToCString(msg));
		}
		else {
			std::string msg = jsonRes["error"];

			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowText(ChangeFormat::UTF8ToCString(msg));
		}
	}

}

void CHomeChatDlg::DrawFriendList(CDC* pDC) {
	int nSavedDC = pDC->SaveDC();
	pDC->IntersectClipRect(&m_rectFriendArea);
	pDC->FillSolidRect(&m_rectFriendArea, RGB(255, 255, 255));

	CFont font;
	font.CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	CFont* pOldFont = pDC->SelectObject(&font);

	int itemHeight = 60;
	int currentY = m_rectFriendArea.top + 10 - m_nFriendScrollPos;
	int nTotalHeight = 0; 

	for (const auto& friendObj : m_vecFriendDisplay) {
		if (friendObj.pAvatar != nullptr && !friendObj.pAvatar->IsNull()) {
			friendObj.pAvatar->StretchBlt(pDC->GetSafeHdc(), m_rectFriendArea.left + 10, currentY, 50, 50, SRCCOPY);
		}

		pDC->SetBkMode(TRANSPARENT);
		pDC->TextOut(m_rectFriendArea.left + 75, currentY + 15, ChangeFormat::UTF8ToCString(friendObj.fullName));

		int dotSize = 12;
		int dotX = m_rectFriendArea.left + 10 + 50 - dotSize;
		int dotY = currentY + 50 - dotSize;                  
		PaintService::DrawStatusDot(pDC, dotX, dotY, dotSize, friendObj.isOnline);

		currentY += itemHeight;
		nTotalHeight += itemHeight;
	}

	m_nTotalFriendHeight = nTotalHeight;

	pDC->RestoreDC(nSavedDC);
}

BOOL CHomeChatDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	CPoint ptClient = pt;
	ScreenToClient(&ptClient);

	if (m_rectFriendArea.PtInRect(ptClient)) {
		int scrollStep = 60;
		if (zDelta > 0) m_nFriendScrollPos -= scrollStep; 
		else m_nFriendScrollPos += scrollStep; 

		if (m_nFriendScrollPos < 0) m_nFriendScrollPos = 0;

		int maxScroll = max(0, m_nTotalFriendHeight - m_rectFriendArea.Height());
		if (m_nFriendScrollPos > maxScroll) m_nFriendScrollPos = maxScroll;

		InvalidateRect(m_rectFriendArea);
		return TRUE;
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CHomeChatDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_rectFriendArea.PtInRect(point)) {
		int itemHeight = 60;
		int relativeY = point.y - m_rectFriendArea.top + m_nFriendScrollPos;
		int clickedIndex = relativeY / itemHeight;
		if (clickedIndex >= 0 && clickedIndex < (int)theApp.m_vecFriend.size()) {
			theApp.m_selectedFriend = theApp.m_vecFriend[clickedIndex];
			EndDialog(ID_CHATFRIEND_TRIGGER);
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CHomeChatDlg::OnChangeEditHomechatSearch() {
	CString strSearch;
	GetDlgItemText(IDC_EDIT_HOMECHAT_SEARCH, strSearch);
	strSearch.MakeLower().Trim();

	m_vecFriendDisplay.clear();

	for (const auto& friendObj : theApp.m_vecFriend) {
		CString strName = ChangeFormat::UTF8ToCString(friendObj.fullName);
		strName.MakeLower();

		if (strSearch.IsEmpty() || strName.Find(strSearch) != -1) {
			m_vecFriendDisplay.push_back(friendObj);
		}
	}

	InvalidateRect(&m_rectFriendArea, TRUE);
}