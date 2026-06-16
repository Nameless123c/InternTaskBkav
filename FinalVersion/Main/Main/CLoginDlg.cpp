#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CLoginDlg.h"
#include "ApiService.h"
#include "ChangeFormat.h"
#include "PaintService.h"
#include "DatabaseService.h"


IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_LOGIN_SUBMIT, &CLoginDlg::OnBnClickedBtnLoginSubmit)
	ON_STN_CLICKED(IDC_STATIC_LOGIN_GOTO_SIGNUP, &CLoginDlg::OnStnClickedStaticLoginGotoSignup)
END_MESSAGE_MAP()

CLoginDlg::CLoginDlg(CWnd* pParent)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent){

}

CLoginDlg::~CLoginDlg(){

}

void CLoginDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
}

void CLoginDlg::OnBnClickedBtnLoginSubmit(){
	CString username, password;
	
	GetDlgItemText(IDC_EDIT_LOGIN_USERNAME, username);
	GetDlgItemText(IDC_EDIT_LOGIN_PASSWORD, password);

	if (username == _T("")) {
		PaintService::DrawErrorMessage(this, IDC_STATIC_LOGIN_ERROR, ChangeFormat::UTF8ToCString("Tên đăng nhập không được để không được để trống"));
	}
	else if (password == _T("")) {
		PaintService::DrawErrorMessage(this, IDC_STATIC_LOGIN_ERROR, ChangeFormat::UTF8ToCString("Mật khẩu không được để không được để trống"));
	}
	else {
		std::string strUsername = ChangeFormat::CStringToUTF8(username);
		std::string strPassword = ChangeFormat::CStringToUTF8(password);

		nlohmann::json data;
		data["Username"] = strUsername;
		data["Password"] = strPassword;

		std::string url = "http://localhost:8888/api/auth/login";

		std::string strRes = ApiService::SendPostRequest(url, data, "");

		if (strRes == "") {
			GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Lỗi kết nối mạng"));
		}
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(strRes);

			if (jsonRes["status"] == 1) {
				// session
				theApp.m_userData.username = jsonRes["data"]["Username"];
				theApp.m_userData.fullName = jsonRes["data"]["FullName"];

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

						// Nếu chưa tồn tại, thực hiện Insert
						if (count == 0) {
							std::string sqlInsert = "INSERT INTO Users (username, fullName, avatar, password) VALUES (?, ?, ?, ?);";
							sqlite3_stmt* stmtInsert;

							if (sqlite3_prepare_v2(DatabaseService::m_db, sqlInsert.c_str(), -1, &stmtInsert, nullptr) == SQLITE_OK) {
								sqlite3_bind_text(stmtInsert, 1, theApp.m_userData.username.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 2, theApp.m_userData.fullName.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 3, theApp.m_userData.avatar.c_str(), -1, SQLITE_TRANSIENT);
								sqlite3_bind_text(stmtInsert, 3, strPassword.c_str(), -1, SQLITE_TRANSIENT);

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
					PaintService::DrawErrorMessage(this, IDC_STATIC_LOGIN_ERROR, ChangeFormat::UTF8ToCString("Bạn nhập sai tên tài khoản hoặc mật khẩu!"));
				}
				else {
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_STATIC_LOGIN_ERROR)->SetWindowTextW(CA2W(msg.c_str()));
				}
			}
		}
	}
}

void CLoginDlg::OnStnClickedStaticLoginGotoSignup(){
	EndDialog(ID_SIGNUP_TRIGGER);
}
