#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CHomeChatDlg.h"
#include "ApiService.h"
#include "Friend.h"
#include "FileService.h"
#include "PaintService.h"
#include "DatabaseService.h"
#include "Nickname.h"


IMPLEMENT_DYNAMIC(CHomeChatDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CHomeChatDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_MOUSEWHEEL()
    ON_WM_LBUTTONDOWN()
    ON_EN_CHANGE(IDC_EDIT_HOMECHAT_SEARCH, &CHomeChatDlg::OnChangeEditHomechatSearch)
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_NCHITTEST()
    ON_MESSAGE(WM_USER_LOGOUT_SIGNAL, &CHomeChatDlg::OnLogoutSignal)
END_MESSAGE_MAP()

CHomeChatDlg::CHomeChatDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_HOMECHAT_DIALOG, pParent)
{
}

CHomeChatDlg::~CHomeChatDlg()
{
}

void CHomeChatDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CHomeChatDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (GetFiendList() && GetNickname()) {
        theApp.m_vecFriend = m_tempFriendList;
        theApp.m_mapNickname = m_tempNickname;
    }
    
    GetUserData();
    m_vecFriendDisplay = theApp.m_vecFriend;
    

    CWnd* pArea = GetDlgItem(IDC_STATIC_HOMECHAT_FRIENDAREA);
    if (pArea) {
        pArea->GetWindowRect(&m_rectFriendArea);
        ScreenToClient(&m_rectFriendArea);
        pArea->ShowWindow(SW_HIDE);
    }

    CWnd* pAvatar = GetDlgItem(IDC_STATIC_HOMECHAT_AVT);
    if (pAvatar) {
        pAvatar->GetWindowRect(&m_rectUserAvatar);
        ScreenToClient(&m_rectUserAvatar);
        pAvatar->ShowWindow(SW_HIDE);
    }

    m_nFriendScrollPos = 0;
    m_nTotalFriendHeight = 0;

    SetBackgroundColor(RGB(255, 255, 255));

    m_fontTitle.CreatePointFont(180, _T("Segoe UI Semibold"));
    CWnd* pTitle = GetDlgItem(IDC_STATIC_HOMECHAT_TITLE);
    if (pTitle) {
        pTitle->SetFont(&m_fontTitle);
    }

    SetTimer(2, 10000, NULL);

    return TRUE;
}

void CHomeChatDlg::OnPaint()
{
    CPaintDC dc(this);
    GetDlgItem(IDC_STATIC_HOMECHAT_FULLNAME)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_HOMECHAT_FULLNAME)->SetWindowText(CA2W(theApp.m_userData.fullName.c_str(), CP_UTF8));

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HOMECHAT_SEARCH);
    if (pEdit) {
        pEdit->SetCueBanner(_T("Tìm kiếm"));
    }

    if (theApp.m_userData.pAvatar != nullptr && !theApp.m_userData.pAvatar->IsNull()) {
        PaintService::DrawAvatar(&dc, theApp.m_userData.pAvatar,
            m_rectUserAvatar.left,
            m_rectUserAvatar.top,
            m_rectUserAvatar.Width(),
            m_rectUserAvatar.Height());
    }

    DrawFriendList(&dc);
}

bool CHomeChatDlg::GetFiendList()
{
    std::string url = "http://localhost:8888/api/message/list-friend";
    std::string token = theApp.m_userData.token;
    std::string res = ApiService::SendGetRequest(url, token);

    if (res == "") {
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
        return false;
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
                    f.avatar = f.avatar.substr(1);
                }

                f.pAvatar = FileService::LoadImageFromFile(f.avatar);
                m_tempFriendList.push_back(f);
                DatabaseService::SyncFriendsToDB(m_tempFriendList);
            }
            DatabaseService::SyncFriendsToDB(m_tempFriendList);
            return true;
        }
    }
    return false;
}

