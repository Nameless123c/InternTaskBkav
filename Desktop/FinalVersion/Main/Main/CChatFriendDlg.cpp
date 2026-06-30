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

	m_vecLocalMessages = DatabaseService::LoadMessages(theApp.m_userData.userId, m_currentFriend.friendId);

	m_pImgSend = FileService::LoadImageFromFile("icon/SendMsgIcon.png");
	m_pImgEmoji = FileService::LoadImageFromFile("icon/EmojiPickerIcon.png");
	m_pImgImage = FileService::LoadImageFromFile("icon/ImagePickerIcon.png");
	m_pImgAttach = FileService::LoadImageFromFile("icon/AttachmentButtonIcon.png");
	m_pImgFile = FileService::LoadImageFromFile("OtherRC/file.png");
	m_pImgDownload = FileService::LoadImageFromFile("OtherRC/download.png");

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

	GetDlgItem(IDC_STATIC_CHATFRIEND_FIAREA)->GetWindowRect(&m_rectFIArea);
	ScreenToClient(&m_rectFIArea);
	GetDlgItem(IDC_STATIC_CHATFRIEND_FIAREA)->ShowWindow(SW_HIDE);

	m_nScrollPos = 0;
	m_nTotalHeight = 0;

	GetDlgItem(IDC_STATIC_CHATFRIEND_CHATAREA)->GetWindowRect(&m_rectChatArea);
	ScreenToClient(&m_rectChatArea);
	GetDlgItem(IDC_STATIC_CHATFRIEND_CHATAREA)->ShowWindow(SW_HIDE);

	RefreshChatLayout();

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHATFRIEND_SEND);
	if (pEdit) {
		pEdit->SetCueBanner(_T("Nhập tin nhắn..."));
	}

	m_fontTitle.CreatePointFont(120, _T("Segoe UI Semibold"));

	GetDlgItem(IDC_STATIC_CHATFRIEND_FULLNAME)->SetFont(&m_fontTitle);

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

	if (m_bHasFileSelected) {
		PaintService::DrawFileArea(&dc, m_rectFIArea, m_pImgFile, m_vecPendingFiles);
	}
}

void CChatFriendDlg::GetMessage() {
	std::string lastTime = "0";
	if (!m_vecLocalMessages.empty()) {
		lastTime = m_vecLocalMessages.back().createdAt;
	}


	// 1. Gọi API dựa trên friendId riêng của khung chat này
	std::string url = "http://localhost:8888/api/message/get-message?FriendID="
		+ m_currentFriend.friendId + "&LastTime=" + lastTime;
	std::string token = theApp.m_userData.token;
	std::string res = ApiService::SendGetRequest(url, token);

	if (res == "") return;

	nlohmann::json jsonRes = nlohmann::json::parse(res);
	if (jsonRes["status"] == 1) {
		
		if (!jsonRes.contains("data") || jsonRes["data"].empty()) return;

		std::string lastServerId = jsonRes["data"].back().value("id", "");


		if (!m_vecLocalMessages.empty() && m_vecLocalMessages.back().id == lastServerId) {
			return;
		}

		DatabaseService::ExecuteSQL("BEGIN TRANSACTION;");

		// 3. Clear và nạp tin nhắn mới vào vector riêng
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
					img.url = img.url.substr(1);
					DownloadImg(img);
					msg.images.push_back(img);
				}
			}
			if (item.contains("Files") && item["Files"].is_array()) {
				for (auto& fileJson : item["Files"]) {
					MediaItem file; file.id = fileJson.value("_id", ""); file.url = fileJson.value("urlFile", ""); file.fileName = fileJson.value("FileName", "");
					file.url = file.url.substr(1);
					msg.files.push_back(file);
				}
			}

			std::string sender = "", receiver = "";
			
			if (msg.messageType == 0) {
				sender = m_currentFriend.friendId;
				receiver = theApp.m_userData.userId;
			}
			else {
				sender = theApp.m_userData.userId;
				receiver = m_currentFriend.friendId;
			}

			DatabaseService::EnsureSenderExists(sender);
			DatabaseService::SaveMessageToDB(msg, sender, receiver);
			m_vecLocalMessages.push_back(msg);
		}

		DatabaseService::ExecuteSQL("COMMIT;");

		// 4. Tính toán chiều cao dựa trên m_vecLocalMessages
		RefreshChatLayout();


		UpdateLastMessageTime();

		InvalidateRect(&m_rectChatArea, FALSE);
	}
}


