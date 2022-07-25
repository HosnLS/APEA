//
// Created by HosnLS on 2022/7/25.
//

#include "GDIPaint.h"

GDIPaint::GDIPaint() {
    hdc = GetDC(NULL);
}

GDIPaint::~GDIPaint() {
    ReleaseDC(NULL, hdc);
}

bool GDIPaint::Paint(AimBox &aimbox) {
    for (int x = aimbox.xl; x <= aimbox.xr; x++){
        SetPixel(hdc, x, aimbox.yl, RGB(255, 255, 0));
        SetPixel(hdc, x, aimbox.yr, RGB(255, 255, 0));
    }
    for (int y = aimbox.yl; y <= aimbox.yr; y++){
        SetPixel(hdc, aimbox.xl, y, RGB(255, 255, 0));
        SetPixel(hdc, aimbox.xr, y, RGB(255, 255, 0));
    }
    return true;
}

bool GDIPaint::Clear() {
//    RECT rect{0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
//    HRGN region = CreateRoundRectRgn(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0);
//    RedrawWindow(NULL, &rect, region, RDW_INVALIDATE);
    return true;
}

