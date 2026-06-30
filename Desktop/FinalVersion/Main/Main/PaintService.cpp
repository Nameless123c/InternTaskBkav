#include "pch.h"
#include "PaintService.h"
#include "Message.h"

void PaintService::DrawAvatar(CDC* pDC, CImage* pImage, int x, int y, int w, int h) {
    if (!pImage || pImage->IsNull()) return;

    int nSaved = pDC->SaveDC();

    // 1. GIỚI HẠN VÙNG VẼ: Chỉ cho phép vẽ bên trong hình chữ nhật này
    // Điều này chặn đứng việc avatar vẽ lấn ra ngoài
    pDC->IntersectClipRect(x, y, x + w, y + h);

    // 2. CẮT HÌNH TRÒN: 
    CRgn circleRgn;
    circleRgn.CreateEllipticRgn(x, y, x + w, y + h);
    pDC->SelectClipRgn(&circleRgn, RGN_AND); // RGN_AND đảm bảo nó nằm gọn trong IntersectClipRect ở trên

    // 3. VẼ
    pDC->SetStretchBltMode(HALFTONE);
    pImage->StretchBlt(pDC->GetSafeHdc(), x, y, w, h, SRCCOPY);

    pDC->RestoreDC(nSaved);
}

void PaintService::DrawStatusDot(CDC* pDC, int x, int y, int size, bool isOnline) {
    if (!isOnline) return;

    CBrush brush(RGB(0, 200, 0)); 
    CPen pen(PS_SOLID, 2, RGB(255, 255, 255)); 

    CBrush* pOldBrush = pDC->SelectObject(&brush);
    CPen* pOldPen = pDC->SelectObject(&pen);

    pDC->Ellipse(x, y, x + size, y + size);

    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}

void PaintService::DrawIcon(CDC* pDC, CImage* pImage, int x, int y, int w, int h) {
    if (!pImage || pImage->IsNull()) return;
    
    pImage->Draw(pDC->GetSafeHdc(), x, y, w, h);
}


void PaintService::DrawAppBar(CWnd* pParent, UINT nID, CDC* pDC) {
    if (!pParent) return;

    CWnd* pCtrl = pParent->GetDlgItem(nID);

    if (pCtrl != nullptr) {
        if (pCtrl->IsWindowVisible()) {
            pCtrl->ShowWindow(SW_HIDE);
        }

        CRect rect;
        pCtrl->GetWindowRect(&rect);
        pParent->ScreenToClient(&rect);

        CBrush brush(RGB(217, 217, 217));
        pDC->FillRect(&rect, &brush);

        CPen pen(PS_SOLID, 1, RGB(180, 180, 180));
        CPen* pOldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left, rect.bottom - 1);
        pDC->LineTo(rect.right, rect.bottom - 1);
        pDC->SelectObject(pOldPen);
    }
}

void PaintService::DrawFileArea(CDC* pDC, CRect rect, CImage* pImgFile, const std::vector<MediaItem>& files) {
    if (!pDC) return;

    // Vẽ nền bao
    CPen pen(PS_SOLID, 1, RGB(196, 196, 196));
    CBrush brush(RGB(245, 245, 245));
    CPen* pOldPen = pDC->SelectObject(&pen);
    CBrush* pOldBrush = pDC->SelectObject(&brush);
    pDC->RoundRect(rect, CPoint(20, 20));
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);

    if (files.empty()) return;

    int currentX = rect.left + 15; // Vị trí bắt đầu của item đầu tiên
    int itemPadding = 20;          // Khoảng cách giữa các item với nhau
    int iconSize = 25;
    int yPos = rect.top + (rect.Height() - iconSize) / 2;

    for (const auto& file : files) {
        CString strFileName(CA2W(file.fileName.c_str(), CP_UTF8));
        bool isImage = strFileName.Right(4).CompareNoCase(_T(".jpg")) == 0 ||
            strFileName.Right(4).CompareNoCase(_T(".png")) == 0 ||
            strFileName.Right(5).CompareNoCase(_T(".jpeg")) == 0;

        int contentWidth = iconSize; // Bắt đầu bằng width của ảnh/icon

        // 1. Vẽ Ảnh hoặc Icon
        if (isImage) {
            CImage img;
            if (SUCCEEDED(img.Load(CA2W(file.url.c_str())))) {
                img.Draw(pDC->GetSafeHdc(), currentX, yPos, iconSize, iconSize);
            }
        }
        else {
            if (pImgFile && !pImgFile->IsNull()) {
                pImgFile->Draw(pDC->GetSafeHdc(), currentX, yPos, iconSize, iconSize);
            }

            // Vẽ tên file và mở rộng contentWidth
            CSize textSize = pDC->GetTextExtent(strFileName);
            contentWidth += (textSize.cx + 5);

            CRect rText(currentX + iconSize + 5, rect.top, currentX + iconSize + 5 + textSize.cx, rect.bottom);
            pDC->SetBkMode(TRANSPARENT);
            pDC->DrawText(strFileName, &rText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        }

        // 2. Vẽ dấu X (ngay sau nội dung)
        int xPosBtn = currentX + contentWidth + 5;
        int yTop = rect.top + 3;
        CRect rErase(xPosBtn, yTop, xPosBtn + 15, yTop + 15);
        pDC->SetTextColor(RGB(0, 0, 0));
        pDC->DrawText(_T("x"), &rErase, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

        // 3. Cập nhật currentX cho item tiếp theo
        currentX = rErase.right + itemPadding;
    }
}

void PaintService::DrawFileMessage(CDC* pDC, CRect rect, CString fileName, CImage* pImgDownload) {
    if (!pDC) return;

    // 1. Vẽ nền bong bóng (bo góc)
    CPen pen(PS_SOLID, 1, RGB(220, 220, 220));
    CBrush brush(RGB(240, 240, 240));
    CPen* pOldPen = pDC->SelectObject(&pen);
    CBrush* pOldBrush = pDC->SelectObject(&brush);

    pDC->RoundRect(rect, CPoint(10, 10));

    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);

    // 2. Vẽ Icon tải xuống
    int iconSize = 32;
    int iconX = rect.left + 15;
    int iconY = rect.top + (rect.Height() - iconSize) / 2;

    if (pImgDownload && !pImgDownload->IsNull()) {
        pImgDownload->Draw(pDC->GetSafeHdc(), iconX, iconY, iconSize, iconSize);
    }

    // 3. Vẽ Tên File 
    CRect nameRect(iconX + iconSize + 10, rect.top, rect.right - 10, rect.bottom);

    CFont* pDefaultFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
    CFont* pOldFont = pDC->SelectObject(pDefaultFont);

    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->DrawText(fileName, &nameRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    // Reset lại màu chữ về đen để tránh ảnh hưởng đến các hàm vẽ khác
    pDC->SetTextColor(RGB(0, 0, 0));
}