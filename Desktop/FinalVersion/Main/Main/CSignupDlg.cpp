#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CSignupDlg.h"
#include "ApiService.h"
#include "DatabaseService.h"	
#include "PaintService.h"

IMPLEMENT_DYNAMIC(CSignupDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSignupDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SIGNUP_SUBMIT, &CSignupDlg::OnBnClickedButtonSignupSubmit)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_STN_CLICKED(IDC_STATIC_SIGNUP_GOTO_LOGIN, &CSignupDlg::OnStnClickedStaticSignupGotoLogin)
	ON_STN_CLICKED(IDC_STATIC_SIGNUP_EXIT, &CSignupDlg::OnStnClickedStaticSignupExit)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CSignupDlg::CSignupDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIGNUP_DIALOG, pParent)
{

}

CSignupDlg::~CSignupDlg(){

}

void CSignupDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_SIGNUP_SUBMIT, m_btnSignup);
}

BOOL CSignupDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	SetBackgroundColor(RGB(255, 255, 255));

	m_fontTitle.CreatePointFont(120, _T("Segoe UI Semibold"));

	GetDlgItem(IDC_STATIC_SIGNUP_TITLE)->SetFont(&m_fontTitle);


	m_btnSignup.m_bDontUseWinXPTheme = TRUE;
	m_btnSignup.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_btnSignup.SetMouseCursorHand();
	m_btnSignup.m_bDrawFocus = FALSE;
	m_btnSignup.m_bTransparent = FALSE;
	m_btnSignup.SetFaceColor(RGB(28, 127, 217));
	m_btnSignup.SetTextColor(RGB(255, 255, 255));

	return TRUE;
}

void CSignupDlg::OnPaint() {
	CPaintDC dc(this);
	PaintService::DrawAppBar(this, IDC_STATIC_SIGNUP_APPBAR, &dc);
}

void CSignupDlg::OnBnClickedButtonSignupSubmit(){
	CString fullName, username, password, rePass;

	GetDlgItemText(IDC_EDIT_SIGNUP_FULLNAME, fullName);
	GetDlgItemText(IDC_EDIT_SIGNUP_USERNAME, username);
	GetDlgItemText(IDC_EDIT_SIGNUP_PASSWORD, password);
	GetDlgItemText(IDC_EDIT_SIGNUP_REPASS, rePass);

	if (fullName == _T("") || username == _T("") || password == _T("") || rePass == _T("")) {
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(_T("Thông tin không được để trống"));
	}
	else if (password != rePass) {
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(_T("Mật khẩu không khớp!"));
	}
	else {
		std::string url = "http://localhost:8888/api/auth/register";

		std::string strFullName = CW2A(fullName.GetString(), CP_UTF8);
		std::string strUsername = CW2A(username.GetString(), CP_UTF8);
		std::string strPassword = CW2A(password.GetString(), CP_UTF8);

		nlohmann::json data;
		data["FullName"] = strFullName;
		data["Username"] = strUsername;
		data["Password"] = strPassword;

		std::string res = ApiService::SendPostRequest(url, data, "");

		if (res == "") {
			GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
		} 
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(res);
			std::string strUserId = jsonRes["userId"];

			if (jsonRes["status"] == 1) {

				// lưu db
				std::string sql = "INSERT INTO Users (userId, username, fullName, password) VALUES (?, ?, ?, ?);";
				sqlite3_stmt* stmt;

				if (sqlite3_prepare_v2(DatabaseService::m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
					// 2. Gán giá trị theo thứ tự (userId là index 1)
					sqlite3_bind_text(stmt, 1, strUserId.c_str(), -1, SQLITE_TRANSIENT);   // userId (lấy từ JSON response)
					sqlite3_bind_text(stmt, 2, strUsername.c_str(), -1, SQLITE_TRANSIENT); // username
					sqlite3_bind_text(stmt, 3, strFullName.c_str(), -1, SQLITE_TRANSIENT); // fullName
					sqlite3_bind_text(stmt, 4, strPassword.c_str(), -1, SQLITE_TRANSIENT); // password

					sqlite3_step(stmt);
					sqlite3_finalize(stmt);
				}

				EndDialog(ID_LOGIN_TRIGGER);
			}
			else {
				std::string msg = jsonRes["message"];

				if (msg == "Username already exists") {
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(_T("Tài khoản đã tồn tại!"));
				}
				else {
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));
				}
			}
		}
	}

}

HBRUSH CSignupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_SIGNUP_ERROR) {
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_SIGNUP_TITLE) {
		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(217, 217, 217)); // #D9D9D9
		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_SIGNUP_GOTO_LOGIN) {
		pDC->SetTextColor(RGB(4, 125, 231));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_SIGNUP_EXIT) {
		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(217, 217, 217));
		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}
	
	return hbr;
}



void CSignupDlg::OnStnClickedStaticSignupGotoLogin(){
	EndDialog(ID_LOGIN_TRIGGER);
}

void CSignupDlg::OnStnClickedStaticSignupExit(){
	EndDialog(ID_EXIT_TRIGGER);
}

LRESULT CSignupDlg::OnNcHitTest(CPoint point){
	LRESULT hit = CDialogEx::OnNcHitTest(point);

	if (hit == HTCLIENT) {
		return HTCAPTION;
	}

	return hit;
}
