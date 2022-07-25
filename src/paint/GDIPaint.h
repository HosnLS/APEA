//
// Created by HosnLS on 2022/7/25.
//

#ifndef FPSHELPER_GDIPAINT_H
#define FPSHELPER_GDIPAINT_H

#include <Windows.h>

#include "../core/pipeline.h"

class GDIPaint{
public:
    GDIPaint();
    ~GDIPaint();
    bool Paint(AimBox &aimbox);
    bool Clear();
private:
    HDC hdc;
};

#endif //FPSHELPER_GDIPAINT_H
