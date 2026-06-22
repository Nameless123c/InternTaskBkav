#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CChatFriendDlg.h"
#include "ApiService.h"
#include "ChangeFormat.h"
#include "FileService.h"
#include "PaintService.h"
#include "DatabaseService.h"

IMPLEMENT_DYNAMIC(CChatFriendDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CChatFriendDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_CHATFRIEND_FULLNAME, &CChatFriendDlg::OnStnClickedStaticChatfriendFullname)
	ON_WM_CLOSE()
	ON_STN_CLICKED(IDC_STATIC_CHATFRIEND_EXIT, &CChatFriendDlg::OnStnClickedStaticChatfriendExit)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CChatFriendDlg::CChatFriendDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATFRIEND_DIALOG, pParent)
	, m_nScrollPos(0)
	, m_nTotalHeight(0)
	, m_pImgSend(nullptr)
	, m_pImgEmoji(nullptr)
	, m_pImgImage(nullptr)
	, m_pImgAttach(nullptr)
	, m_bIsFirstLoad(true)
{
}

CChatFriendDlg::~CChatFriendDlg() {
}

void CChatFriendDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BOOL CChatFriendDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	SetBackgroundColor(RGB(255, 255, 255));

	m_pImgSend = FileService::LoadImageFromFile("icon/SendMsgIcon.png");
	m_pImgEmoji = FileService::LoadImageFromFile("icon/EmojiPickerIcon.png");
	m_pImgImage = FileService::LoadImageFromFile("icon/ImagePickerIcon.png");
	m_pImgAttach = FileService::LoadImageFromFile("icon/AttachmentButtonIcon.png");

	GetDlgItem(IDC_STATIC_CHATFRIEND_SEND)->GetWindowRect(&m_rectSendBtn);
	ScreenToClient(&m_rectSendBtn);
	GetDlgItem(IDC_STATIC_CHATFRIEND_SEND)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_CHATFRIEND_EMOJI)->GetWindowRect(&m_rectEmojiBtn);
	ScreenToClient(&m_rectEmojiBtn);
	GetDlgItem(IDC_STATIC_CHATFRIEND_EMOJI)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_CHATFRIEND_IMAGE)->GetWindowRect(&m_rectImageBtn);
	ScreenToClient(&m_rectImageBtn);
	GetDlgItem(IDC_STATIC_CHATFRIEND_IMAGE)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_CHATFRIEND_ATTACH)->GetWindowRect(&m_rectAttachBtn);
	ScreenToClient(&m_rectAttachBtn);
	GetDlgItem(IDC_STATIC_CHATFRIEND_ATTACH)->ShowWindow(SW_HIDE);

	m_nScrollPos = 0;
	m_nTotalHeight = 0;

	GetDlgItem(IDC_STATIC_CHATFRIEND_CHATAREA)->GetWindowRect(&m_rectChatArea);
	ScreenToClient(&m_rectChatArea);
	GetDlgItem(IDC_STATIC_CHATFRIEND_CHATAREA)->ShowWindow(SW_HIDE);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHATFRIEND_SEND);
	if (pEdit) {
		pEdit->SetCueBanner(_T("Nhập tin nhắn..."));
	}

	m_fontTitle.CreatePointFont(120, _T("Segoe UI Semibold"));

	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->SetFont(&m_fontTitle);

	GetMessage();
	UpdateLastMessageTime();
	SetTimer(1, 1000, NULL);

	return TRUE;
}

