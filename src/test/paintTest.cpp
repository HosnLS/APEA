//
// Created by HosnLS on 2022/7/25.
//

#include "../core/pipeline.h"
#include "../paint/GDIPaint.h"

int main() {
    GDIPaint paint;
    for (int i = 0; i < 500; ++i) {
        AimBox ab{i, i + 100, i, i + 100};
        paint.Paint(ab);
        Sleep(10);
        paint.Clear();
    }
    return 0;
}