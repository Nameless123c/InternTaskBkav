#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CSignupDlg.h"
#include "ChangeFormat.h"
#include "ApiService.h"
#include "DatabaseService.h"	

IMPLEMENT_DYNAMIC(CSignupDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSignupDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SIGNUP_SUBMIT, &CSignupDlg::OnBnClickedButtonSignupSubmit)
END_MESSAGE_MAP()

CSignupDlg::CSignupDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIGNUP_DIALOG, pParent)
{

}

CSignupDlg::~CSignupDlg(){

}

void CSignupDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
}




void CSignupDlg::OnBnClickedButtonSignupSubmit(){
	CString fullName, username, password, rePass;

	GetDlgItemText(IDC_EDIT_SIGNUP_FULLNAME, fullName);
	GetDlgItemText(IDC_EDIT_SIGNUP_USERNAME, username);
	GetDlgItemText(IDC_EDIT_SIGNUP_PASSWORD, password);
	GetDlgItemText(IDC_EDIT_SIGNUP_REPASS, rePass);

	if (fullName == _T("") || username == _T("") || password == _T("") || rePass == _T("")) {
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Thông tin không được để trống"));
	}
	else if (password != rePass) {
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Mật khẩu không khớp!"));
	}
	else {
		std::string url = "http://localhost:8888/api/auth/register";

		std::string strFullName = ChangeFormat::CStringToUTF8(fullName);
		std::string strUsername = ChangeFormat::CStringToUTF8(username);
		std::string strPassword = ChangeFormat::CStringToUTF8(password);

		nlohmann::json data;
		data["FullName"] = strFullName;
		data["Username"] = strUsername;
		data["Password"] = strPassword;

		std::string res = ApiService::SendPostRequest(url, data, "");

		if (res == "") {
			GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Lỗi kết nối mạng"));
		} 
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(res);

			if (jsonRes["status"] == 1) {

				std::string sql = "INSERT INTO Users (username, fullName, password) VALUES (?, ?, ?);";
				sqlite3_stmt* stmt;
				if (sqlite3_prepare_v2(DatabaseService::m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
					sqlite3_bind_text(stmt, 1, strFullName.c_str(), -1, SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 2, strUsername.c_str(), -1, SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 3, strPassword.c_str(), -1, SQLITE_TRANSIENT);

					sqlite3_step(stmt);
					sqlite3_finalize(stmt);
				}

				EndDialog(ID_LOGIN_TRIGGER);
			}
			else {
				std::string msg = jsonRes["message"];

				if (msg == "Username already exists") {
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Tài khoản đã tồn tại!"));
				}
				else {
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_SIGNUP_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString(msg));
				}
			}
		}
	}

}