void CChatFriendDlg::OnPaint() {
	CPaintDC dc(this);
	PaintService::DrawAppBar(this, IDC_STATIC_CHATFRIEND_APPBAR, &dc);

	// hiển thị nickname
	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->ShowWindow(SW_SHOW);

	std::string friendId = m_currentFriend.friendId;
	CString strDisplayName = CA2W(m_currentFriend.fullName.c_str(), CP_UTF8);
	auto it = theApp.m_mapNickname.find(friendId);
	if (it != theApp.m_mapNickname.end()) {
		strDisplayName = CA2W(it->second.nickname.c_str(), CP_UTF8);
	}

	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->SetWindowText(strDisplayName);

	// Hiển thị ra UI
	CWnd* pLabel = GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME);
	if (pLabel) {
		pLabel->ShowWindow(SW_SHOW);
		pLabel->SetWindowText(strDisplayName);
	}

	PaintService::DrawIcon(&dc, m_pImgSend, m_rectSendBtn.left, m_rectSendBtn.top, m_rectSendBtn.Width(), m_rectSendBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgEmoji, m_rectEmojiBtn.left, m_rectEmojiBtn.top, m_rectEmojiBtn.Width(), m_rectEmojiBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgImage, m_rectImageBtn.left, m_rectImageBtn.top, m_rectImageBtn.Width(), m_rectImageBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgAttach, m_rectAttachBtn.left, m_rectAttachBtn.top, m_rectAttachBtn.Width(), m_rectAttachBtn.Height());

	DrawChatArea(&dc);
}

void CChatFriendDlg::GetMessage() {
	// 1. Gọi API dựa trên friendId riêng của khung chat này
	std::string url = "http://localhost:8888/api/message/get-message?FriendID=" + m_currentFriend.friendId;
	std::string token = theApp.m_userData.token;
	std::string res = ApiService::SendGetRequest(url, token);

	if (res == "") return;

	nlohmann::json jsonRes = nlohmann::json::parse(res);
	if (jsonRes["status"] == 1) {
		// 2. Kiểm tra thay đổi dữ liệu trên vector riêng m_vecLocalMessages
		bool bDataChanged = false;
		if (m_vecLocalMessages.size() != jsonRes["data"].size()) bDataChanged = true;
		else if (!m_vecLocalMessages.empty() && !jsonRes["data"].empty()) {
			if (m_vecLocalMessages.back().id != jsonRes["data"].back().value("id", "")) bDataChanged = true;
		}

		if (!bDataChanged && !m_bIsFirstLoad) return;

		DatabaseService::ExecuteSQL("BEGIN TRANSACTION;");

		// 3. Clear và nạp tin nhắn mới vào vector riêng
		m_vecLocalMessages.clear();
		for (auto& item : jsonRes["data"]) {
			Message msg;
			msg.id = item.value("id", "");
			msg.content = item.value("Content", "");
			msg.isSend = item.value("isSend", 0);
			msg.createdAt = item.value("CreatedAt", "");
			msg.messageType = item.value("MessageType", 0);

			if (item.contains("Images") && item["Images"].is_array()) {
				for (auto& imgJson : item["Images"]) {
					MediaItem img; img.id = imgJson.value("_id", ""); img.url = imgJson.value("urlImage", ""); img.fileName = imgJson.value("FileName", "");
					msg.images.push_back(img);
				}
			}
			if (item.contains("Files") && item["Files"].is_array()) {
				for (auto& fileJson : item["Files"]) {
					MediaItem file; file.id = fileJson.value("_id", ""); file.url = fileJson.value("urlFile", ""); file.fileName = fileJson.value("FileName", "");
					msg.files.push_back(file);
				}
			}
			DatabaseService::SaveMessageToDB(msg, m_currentFriend.friendId);
			m_vecLocalMessages.push_back(msg);
		}

		DatabaseService::ExecuteSQL("COMMIT;");

		// 4. Tính toán chiều cao dựa trên m_vecLocalMessages
		CClientDC dc(this);
		CFont font; font.CreatePointFont(90, _T("Segoe UI"));
		CFont* pOld = dc.SelectObject(&font);

		int total = 0;
		int nLimitWidth = (int)(m_rectChatArea.Width() * 0.7);
		int nPadding = 8;
		for (const auto& msg : m_vecLocalMessages) {
			CRect rCalc(0, 0, nLimitWidth, 0);
			CString strContent(CA2W(msg.content.c_str(), CP_UTF8));
			dc.DrawText(strContent, rCalc, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);
			total += (rCalc.Height() + (nPadding * 3));
		}
		m_nTotalHeight = total + 5;
		dc.SelectObject(pOld);

		// 5. Cập nhật vị trí cuộn và yêu cầu vẽ lại
		int nVisibleHeight = m_rectChatArea.Height();
		int maxScroll = max(0, m_nTotalHeight - nVisibleHeight);

		if (m_bIsFirstLoad || bDataChanged) {
			m_nScrollPos = maxScroll;
			m_bIsFirstLoad = false;
		}

		// 6. Cập nhật thời gian hiển thị cuối cùng
		UpdateLastMessageTime();

		InvalidateRect(&m_rectChatArea, FALSE);
	}
}


