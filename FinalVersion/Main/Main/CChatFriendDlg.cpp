#include "pch.h"
#include "Main.h"
#include "afxdialogex.h"
#include "CChatFriendDlg.h"
#include "ApiService.h"
#include "ChangeFormat.h"
#include "FileService.h"
#include "PaintService.h"


IMPLEMENT_DYNAMIC(CChatFriendDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CChatFriendDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CChatFriendDlg::CChatFriendDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATFRIEND_DIALOG, pParent)
	, m_nScrollPos(0)            
	, m_nTotalHeight(0)         
	, m_pImgSend(nullptr)        
	, m_pImgEmoji(nullptr)
	, m_pImgImage(nullptr)
	, m_pImgAttach(nullptr)
{

}

CChatFriendDlg::~CChatFriendDlg(){

}

void CChatFriendDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
}


BOOL CChatFriendDlg::OnInitDialog(){
	CDialogEx::OnInitDialog();

	m_pImgSend = FileService::LoadImageFromFile("icon/SendMsgIcon.png");
	m_pImgEmoji = FileService::LoadImageFromFile("icon/EmojiPickerIcon.png");
	m_pImgImage = FileService::LoadImageFromFile("icon/ImagePickerIcon.jpg");
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
		pEdit->SetCueBanner(ChangeFormat::UTF8ToCString("Nhập tin nhắn..."));
	}

	GetMessage();
	UpdateLastMessageTime();
	SetTimer(1, 1000, NULL);

	return TRUE;
}

void CChatFriendDlg::OnPaint(){
	CPaintDC dc(this);

	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->SetWindowText(ChangeFormat::UTF8ToCString(theApp.m_selectedFriend.fullName));
	
	
	PaintService::DrawIcon(&dc, m_pImgSend, m_rectSendBtn.left, m_rectSendBtn.top, m_rectSendBtn.Width(), m_rectSendBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgEmoji, m_rectEmojiBtn.left, m_rectEmojiBtn.top, m_rectEmojiBtn.Width(), m_rectEmojiBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgImage, m_rectImageBtn.left, m_rectImageBtn.top, m_rectImageBtn.Width(), m_rectImageBtn.Height());
	PaintService::DrawIcon(&dc, m_pImgAttach, m_rectAttachBtn.left, m_rectAttachBtn.top, m_rectAttachBtn.Width(), m_rectAttachBtn.Height());
		
	DrawChatArea(&dc);
}

void CChatFriendDlg::GetMessage() {
	std::string url = "http://localhost:8888/api/message/get-message?FriendID=" + theApp.m_selectedFriend.friendId;
	std::string token = theApp.m_userData.token;
	std::string res = ApiService::SendGetRequest(url, token);

	if (res == "") return;

	nlohmann::json jsonRes = nlohmann::json::parse(res);
	if (jsonRes["status"] == 1) {
		// 1. KIỂM TRA THAY ĐỔI: Chỉ vẽ lại khi có tin nhắn mới hoặc thay đổi
		bool bDataChanged = false;
		if (theApp.m_vecMessage.size() != jsonRes["data"].size()) {
			bDataChanged = true;
		}
		else if (!theApp.m_vecMessage.empty() && !jsonRes["data"].empty()) {
			// Kiểm tra ID tin nhắn cuối để chắc chắn
			if (theApp.m_vecMessage.back().id != jsonRes["data"].back().value("id", ""))
				bDataChanged = true;
		}

		if (!bDataChanged) return; // Thoát nếu không có gì mới

		// 2. NẠP DỮ LIỆU
		theApp.m_vecMessage.clear();
		for (auto& item : jsonRes["data"]) {
			Message msg;
			msg.id = item.value("id", "");
			msg.content = item.value("Content", "");
			msg.isSend = item.value("isSend", 0);
			msg.createdAt = item.value("CreatedAt", "");
			msg.messageType = item.value("MessageType", 0);

			// Nạp Images
			if (item.contains("Images") && item["Images"].is_array()) {
				for (auto& imgJson : item["Images"]) {
					MediaItem img;
					img.id = imgJson.value("_id", "");
					img.url = imgJson.value("urlImage", "");
					img.fileName = imgJson.value("FileName", "");
					msg.images.push_back(img);
				}
			}

			// Nạp Files
			if (item.contains("Files") && item["Files"].is_array()) {
				for (auto& fileJson : item["Files"]) {
					MediaItem file;
					file.id = fileJson.value("_id", "");
					file.url = fileJson.value("urlFile", "");
					file.fileName = fileJson.value("FileName", "");
					msg.files.push_back(file);
				}
			}
			theApp.m_vecMessage.push_back(msg);
		}

			// 3. TÍNH TOÁN CHIỀU CAO VÀ AUTO-SCROLL
			int nVisibleHeight = m_rectChatArea.Height();
			int oldMaxScroll = max(0, m_nTotalHeight - nVisibleHeight);
			bool isAtBottom = (m_nScrollPos >= oldMaxScroll - 50);

			CClientDC dc(this);
			CFont font; font.CreatePointFont(80, _T("Segoe UI"));
			CFont* pOld = dc.SelectObject(&font);

			int total = 0;
			int nLimitWidth = (int)(m_rectChatArea.Width() * 0.7);
			for (const auto& msg : theApp.m_vecMessage) {
				CRect rCalc(0, 0, nLimitWidth, 0);
				dc.DrawText(ChangeFormat::UTF8ToCString(msg.content), rCalc, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);
				total += (rCalc.Height() + 30);
			}
			m_nTotalHeight = total;
			dc.SelectObject(pOld);

			// 4. CẬP NHẬT VỊ TRÍ CUỘN
			if (isAtBottom) {
				m_nScrollPos = max(0, m_nTotalHeight - nVisibleHeight);
			}

			// 5. VẼ LẠI VÙNG CHAT (FALSE = không xóa nền để chống nhấp nháy)
			InvalidateRect(&m_rectChatArea, FALSE);
	}
}

