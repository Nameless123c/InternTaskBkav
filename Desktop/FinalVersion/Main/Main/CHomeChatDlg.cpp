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
    ON_WM_CTLCOLOR()
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

    SetBackgroundColor(RGB(255, 255, 255));

    m_fontTitle.CreatePointFont(180, _T("Segoe UI Semibold"));
    CWnd* pTitle = GetDlgItem(IDC_STATIC_HOMECHAT_TITLE);
    if (pTitle) {
        pTitle->SetFont(&m_fontTitle);
    }

    return TRUE;
}

void CHomeChatDlg::OnPaint()
{
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

void CHomeChatDlg::GetFiendList()
{
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
                if (item.contains("Avatar")) f.avatar = item["Avatar"];
                f.pAvatar = FileService::LoadImageFromFile(f.avatar);
                theApp.m_vecFriend.push_back(f);
            }
        }
    }
}

void CHomeChatDlg::GetUserData()
{
    std::string url = "http://localhost:8888/api/user/info";
    std::string token = theApp.m_userData.token;
    std::string res = ApiService::SendGetRequest(url, token);

    if (res != "") {
        nlohmann::json jsonRes = nlohmann::json::parse(res);
        if (jsonRes["status"] == 1) {
            if (jsonRes["data"].contains("Avatar")) theApp.m_userData.avatar = jsonRes["data"]["Avatar"];
            theApp.m_userData.pAvatar = FileService::LoadImageFromFile(theApp.m_userData.avatar);
        }
    }
}

void CHomeChatDlg::DrawFriendList(CDC* pDC)
{
    int nSavedDC = pDC->SaveDC();
    pDC->IntersectClipRect(&m_rectFriendArea);
    pDC->FillSolidRect(&m_rectFriendArea, RGB(255, 255, 255));

    // Tính toán tỉ lệ động dựa trên chiều cao vùng chứa
    int areaHeight = m_rectFriendArea.Height();
    int itemHeight = areaHeight / 2;
    int iconSize = itemHeight - 10;
    int padding = 10;

    CFont font;
    font.CreatePointFont(80, _T("Segoe UI"));
    CFont* pOldFont = pDC->SelectObject(&font);

    int currentY = m_rectFriendArea.top + padding - m_nFriendScrollPos;
    int nTotalHeight = 0;

    for (const auto& friendObj : m_vecFriendDisplay) {
        if (friendObj.pAvatar != nullptr && !friendObj.pAvatar->IsNull()) {
            pDC->SetStretchBltMode(HALFTONE);

            int x = (int)(m_rectFriendArea.left + padding);
            int y = (int)currentY;
            int w = (int)iconSize;
            int h = (int)iconSize;

            friendObj.pAvatar->StretchBlt(pDC->GetSafeHdc(), x, y, w, h, SRCCOPY);
        }

        pDC->SetBkMode(TRANSPARENT);
        pDC->TextOut(m_rectFriendArea.left + iconSize + 20, currentY + (iconSize / 4), ChangeFormat::UTF8ToCString(friendObj.fullName));

        int dotSize = iconSize / 4;
        int dotX = m_rectFriendArea.left + padding + iconSize - dotSize * 1.3;
        int dotY = currentY + iconSize - dotSize * 1.3;
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
        int itemHeight = m_rectFriendArea.Height() / 2;
        int relativeY = point.y - m_rectFriendArea.top + m_nFriendScrollPos;
        int clickedIndex = relativeY / itemHeight;

        if (clickedIndex >= 0 && clickedIndex < (int)m_vecFriendDisplay.size()) {
            theApp.m_selectedFriend = m_vecFriendDisplay[clickedIndex];
            EndDialog(ID_CHATFRIEND_TRIGGER);
        }
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
        CString strName = ChangeFormat::UTF8ToCString(friendObj.fullName);
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