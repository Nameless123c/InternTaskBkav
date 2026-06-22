#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CUserDlg.h"
#include "PaintService.h"
#include "CHomeChatDlg.h"


// CUserDlg dialog

BEGIN_MESSAGE_MAP(CUserDlg, CDialogEx)
	ON_WM_PAINT()
    ON_STN_CLICKED(IDC_STATIC_USER_EXIT, &CUserDlg::OnStnClickedStaticUserExit)
    ON_WM_NCHITTEST()
    ON_BN_CLICKED(IDC_BTN_USER_LOGOUT, &CUserDlg::OnBnClickedBtnUserLogout)
    ON_WM_CTLCOLOR()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CUserDlg, CDialogEx)

CUserDlg::CUserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USER_DIALOG, pParent)
{

}

CUserDlg::~CUserDlg()
{
}

void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_USER_LOGOUT, m_btnLogout);
}


BOOL CUserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // btn login
    m_btnLogout.m_bDontUseWinXPTheme = TRUE;
    m_btnLogout.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
    m_btnLogout.SetMouseCursorHand();
    m_btnLogout.m_bDrawFocus = FALSE;
    m_btnLogout.m_bTransparent = FALSE;
    m_btnLogout.SetFaceColor(RGB(28, 127, 217));
    m_btnLogout.SetTextColor(RGB(255, 255, 255));

    SetBackgroundColor(RGB(255, 255, 255));

    m_fontTitle.CreatePointFont(120, _T("Segoe UI Semibold"));

    GetDlgItem(IDC_STATIC_USER_TITLE)->SetFont(&m_fontTitle);

	return TRUE;
}

void CUserDlg::OnPaint()
{
	CPaintDC dc(this);
    PaintService::DrawAppBar(this, IDC_STATIC_USER_APPBAR, &dc);

    CWnd* pArea = GetDlgItem(IDC_STATIC_USER_AVT);
    if (pArea) {
        CRect rectAvt;
        pArea->GetWindowRect(&rectAvt);
        ScreenToClient(&rectAvt);
        pArea->ShowWindow(SW_HIDE);

        PaintService::DrawAvatar(&dc, theApp.m_userData.pAvatar,
            rectAvt.left, rectAvt.top,
            rectAvt.Width(), rectAvt.Height());
    }
}

void CUserDlg::OnStnClickedStaticUserExit()
{
    SendMessage(WM_CLOSE);
    DestroyWindow();
}

LRESULT CUserDlg::OnNcHitTest(CPoint point)
{
    LRESULT hit = CDialogEx::OnNcHitTest(point);

    if (hit == HTCLIENT) {
        return HTCAPTION;
    }

    return hit;
}

void CUserDlg::OnBnClickedBtnUserLogout(){
    GetParent()->PostMessage(WM_USER_LOGOUT_SIGNAL, 0, 0);
    DestroyWindow();
}

HBRUSH CUserDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_STATIC_USER_TITLE || pWnd->GetDlgCtrlID() == IDC_STATIC_USER_EXIT) {
        pDC->SetTextColor(RGB(0, 0, 0));

        pDC->SetBkColor(RGB(217, 217, 217));

        static CBrush brush(RGB(217, 217, 217));
        return (HBRUSH)brush.GetSafeHandle();
    }

    return hbr;
}

void CUserDlg::OnClose() {
    // Logic reset của bạn ở đây
    CHomeChatDlg* pParent = (CHomeChatDlg*)GetParent();
    if (pParent != nullptr) {
        pParent->m_pUserDlg = nullptr;
        pParent->m_isUserCreate = false;
    }
    
    CDialogEx::OnClose();
}