int CChatFriendDlg::DrawSingleMessage(CDC* pDC, const Message& msg, int x, int y, int nContainerWidth) {
	int nLimitWidth = (int)(nContainerWidth * 0.7);
	int nPadding = 8;
	int currentY = y;

	bool isFileMsg = !msg.files.empty(); 
	bool isImageMsg = !msg.images.empty(); 
	
	// kịch bản đang là một tin nhắn chứa 1 file
	if (isFileMsg) {
		for (const auto& file : msg.files) {
			CString fileName(CA2W(file.fileName.c_str(), CP_UTF8));
			int iconArea = 42;
			CRect rText(0, 0, nLimitWidth - iconArea, 0);
			pDC->DrawText(fileName, &rText, DT_CALCRECT | DT_SINGLELINE);

			int bubbleW = min(rText.Width() + iconArea + (nPadding * 3), nLimitWidth);
			int bubbleH = 50;
			int startX = (msg.messageType == 0) ? (x + 8) : (x + nContainerWidth - bubbleW - 8);
			CRect rBubble(startX, currentY, startX + bubbleW, currentY + bubbleH);

			PaintService::DrawFileMessage(pDC, rBubble, fileName, m_pImgDownload);
			currentY += (bubbleH + nPadding);
		}
	}


	if (isImageMsg) {
		for (const auto& imgItem : msg.images) {
			int bubbleW = 50, bubbleH = 50;
			int startX = (msg.messageType == 0) ? (x + 8) : (x + nContainerWidth - bubbleW - 8);
			CRect rBubble(startX, currentY, startX + bubbleW, currentY + bubbleH);

			CBrush brush(RGB(240, 240, 240));
			pDC->FillRect(rBubble, &brush);

			CImage* img = FileService::LoadImageFromFile(imgItem.url);
			if (img && !img->IsNull()) {
				img->Draw(pDC->GetSafeHdc(), rBubble.left + 2, rBubble.top + 2, bubbleW - 4, bubbleH - 4);
				delete img;
			}
			else {
				pDC->DrawText(_T("Ảnh lỗi"), rBubble, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			currentY += (bubbleH + nPadding);
		}
	}

	CFont font; font.CreatePointFont(90, _T("Segoe UI"));
	CFont* pOldFont = pDC->SelectObject(&font);

	CString strContent(CA2W(msg.content.c_str(), CP_UTF8));
	CRect rText(0, 0, nLimitWidth, 0);
	pDC->DrawText(strContent, rText, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);

	int bubbleW = rText.Width() + (nPadding * 3);
	int bubbleH = rText.Height() + (nPadding * 2);

	int startX = (msg.messageType == 0) ? (x + nPadding) : (x + nContainerWidth - bubbleW - nPadding);
	CRect rBubble(startX, currentY, startX + bubbleW, currentY + bubbleH);

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

	currentY += (bubbleH + nPadding);

	return (currentY - y);
}

void CChatFriendDlg::DrawChatArea(CDC* pDC) {
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
	std::string strContent = CW2A(content.GetString(), CP_UTF8);

	if (strContent.empty()) {
		return;
	}

	std::string url = "http://localhost:8888/api/message/send-message";
	std::string token = theApp.m_userData.token;

	if (m_vecPendingFiles.empty()) {
		nlohmann::json data;
		data["FriendID"] = m_currentFriend.friendId;
		data["Content"] = strContent;

		ApiService::SendPostRequest(url, data, token);
	}
	else {
		ApiService::SendMultipartRequest(url, strContent, m_currentFriend.friendId, m_vecPendingFiles, token);
	}

	SetDlgItemText(IDC_EDIT_CHATFRIEND_SEND, _T(""));
	m_vecPendingFiles.clear(); 
	m_bHasFileSelected = FALSE; 

	InvalidateRect(m_rectFIArea, TRUE);
	RefreshChatLayout();
}

void CChatFriendDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_rectSendBtn.PtInRect(point)) {
		SendMessage();
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHATFRIEND_SEND);
		if (pEdit) pEdit->SetWindowText(_T(""));
	}

	if (m_rectAttachBtn.PtInRect(point)) {
		OpenAttachFileDialog();
	}

	if (m_rectFIArea.PtInRect(point) && !m_vecPendingFiles.empty()) {
		int currentX = m_rectFIArea.left + 15; // Vị trí bắt đầu y hệt hàm Draw
		int itemPadding = 20;
		int iconSize = 25;

		for (int i = 0; i < (int)m_vecPendingFiles.size(); ++i) {
			// 1. Tính toán độ rộng nội dung giống y hệt hàm Draw
			CString strFileName(CA2W(m_vecPendingFiles[i].fileName.c_str(), CP_UTF8));
			bool isImage = strFileName.Right(4).CompareNoCase(_T(".jpg")) == 0 ||
				strFileName.Right(4).CompareNoCase(_T(".png")) == 0 ||
				strFileName.Right(5).CompareNoCase(_T(".jpeg")) == 0;

			int contentWidth = iconSize;
			if (!isImage) {
				CSize textSize = GetDC()->GetTextExtent(strFileName); // Lưu ý: Cần đảm bảo context hợp lệ
				contentWidth += (textSize.cx + 5);
			}

			// 2. Tính lại vị trí dấu X (xPosBtn) và vùng chứa X (rErase)
			int xPosBtn = currentX + contentWidth + 5;

			// Vị trí X cao lên (top + 3) giống như bạn đã yêu cầu ở bước trước
			CRect rErase(xPosBtn, m_rectFIArea.top + 3, xPosBtn + 15, m_rectFIArea.top + 18);

			// 3. Kiểm tra xem click vào X chưa
			if (rErase.PtInRect(point)) {
				m_vecPendingFiles.erase(m_vecPendingFiles.begin() + i);
				if(m_vecPendingFiles.size() == 0)
					m_bHasFileSelected = FALSE;
				InvalidateRect(m_rectFIArea, TRUE);
				return;
			}

			// 4. Cập nhật currentX cho item tiếp theo giống hàm Draw
			currentX = rErase.right + itemPadding;
		}
	}

	if (m_rectChatArea.PtInRect(point)) {
		CPoint ptHit = point;
		ptHit.y += m_nScrollPos;
		ptHit.y -= m_rectChatArea.top;

		int nPadding = 8;
		int nLimitWidth = (int)(m_rectChatArea.Width() * 0.7);
		int currentY = 0;

		for (const auto& msg : m_vecLocalMessages) {
			// 1. Tính chiều cao tin nhắn (phải dùng lại công thức của RefreshChatLayout)
			int msgHeight = 0;
			for (const auto& f : msg.files) msgHeight += (50 + nPadding);
			for (const auto& i : msg.images) msgHeight += (50 + nPadding);
			
			CClientDC dc(this);
			CRect rCalc(0, 0, nLimitWidth - (nPadding * 2), 0);
			CString strContent(CA2W(msg.content.c_str(), CP_UTF8));
			dc.DrawText(strContent, rCalc, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);
			msgHeight += (rCalc.Height() + (nPadding * 3));

			// Nếu click nằm trong vùng tin nhắn này
			if (ptHit.y >= currentY && ptHit.y <= currentY + msgHeight) {
				int itemY = currentY;

				// 2. Kiểm tra File (Logic giống hệt vòng lặp vẽ)
				for (const auto& file : msg.files) {
					CString fileName(CA2W(file.fileName.c_str(), CP_UTF8));

					// Tính lại bubbleW giống hệt như lúc vẽ
					CClientDC dc(this);
					int iconArea = 42;
					CRect rText(0, 0, nLimitWidth - iconArea, 0);
					dc.DrawText(fileName, &rText, DT_CALCRECT | DT_SINGLELINE);
					int bubbleW = min(rText.Width() + iconArea + (nPadding * 3), nLimitWidth);
					int bubbleH = 50;

					int startX = (msg.messageType == 0) ? (m_rectChatArea.left + 8) : (m_rectChatArea.left + m_rectChatArea.Width() - bubbleW - 8);
					CRect rBubble(startX, itemY, startX + bubbleW, itemY + bubbleH);

					if (rBubble.PtInRect(ptHit)) {
						DownloadFile(file);
						return;
					}
					itemY += (bubbleH + nPadding);
				}
			}
			currentY += msgHeight;
		}
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
	CPoint pt = point;
	ScreenToClient(&pt);

	if (m_rectSendBtn.PtInRect(pt) || m_rectAttachBtn.PtInRect(pt) || m_rectFIArea.PtInRect(pt) || m_rectChatArea.PtInRect(pt) ){
		return HTCLIENT;
	}

	return HTCAPTION;
}