int CChatFriendDlg::DrawSingleMessage(CDC* pDC, const Message& msg, int x, int y, int nContainerWidth) {
	int nLimitWidth = (int)(nContainerWidth * 0.7);
	int nPadding = 8;

	CFont font; font.CreatePointFont(90, _T("Segoe UI"));
	CFont* pOldFont = pDC->SelectObject(&font);

	CString strContent(CA2W(msg.content.c_str(), CP_UTF8));
	CRect rText(0, 0, nLimitWidth, 0);
	pDC->DrawText(strContent, rText, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);

	int bubbleW = rText.Width() + (nPadding * 3);
	int bubbleH = rText.Height() + (nPadding * 2);

	int startX = (msg.messageType == 0) ? (x + nPadding) : (x + nContainerWidth - bubbleW - nPadding);
	CRect rBubble(startX, y, startX + bubbleW, y + bubbleH);

	// 1. CHUẨN BỊ BÚT VẼ (PEN) CHO VIỀN MƯỢT HƠN (màu xám nhạt, bo góc đẹp hơn)
	COLORREF penColor = RGB(180, 180, 180);
	CPen pen(PS_SOLID, 1, penColor);
	CPen* pOldPen = pDC->SelectObject(&pen);

	// 2. CHUẨN BỊ MÀU NỀN (BRUSH)
	CBrush brush(msg.messageType == 0 ? RGB(218, 218, 218) : RGB(67, 127, 236));
	CBrush* pOldBrush = pDC->SelectObject(&brush);

	// 3. VẼ BO GÓC
	pDC->RoundRect(rBubble, CPoint(20, 20));

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(msg.messageType == 0 ? RGB(0, 0, 0) : RGB(255, 255, 255));

	CRect rTextDraw = rBubble;
	rTextDraw.DeflateRect(nPadding, nPadding / 2);
	pDC->DrawText(strContent, rTextDraw, DT_WORDBREAK | DT_EDITCONTROL);

	pDC->SelectObject(pOldFont);

	return bubbleH + nPadding;
}

void CChatFriendDlg::DrawChatArea(CDC* pDC) {
	pDC->FillSolidRect(m_rectChatArea, RGB(255, 255, 255));
	int nSaved = pDC->SaveDC();
	pDC->IntersectClipRect(&m_rectChatArea);

	int nContainerWidth = m_rectChatArea.Width();
	int nPadding = 8;
	int currentY = m_rectChatArea.top + nPadding - m_nScrollPos;

	for (const auto& msg : m_vecLocalMessages) {
		int h = DrawSingleMessage(pDC, msg, m_rectChatArea.left, currentY, nContainerWidth);
		currentY += h;
	}

	m_nTotalHeight = (currentY + m_nScrollPos) - m_rectChatArea.top;
	pDC->RestoreDC(nSaved);
}

