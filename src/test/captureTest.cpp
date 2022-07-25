//
// Created by HosnLS on 2022/7/23.
//
#include <iostream>
#include <Windows.h>

#include "opencv2/highgui.hpp"

#include "../capture/DXGICapture.h"

int main() {
    VideoDXGICaptor ss;
    RECT rect{960-208, 540-208, 960+208, 540+208};
    void *pData = new char[416 * 416 * 4];
    time_t t1 = GetTickCount();
    while (true) {
        for (int i = 0; i < 20; ++i) {
            ss.CaptureImage(rect, pData);
            cv::Mat img(416, 416, CV_8UC4, pData);
            cv::imshow("img", img);
            cv::waitKey(1);
        }

        time_t t2 = GetTickCount();
        std::cout << "fps: " << 20000.0 / (t2 - t1) << std::endl;
        t1 = t2;
    }
    delete[] pData;

    return 0;

}