void CHomeChatDlg::GetUserData()
{
    std::string url = "http://localhost:8888/api/user/info";
    std::string token = theApp.m_userData.token;
    std::string res = ApiService::SendGetRequest(url, token);

    if (res != "") {
        nlohmann::json jsonRes = nlohmann::json::parse(res);
        if (jsonRes["status"] == 1) {
            if (jsonRes["data"].contains("Avatar") && !jsonRes["data"]["Avatar"].is_null()) {
                theApp.m_userData.avatar = jsonRes["data"]["Avatar"];
                theApp.m_userData.avatar = theApp.m_userData.avatar.substr(1);

                std::string fullUrl = "http://localhost:8888/api/images/" + theApp.m_userData.avatar;
                std::string localSavePath = theApp.m_userData.avatar;
                std::string token = theApp.m_userData.token;

                bool ok = ApiService::DownloadFile(fullUrl, localSavePath, token);
            }

            theApp.m_userData.pAvatar = FileService::LoadImageFromFile(theApp.m_userData.avatar);

            std::string sqlUpdate = "UPDATE Users SET avatar = ? WHERE userId = ?;";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(DatabaseService::m_db, sqlUpdate.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, theApp.m_userData.avatar.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, theApp.m_userData.userId.c_str(), -1, SQLITE_TRANSIENT);

                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
        else if (jsonRes["status"] == 0) {
            nlohmann::json jsonRes = nlohmann::json::parse(res);

            std::string msg = jsonRes["message"];
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));
        }
    }
}

bool CHomeChatDlg::GetNickname() {
    std::string url = "http://localhost:8888/api/user/nickname/list";
    std::string token = theApp.m_userData.token;

    std::string res = ApiService::SendGetRequest(url, token);

    if (res == "") {
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(_T("Lỗi kết nối mạng"));
        return false;
    }
    else {
        GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_HIDE);
        nlohmann::json jsonRes = nlohmann::json::parse(res);
        if (jsonRes["status"] == 1) {
            if (jsonRes.contains("data") && jsonRes["data"].is_array()) {
                m_tempNickname.clear();

                for (const auto& item : jsonRes["data"]) {
                    std::string friendId = item.value("FriendID", "");

                    if (!friendId.empty()) {
                        NicknameInfo info;
                        info.id = item.value("_id", "");
                        info.userId = item.value("UserID", "");
                        info.friendId = friendId;
                        info.nickname = item.value("Nickname", "");

                        DatabaseService::SaveNicknameToDB(info);
                        m_tempNickname[friendId] = info;
                    }
                }

                return true;
            }
        }
        else if (jsonRes["status"] == 0) {
            nlohmann::json jsonRes = nlohmann::json::parse(res);

            std::string msg = jsonRes["message"];
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
            GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(CA2W(msg.c_str(), CP_UTF8));

            return false;
        }
        return false;
    }
    return false;
}

void CHomeChatDlg::DrawFriendList(CDC* pDC)
{
    int nSavedDC = pDC->SaveDC();
    pDC->IntersectClipRect(&m_rectFriendArea);
    pDC->FillSolidRect(&m_rectFriendArea, RGB(255, 255, 255));

    // Tính toán tỉ lệ động dựa trên chiều cao vùng chứa
    int areaHeight = m_rectFriendArea.Height();
    int itemHeight = areaHeight / 3;
    int iconSize = itemHeight - 5;
    int padding = 10;

    CFont font;
    font.CreatePointFont(80, _T("Segoe UI"));
    CFont* pOldFont = pDC->SelectObject(&font);

    int currentY = m_rectFriendArea.top + padding - m_nFriendScrollPos;
    int nTotalHeight = 0;

    for (const auto& friendObj : m_vecFriendDisplay) {
        if (friendObj.pAvatar != nullptr && !friendObj.pAvatar->IsNull()) {
            pDC->SetStretchBltMode(HALFTONE);

            int x = m_rectFriendArea.left + padding;
            int y = currentY;
            int w = iconSize;
            int h = iconSize;

            PaintService::DrawAvatar(pDC, friendObj.pAvatar, x, y, w, h);
        }

        pDC->SetBkMode(TRANSPARENT);

        CString strDisplayName;
        auto it = theApp.m_mapNickname.find(friendObj.friendId);
        if (it != theApp.m_mapNickname.end()) {
            strDisplayName = CA2W(it->second.nickname.c_str(), CP_UTF8);
        }
        else {
            strDisplayName = CA2W(friendObj.fullName.c_str(), CP_UTF8);
        }

        pDC->TextOut((int)(m_rectFriendArea.left + iconSize + 20),(int)(currentY + (iconSize / 4)), strDisplayName);

        int dotSize = iconSize / 4;
        int dotX = m_rectFriendArea.left + padding + iconSize - static_cast<int>(dotSize * 1.3);
        int dotY = currentY + iconSize - static_cast<int>(dotSize * 1.3);
        PaintService::DrawStatusDot(pDC, dotX, dotY, dotSize, friendObj.isOnline);

        currentY += itemHeight;
        nTotalHeight += itemHeight;
    }

    m_nTotalFriendHeight = nTotalHeight;
    pDC->RestoreDC(nSavedDC);
}

