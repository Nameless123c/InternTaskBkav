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