int CChatFriendDlg::DrawSingleMessage(CDC* pDC, const Message& msg, int x, int y, int nContainerWidth) {
	int nPadding = 10;
	int nLimitWidth = (int)(nContainerWidth * 0.7);

	// 1. Cấu hình Font chữ để đo đạc chính xác
	CFont font;
	font.CreatePointFont(80, _T("Segoe UI"));
	CFont* pOldFont = pDC->SelectObject(&font);

	CString strContent = ChangeFormat::UTF8ToCString(msg.content);

	// 2. Tính toán kích thước với cờ DT_EDITCONTROL để ép xuống dòng tốt hơn
	CRect rText(0, 0, nLimitWidth, 0);
	pDC->DrawText(strContent, rText, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);

	int bubbleW = rText.Width() + 30; // +30 cho khoảng đệm 15px mỗi bên
	int bubbleH = rText.Height() + 20; // +20 cho khoảng đệm 10px trên/dưới

	// Xác định vị trí
	int startX = (msg.messageType == 0) ? (x + nPadding) : (x + nContainerWidth - bubbleW - nPadding);
	CRect rBubble(startX, y, startX + bubbleW, y + bubbleH);

	// 3. Vẽ nền (Bong bóng)
	CBrush brush(msg.messageType == 0 ? RGB(230, 230, 230) : RGB(0, 150, 255));
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->RoundRect(rBubble, CPoint(15, 15));
	pDC->SelectObject(pOldBrush);

	// 4. Vẽ chữ (Nội dung)
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(msg.messageType == 0 ? RGB(0, 0, 0) : RGB(255, 255, 255));

	CRect rTextDraw = rBubble;
	rTextDraw.DeflateRect(15, 10); // Phải khớp với khoảng đệm đã tính ở trên
	pDC->DrawText(strContent, rTextDraw, DT_WORDBREAK | DT_EDITCONTROL);

	// Khôi phục font cũ
	pDC->SelectObject(pOldFont);

	return bubbleH + 10;
}

void CChatFriendDlg::DrawChatArea(CDC* pDC) {
	pDC->FillSolidRect(m_rectChatArea, RGB(255, 255, 255));
	int nSaved = pDC->SaveDC();
	pDC->IntersectClipRect(&m_rectChatArea);

	int nContainerWidth = m_rectChatArea.Width();
	int currentY = m_rectChatArea.top + 10 - m_nScrollPos;

	for (const auto& msg : theApp.m_vecMessage) {
		int h = DrawSingleMessage(pDC, msg, m_rectChatArea.left, currentY, nContainerWidth);
		currentY += h;
	}

	m_nTotalHeight = (currentY - (m_rectChatArea.top + 10 - m_nScrollPos)) + m_nScrollPos;
	pDC->RestoreDC(nSaved);
}

BOOL CChatFriendDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	CPoint ptClient = pt;
	ScreenToClient(&ptClient);

	if (m_rectChatArea.PtInRect(ptClient)) {
		int scrollStep = 60;
		if (zDelta > 0) m_nScrollPos -= scrollStep;
		else m_nScrollPos += scrollStep;     

		if (m_nScrollPos < 0) m_nScrollPos = 0;
		int maxScroll = max(0, m_nTotalHeight - m_rectChatArea.Height());
		if (m_nScrollPos > maxScroll) m_nScrollPos = maxScroll;

		InvalidateRect(&m_rectChatArea);
		return TRUE;
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CChatFriendDlg::SendMessage() {

	CString content;
	GetDlgItemText(IDC_EDIT_CHATFRIEND_SEND, content);

	if (content == _T("")) {

	}
	else {
		std::string url = "http://localhost:8888/api/message/send-message";
		std::string token = theApp.m_userData.token;

		std::string strContent = ChangeFormat::CStringToUTF8(content);

		nlohmann::json data;
		data["FriendID"] = theApp.m_selectedFriend.friendId;
		data["Content"] = strContent;

		std::string res = ApiService::SendPostRequest(url, data, token);

		if (res == "") {
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOMECHAT_ERROR)->SetWindowTextW(ChangeFormat::UTF8ToCString("Lỗi kết nối mạng"));
		}
		else {
			nlohmann::json jsonRes = nlohmann::json::parse(res);

			if (jsonRes["status"] == 1) {
				
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

}

void CChatFriendDlg::OnLButtonDown(UINT nFlags, CPoint point){
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
	if (theApp.m_vecMessage.empty()) return;

	const Message& lastMsg = theApp.m_vecMessage.back();

	CString strDisplayTime = ChangeFormat::FormatISOToDateTime(lastMsg.createdAt);

	GetDlgItem(IDC_STATIC_CHATFRIEND_LT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CHATFRIEND_LT)->SetWindowText(strDisplayTime);
}