BOOL CChatFriendDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	CPoint ptClient = pt;
	ScreenToClient(&ptClient);

	if (m_rectChatArea.PtInRect(ptClient)) {
		int scrollStep = m_rectChatArea.Height() / 3;

		if (zDelta > 0) { 
			m_nScrollPos -= scrollStep;
		}
		else { 
			m_nScrollPos += scrollStep;
		}

		if (m_nScrollPos < 0) m_nScrollPos = 0;

		int nVisibleHeight = m_rectChatArea.Height();
		int maxScroll = max(0, m_nTotalHeight - nVisibleHeight);

		if (m_nScrollPos > maxScroll) m_nScrollPos = maxScroll;

		InvalidateRect(&m_rectChatArea);
		return TRUE;
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CChatFriendDlg::SendMessage() {
	CString content;
	GetDlgItemText(IDC_EDIT_CHATFRIEND_SEND, content);
	if (content != _T("")) {
		std::string url = "http://localhost:8888/api/message/send-message";
		std::string token = theApp.m_userData.token;
		std::string strContent = CW2A(content.GetString(), CP_UTF8);
		nlohmann::json data;
		data["FriendID"] = m_currentFriend.friendId;
		data["Content"] = strContent;
		ApiService::SendPostRequest(url, data, token);
	}
}

void CChatFriendDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_rectSendBtn.PtInRect(point)) {
		SendMessage();
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHATFRIEND_SEND);
		if (pEdit) pEdit->SetWindowText(_T(""));
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CChatFriendDlg::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == 1) {
		GetMessage();
		UpdateLastMessageTime();
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CChatFriendDlg::UpdateLastMessageTime() {
	if (m_vecLocalMessages.empty()) return;
	const Message& lastMsg = m_vecLocalMessages.back();
	CString strDisplayTime = ChangeFormat::FormatISOToDateTime(lastMsg.createdAt);
	GetDlgItem(IDC_STATIC_CHATFRIEND_LT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CHATFRIEND_LT)->SetWindowText(strDisplayTime);
}

HBRUSH CChatFriendDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_CHATFRIEND_FULLNAME) {
		pDC->SetTextColor(RGB(0, 0, 0));
		
		pDC->SetBkColor(RGB(217, 217, 217)); 

		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_EDIT_CHATFRIEND_SEND) {
		pDC->SetTextColor(RGB(0, 0, 0));

		pDC->SetBkColor(RGB(235, 233, 239));

		static CBrush brush(RGB(235, 233, 239));
		return (HBRUSH)brush.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_CHATFRIEND_EXIT) {
		pDC->SetTextColor(RGB(0, 0, 0));

		pDC->SetBkColor(RGB(217, 217, 217));

		static CBrush brush(RGB(217, 217, 217));
		return (HBRUSH)brush.GetSafeHandle();
	}

	return hbr;
}

void CChatFriendDlg::OnStnClickedStaticChatfriendFullname()
{
	if (!m_isFriendDlgCreated) {
		if (m_friendDlg.Create(IDD_FRIEND_DIALOG, this)) {
			m_isFriendDlgCreated = TRUE;
		}
	}

	m_friendDlg.m_friendData = this->m_currentFriend;

	m_friendDlg.GetDlgItem(IDC_STATIC_FRIEND_ERROR)->ShowWindow(SW_HIDE);

	m_friendDlg.ShowWindow(SW_SHOW);
	m_friendDlg.BringWindowToTop();
}

void CChatFriendDlg::OnClose() {
	if (m_isFriendDlgCreated && ::IsWindow(m_friendDlg.GetSafeHwnd())) {
		m_friendDlg.DestroyWindow(); // Hủy cửa sổ Friend
		m_isFriendDlgCreated = FALSE; // Reset lại cờ
	}

	CDialogEx::OnClose();
}

void CChatFriendDlg::OnStnClickedStaticChatfriendExit(){
	EndDialog(ID_EXIT_TRIGGER);
}


LRESULT CChatFriendDlg::OnNcHitTest(CPoint point){
	LRESULT hit = CDialogEx::OnNcHitTest(point);

	if (hit == HTCLIENT) {
		return HTCAPTION;
	}

	return hit;
}