void CChatFriendDlg::RefreshChatLayout() {
	CClientDC dc(this);
	CFont font; font.CreatePointFont(90, _T("Segoe UI"));
	CFont* pOld = dc.SelectObject(&font);

	int total = 0;
	int nLimitWidth = (int)(m_rectChatArea.Width() * 0.7);
	int nPadding = 8;

	for (const auto& msg : m_vecLocalMessages) {
		int msgHeight = 0;

		// Cộng chiều cao File
		for (const auto& file : msg.files) msgHeight += (50 + nPadding);

		// Cộng chiều cao Ảnh
		for (const auto& img : msg.images) msgHeight += (50 + nPadding);

		// Cộng chiều cao Text
		if (!msg.content.empty()) {
			CRect rCalc(0, 0, nLimitWidth - (nPadding * 2), 0);
			CString strContent(CA2W(msg.content.c_str(), CP_UTF8));
			dc.DrawText(strContent, rCalc, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);
			msgHeight += (rCalc.Height() + (nPadding * 3));
		}

		total += msgHeight;
	}

	m_nTotalHeight = total + (nPadding);
	dc.SelectObject(pOld);

	// Tự động cuộn xuống dưới
	int nVisibleHeight = m_rectChatArea.Height();
	m_nScrollPos = max(0, m_nTotalHeight - nVisibleHeight);

	InvalidateRect(&m_rectChatArea, TRUE);
}

