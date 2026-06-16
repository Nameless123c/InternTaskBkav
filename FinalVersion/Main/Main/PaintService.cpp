#include "pch.h"
#include "PaintService.h"

void PaintService::DrawAvatar(CDC* pDC, CImage* pImage, int x, int y, int w, int h) {
    if (!pImage || pImage->IsNull()) return;

    int nSaved = pDC->SaveDC();

    CRgn circleRgn;
    circleRgn.CreateEllipticRgn(x, y, x + w, y + h);
    pDC->SelectClipRgn(&circleRgn);

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
    
    if (pImage->GetBPP() != 32) {
        pImage->StretchBlt(pDC->GetSafeHdc(), x, y, w, h, SRCCOPY);
    }
    else {
        BLENDFUNCTION bf = { 0 };
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 255; 
        bf.AlphaFormat = AC_SRC_ALPHA;

        ::AlphaBlend(pDC->GetSafeHdc(), x, y, w, h,
            pImage->GetDC(), 0, 0, pImage->GetWidth(), pImage->GetHeight(),
            bf);
        pImage->ReleaseDC();
    }
}

void PaintService::DrawErrorMessage(CWnd* pParent, int nCtrlID, CString strMessage) {
    CWnd* pCtrl = pParent->GetDlgItem(nCtrlID);
    if (!pCtrl) return;

    // 1. Hiện control ra trước
    pCtrl->ShowWindow(SW_SHOW);

    // 2. Ép Windows vẽ cái khung trống ngay lập tức
    pCtrl->UpdateWindow();

    // 3. Bây giờ mới vẽ chữ đè lên
    CRect rect;
    pCtrl->GetWindowRect(&rect);
    pParent->ScreenToClient(&rect);

    CClientDC dc(pParent);
    dc.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));
    dc.SetTextColor(RGB(255, 0, 0));
    dc.SetBkMode(TRANSPARENT);

    CFont font;
    font.CreatePointFont(80, _T("Arial"));
    CFont* pOldFont = dc.SelectObject(&font);

    dc.DrawText(strMessage, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    dc.SelectObject(pOldFont);
}