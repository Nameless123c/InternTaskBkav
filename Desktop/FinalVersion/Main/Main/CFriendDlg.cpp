#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CFriendDlg.h"
#include "PaintService.h"
#include "CChatFriendDlg.h"
#include "FileService.h"
#include "nlohmann/json.hpp"
#include "ApiService.h"


IMPLEMENT_DYNAMIC(CFriendDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CFriendDlg, CDialogEx)
	ON_WM_PAINT()
    ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_FRIEND_SUBMIT, &CFriendDlg::OnBnClickedBtnFriendSubmit)
	ON_STN_CLICKED(IDC_STATIC_FRIEND_EXIT, &CFriendDlg::OnStnClickedStaticFriendExit)
	ON_WM_NCHITTEST()
	ON_STN_CLICKED(IDC_STATIC_FRIEND_DELETE, &CFriendDlg::OnStnClickedStaticFriendDelete)
END_MESSAGE_MAP()

CFriendDlg::CFriendDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRIEND_DIALOG, pParent)
{

}

CFriendDlg::~CFriendDlg()
{
}

void CFriendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_FRIEND_SUBMIT, m_btnNickName);
}

BOOL CFriendDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    SetBackgroundColor(RGB(255, 255, 255));

	// btn NickName
	m_btnNickName.m_bDontUseWinXPTheme = TRUE;
	m_btnNickName.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_btnNickName.SetMouseCursorHand();
	m_btnNickName.m_bDrawFocus = FALSE;
	m_btnNickName.m_bTransparent = FALSE;
	m_btnNickName.SetFaceColor(RGB(28, 127, 217));
	m_btnNickName.SetTextColor(RGB(255, 255, 255));

	// tên
	if (m_fontTitle.GetSafeHandle() != nullptr) {
		m_fontTitle.DeleteObject(); 
	}
	m_fontTitle.CreatePointFont(120, _T("Segoe UI Semibold"));

	GetDlgItem(IDC_STATIC_FRIEND_NICKNAME)->SetFont(&m_fontTitle);
	GetDlgItem(IDC_STATIC_FRIEND_TITLE)->SetFont(&m_fontTitle);

    m_friendData.pAvatar = FileService::LoadImageFromFile(m_friendData.avatar);

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FRIEND_NICKNAME);
    if (pEdit) {
        pEdit->SetCueBanner(_T("Nhập biệt danh..."));
    }

	return TRUE;
}

void CFriendDlg::OnPaint()
{
	CPaintDC dc(this);
	PaintService::DrawAppBar(this, IDC_STATIC_FRIEND_APPBAR, &dc);

    CWnd* pArea = GetDlgItem(IDC_STATIC_FRIEND_AVT);
    if (pArea) {
        CRect rectAvt;
        pArea->GetWindowRect(&rectAvt);
        ScreenToClient(&rectAvt);
        pArea->ShowWindow(SW_HIDE);

        PaintService::DrawAvatar(&dc, m_friendData.pAvatar,
            rectAvt.left, rectAvt.top,
            rectAvt.Width(), rectAvt.Height());
    }

	// hiển thị nickname
	GetDlgItem(IDC_STATIC_FRIEND_NICKNAME)->ShowWindow(SW_SHOW);

	std::string friendId = m_friendData.friendId;
	CString strDisplayName = CA2W(m_friendData.fullName.c_str(), CP_UTF8);
	auto it = theApp.m_mapNickname.find(friendId);
	if (it != theApp.m_mapNickname.end()) {
		strDisplayName = CA2W(it->second.nickname.c_str(), CP_UTF8);
	}

	GetDlgItem(IDC_STATIC_FRIEND_NICKNAME)->SetWindowText(strDisplayName);
}

void CFriendDlg::OnBnClickedBtnFriendSubmit()
{
	CString nickName;
	GetDlgItemText(IDC_EDIT_FRIEND_NICKNAME, nickName);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FRIEND_NICKNAME);
	if (pEdit) pEdit->SetWindowText(_T(""));

	if (nickName == _T("")) {
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->SetWindowTextW(_T("Bạn chưa nhập biệt danh cho đối phương!"));
	}
	else {
		std::string url = "http://localhost:8888/api/user/nickname/update";

		std::string FriendID = m_friendData.friendId;
		std::string Nickname = CW2A(nickName.GetString(), CP_UTF8);

		nlohmann::json data;

		data["FriendID"] = FriendID;
		data["Nickname"] = Nickname;

		std::string res = ApiService::SendPostRequest(url, data, theApp.m_userData.token);

		if (res == "") {
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
		}
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(res);

			std::string msg = jsonRes["message"];
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FRIEND_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));
			
		}
	}
}


HBRUSH CFriendDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_EDIT_FRIEND_NICKNAME) {
		pDC->SetTextColor(RGB(0, 0, 0));

		pDC->SetBkColor(RGB(235, 233, 239));

		static CBrush brush(RGB(235, 233, 239));
		return (HBRUSH)brush.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_FRIEND_ERROR) {
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_FRIEND_DELETE) {
		pDC->SetTextColor(RGB(4, 125, 231));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_FRIEND_TITLE) {
		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(217, 217, 217)); // #D9D9D9
		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_FRIEND_EXIT) {
		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(217, 217, 217));
		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}

	return hbr;
}


void CFriendDlg::OnStnClickedStaticFriendExit(){
	EndDialog(ID_EXIT_TRIGGER);
}



LRESULT CFriendDlg::OnNcHitTest(CPoint point){
	LRESULT hit = CDialogEx::OnNcHitTest(point);

	if (hit == HTCLIENT) {
		return HTCAPTION;
	}

	return hit;
}

void CFriendDlg::OnStnClickedStaticFriendDelete()
{
	std::string url = "http://localhost:8888/api/user/nickname/delete";

	std::string FriendID = m_friendData.friendId;

	nlohmann::json data;

	data["FriendID"] = FriendID;

	std::string res = ApiService::SendPostRequest(url, data, theApp.m_userData.token);

	if (res == "") {
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
	}
	else {
		nlohmann::json jsonRes = nlohmann::json::parse(res);

		std::string msg = jsonRes["message"];
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FRIEND_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));
	}
}
