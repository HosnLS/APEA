//
// Created by HosnLS on 2022/7/24.
//

#include <iostream>
#include <Windows.h>

#include "opencv2/highgui.hpp"

#include "../capture/DXGICapture.h"
#include "../inference/TRTInfer.h"
#include "../paint/GDIPaint.h"

using namespace std;

class Swtch {
public:
    void Update() {
        bool now = GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_F8);
        if (now && !lastSwt) {
            Swt = !Swt;
            cout << (Swt ? "Working" : "Idle") << endl;
        }
        lastSwt = now;
    }

public:
    bool Swt{false};

private:
    bool lastSwt{false};
};

#define MODEL_PATH "C:/Users/HosnLS/source/repos/20220723_FpsHelper/FPS-Helper/workspace/nanodet-plus-m_320.trt"
#define MODEL_R 320

int main() {
    cout << "Loading..." << endl;

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    void *pData = new char[MODEL_R * MODEL_R * 4];


    GDIPaint paint;
//    OnnxInfer infer(MODEL_PATH, MODEL_R, MODEL_R, 't');
    TRTInfer infer(MODEL_PATH, MODEL_R, MODEL_R);
    VideoDXGICaptor ss;

    cout << "FPSHelper" << endl;
    cout << "Press F7+F8 to switch between working and idle" << endl;

    Swtch mainSwt;

    time_t t1 = GetTickCount();
    while (true) {
        for (int i = 0; i < 50; ++i) {
            // get cursor pos
            if (mainSwt.Swt) {
                POINT p;
                GetCursorPos(&p);

                // get frame
                RECT rect{max(0l, p.x - MODEL_R / 2),
                          max(0l, p.y - MODEL_R / 2),
                          min(GetSystemMetrics(SM_CXSCREEN) - 1l, p.x + MODEL_R / 2),
                          min(GetSystemMetrics(SM_CYSCREEN) - 1l, p.y + MODEL_R / 2)};
                ss.CaptureImage(rect, pData);

                // infer
                AimBox aimbox;
                infer.Infer(pData, aimbox);


//              paint
                aimbox.xl += rect.left;
                aimbox.xr += rect.left;
                aimbox.yl += rect.top;
                aimbox.yr += rect.top;
                paint.Paint(aimbox);

                // aim
//                if (GetAsyncKeyState(VK_LBUTTON) < 0)
//                    SetCursorPos(p.x + ceil(0.1 * ((aimbox.xl + aimbox.xr) / 2 - p.x)),
//                                 p.y + ceil(0.1 * ((aimbox.yl + aimbox.yr) / 2 - p.y)));

//                if (GetAsyncKeyState(VK_LBUTTON) < 0)
//                    mouse_event(MOUSEEVENTF_MOVE,
//                                ceil(0.35 * ((aimbox.xl + aimbox.xr) / 2 - p.x)),
//                                ceil(0.3 * ((aimbox.yl + aimbox.yr) / 2 - p.y)), 0, 0);


            cv::Mat img(MODEL_R, MODEL_R, CV_8UC4, pData);
            cv::imshow("img", img);
            cv::waitKey(1);
            } else {
                Sleep(1);
            }
            mainSwt.Update();
        }
        time_t t2 = GetTickCount();
        if (mainSwt.Swt) std::cout << "fps: " << 50000.0 / (t2 - t1) << std::endl;
        t1 = t2;
    }
    return 0;
}