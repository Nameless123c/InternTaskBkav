#pragma once
#include "pch.h"
#include <gdiplus.h>
#include <vector>
#include "Message.h"

class PaintService {
public:
    static void DrawAvatar(CDC* pDC, CImage* pImage, int x, int y, int w, int h);
    static void DrawStatusDot(CDC* pDC, int x, int y, int size, bool isOnline);
    static void DrawIcon(CDC* pDC, CImage* pImage, int x, int y, int w, int h);
    static void DrawStyledTitle(CDC* pDC, CWnd* pParent, int nCtrlID, CString strText);
    static void DrawAppBar(CWnd* pParent, UINT nID, CDC* pDC);
    static void DrawFileArea(CDC* pDC, CRect rect, CImage* pImgFile, const std::vector<MediaItem>& files);
    static void DrawFileMessage(CDC* pDC, CRect rect, CString fileName, CImage* pImgDownload);
};