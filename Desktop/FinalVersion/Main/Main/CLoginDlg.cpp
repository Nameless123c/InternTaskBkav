#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CLoginDlg.h"
#include "ApiService.h"
#include "DatabaseService.h"
#include <afxbutton.h>	


IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_LOGIN_SUBMIT, &CLoginDlg::OnBnClickedBtnLoginSubmit)
	ON_STN_CLICKED(IDC_STATIC_LOGIN_GOTO_SIGNUP, &CLoginDlg::OnStnClickedStaticLoginGotoSignup)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_LOGIN_EXIT, &CLoginDlg::OnStnClickedStaticLoginExit)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CLoginDlg::CLoginDlg(CWnd* pParent)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent){

}

CLoginDlg::~CLoginDlg(){

}

void CLoginDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_LOGIN_SUBMIT, m_btnLogin);
}

BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// btn login
	m_btnLogin.m_bDontUseWinXPTheme = TRUE;
	m_btnLogin.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_btnLogin.SetMouseCursorHand();
	m_btnLogin.m_bDrawFocus = FALSE; 
	m_btnLogin.m_bTransparent = FALSE; 
	m_btnLogin.SetFaceColor(RGB(28, 127, 217));
	m_btnLogin.SetTextColor(RGB(255, 255, 255));

	SetBackgroundColor(RGB(255, 255, 255));

	m_fontTitle.CreatePointFont(180, _T("Segoe UI Semibold"));
	CWnd* pTitle = GetDlgItem(IDC_STATIC_LOGIN_TITLE);
	if (pTitle) {
		pTitle->SetFont(&m_fontTitle);
	}

	return TRUE;
}

void CLoginDlg::OnPaint() {
	CPaintDC dc(this);
}


void CLoginDlg::OnBnClickedBtnLoginSubmit(){
	CString username, password;
	
	GetDlgItemText(IDC_EDIT_LOGIN_USERNAME, username);
	GetDlgItemText(IDC_EDIT_LOGIN_PASSWORD, password);

	if (username == _T("")) {
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(_T("Tên đăng nhập không được để trống"));
	}
	else if (password == _T("")) {
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(_T("Mật khẩu không được để trống"));
	}
	else {
		std::string strUsername = CW2A(username.GetString(), CP_UTF8);
		std::string strPassword = CW2A(password.GetString(), CP_UTF8);

		nlohmann::json data;
		data["Username"] = strUsername;
		data["Password"] = strPassword;

		std::string url = "http://localhost:8888/api/auth/login";

		std::string strRes = ApiService::SendPostRequest(url, data, "");

		if (strRes == "") {
			GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
		}
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(strRes);

			if (jsonRes["status"] == 1) {
				// session
				theApp.m_userData.username = jsonRes["data"]["Username"];
				theApp.m_userData.fullName = jsonRes["data"]["FullName"];
				theApp.m_userData.userId = jsonRes["data"]["userId"];

				if (jsonRes["data"].contains("Avatar")) {
					theApp.m_userData.avatar = jsonRes["data"]["Avatar"];
				}

				theApp.m_userData.token = jsonRes["data"]["token"];

				// db
				std::string username = theApp.m_userData.username;
				sqlite3_stmt* stmt;
				std::string sqlCheck = "SELECT COUNT(*) FROM Users WHERE username = ?;";

				if (sqlite3_prepare_v2(DatabaseService::m_db, sqlCheck.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
					sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

					if (sqlite3_step(stmt) == SQLITE_ROW) {
						int count = sqlite3_column_int(stmt, 0);

						if (count == 0) {
							std::string sqlInsert = "INSERT INTO Users (userId, username, fullName, avatar, password) VALUES (?, ?, ?, ?, ?);";
							sqlite3_stmt* stmtInsert;

							if (sqlite3_prepare_v2(DatabaseService::m_db, sqlInsert.c_str(), -1, &stmtInsert, nullptr) == SQLITE_OK) {
								sqlite3_bind_text(stmtInsert, 1, theApp.m_userData.userId.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 2, theApp.m_userData.username.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 3, theApp.m_userData.fullName.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 4, theApp.m_userData.avatar.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 5, strPassword.c_str(), -1, SQLITE_TRANSIENT); // Sửa thành index 5

								sqlite3_step(stmtInsert);
								sqlite3_finalize(stmtInsert);
							}
						}
					}
					sqlite3_finalize(stmt);
				}

				EndDialog(ID_HOMECHAT_TRIGGER);
			}
			else if (jsonRes["status"] == 0) {
				std::string msg = jsonRes["message"];

				if (msg == "Username not found" || msg == "Incorrect password") {
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(_T("Bạn nhập sai tên đăng nhập và mật khẩu!"));
				}
				else {
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));
				}
			}
		}
	}
}

void CLoginDlg::OnStnClickedStaticLoginGotoSignup(){
	EndDialog(ID_SIGNUP_TRIGGER);
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_LOGIN_ERROR) {
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_LOGIN_GOTO_SIGNUP) {
		pDC->SetTextColor(RGB(4, 125, 231));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_LOGIN_TITLE) {
		pDC->SetTextColor(RGB(20, 106, 224));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return hbr;
}

void CLoginDlg::OnStnClickedStaticLoginExit(){
	EndDialog(ID_EXIT_TRIGGER);
}

LRESULT CLoginDlg::OnNcHitTest(CPoint point){
	LRESULT hit = CDialogEx::OnNcHitTest(point);

	if (hit == HTCLIENT) {
		return HTCAPTION;
	}

	return hit;
}