BOOL CHomeChatDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CPoint ptClient = pt;
    ScreenToClient(&ptClient);

    if (m_rectFriendArea.PtInRect(ptClient)) {
        int scrollStep = m_rectFriendArea.Height() / 2;
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

void CHomeChatDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_rectFriendArea.PtInRect(point)) {
        int itemHeight = m_rectFriendArea.Height() / 3;
        int relativeY = point.y - m_rectFriendArea.top + m_nFriendScrollPos;
        int clickedIndex = relativeY / itemHeight;

        if (clickedIndex >= 0 && clickedIndex < (int)m_vecFriendDisplay.size()) {
            Friend selectedFriend = m_vecFriendDisplay[clickedIndex];
            auto it = m_mapChatWindows.find(selectedFriend.friendId);

            if (it == m_mapChatWindows.end()) {
                CChatFriendDlg* pNewChat = new CChatFriendDlg();

                pNewChat->m_currentFriend = selectedFriend;

                pNewChat->Create(IDD_CHATFRIEND_DIALOG, this);

                m_mapChatWindows[selectedFriend.friendId] = pNewChat;
                

                it = m_mapChatWindows.find(selectedFriend.friendId);
            }

            it->second->ShowWindow(SW_SHOW);
            it->second->BringWindowToTop();
            it->second->GetMessage();
        }
    }

    if (m_rectUserAvatar.PtInRect(point)) {
        if (!m_isUserCreate) { 
            m_pUserDlg = new CUserDlg();
            if (m_pUserDlg->Create(IDD_USER_DIALOG, this)) {
                m_pUserDlg->ShowWindow(SW_SHOW);
                m_isUserCreate = true;
            }
        }
        return;
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}


void CHomeChatDlg::OnChangeEditHomechatSearch()
{
    CString strSearch;
    GetDlgItemText(IDC_EDIT_HOMECHAT_SEARCH, strSearch);
    strSearch.MakeLower().Trim();

    m_vecFriendDisplay.clear();
    for (const auto& friendObj : theApp.m_vecFriend) {
        CString strName = CA2W(friendObj.fullName.c_str());
        strName.MakeLower();
        if (strSearch.IsEmpty() || strName.Find(strSearch) != -1) {
            m_vecFriendDisplay.push_back(friendObj);
        }
    }
    InvalidateRect(&m_rectFriendArea, TRUE);
}

HBRUSH CHomeChatDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_EDIT_HOMECHAT_SEARCH){
        pDC->SetTextColor(RGB(0, 0, 0));

        pDC->SetBkColor(RGB(250, 250, 250)); 

        static CBrush brush(RGB(250, 250, 250));
        return (HBRUSH)brush.GetSafeHandle();
    }

    if (pWnd->GetDlgCtrlID() == IDC_STATIC_HOMECHAT_TITLE) {
        pDC->SetTextColor(RGB(20, 106, 224));
        pDC->SetBkMode(TRANSPARENT);        
        return (HBRUSH)GetStockObject(NULL_BRUSH); 
    }

    return hbr;
}

void CHomeChatDlg::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == 2) {
        m_tempNickname.clear();
        m_tempFriendList.clear();

        if (GetFiendList() && GetNickname()) {
            theApp.m_vecFriend = m_tempFriendList;
            theApp.m_mapNickname = m_tempNickname;

            m_vecFriendDisplay = theApp.m_vecFriend;

            InvalidateRect(&m_rectFriendArea, TRUE);
        }
    }
    CDialogEx::OnTimer(nIDEvent);
}

LRESULT CHomeChatDlg::OnNcHitTest(CPoint point){
    CPoint pt = point;
    ScreenToClient(&pt);

    if (m_rectFriendArea.PtInRect(pt)) {
        return HTCLIENT;
    }

    if (m_rectUserAvatar.PtInRect(pt)) {
        return HTCLIENT;
    }

    return HTCAPTION;
}

LRESULT CHomeChatDlg::OnLogoutSignal(WPARAM wParam, LPARAM lParam) {
    // 1. Gọi hàm dọn dẹp dữ liệu trong App
    theApp.Logout();

    // 2. Đóng tất cả các cửa sổ chat con đang mở (để tránh lỗi truy cập bộ nhớ sau khi logout)
    for (auto& pair : m_mapChatWindows) {
        if (pair.second && ::IsWindow(pair.second->GetSafeHwnd())) {
            pair.second->DestroyWindow();
            delete pair.second;
        }
    }
    m_mapChatWindows.clear();

    // 3. Đóng HomeChat và trả về mã tín hiệu cho vòng lặp của App
    EndDialog(ID_LOGIN_TRIGGER);
    return 0;
}