void CChatFriendDlg::OpenAttachFileDialog() {

	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("All Files (*.*)|*.*||"), this);

	if (fileDlg.DoModal() == IDOK) {
		if (m_vecPendingFiles.size() < 3) {
			m_bHasFileSelected = TRUE;

			MediaItem item;
			// 1. Lấy tên file để hiển thị
			item.fileName = CW2A(PathFindFileName(fileDlg.GetPathName()), CP_UTF8);

			// 2. LẤY ĐƯỜNG DẪN ĐẦY ĐỦ Ở ĐÂY
			item.url = CW2A(fileDlg.GetPathName(), CP_UTF8);

			m_vecPendingFiles.push_back(item);

			InvalidateRect(m_rectFIArea, TRUE);
		}
	}
}

void CChatFriendDlg::DownloadImg(MediaItem img) {
	std::string fullUrl = "http://localhost:8888/api/" + img.url;
	std::string localSavePath = img.url;
	std::string token = theApp.m_userData.token;

	bool ok = ApiService::DownloadFile(fullUrl, localSavePath, token);
}

void CChatFriendDlg::DownloadFile(MediaItem file) {
	// 1. Mở hộp thoại chọn thư mục
	CFolderPickerDialog dlg;

	if (dlg.DoModal() == IDOK) {
		CString strFolderPath = dlg.GetPathName(); // Lấy đường dẫn thư mục user chọn

		// 2. Tạo đường dẫn lưu file đầy đủ (Tên file lấy từ file.fileName)
		CString strFileName(CA2W(file.fileName.c_str(), CP_UTF8));
		CString strFullPath = strFolderPath + _T("\\") + strFileName;

		// 3. Chuẩn bị các tham số cho API
		std::string fullUrl = "http://localhost:8888/api/" + file.url;
		std::string token = theApp.m_userData.token;
		std::string localSavePath = CW2A(strFullPath, CP_UTF8);

		// 4. Gọi API tải file
		bool ok = ApiService::DownloadFile(fullUrl, localSavePath, token);
	}
}