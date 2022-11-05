//
// Created by HosnLS on 2022/11/4.
//
#include <iostream>
#include <fstream>
#include <Windows.h>

#include "opencv2/opencv.hpp"

#include "../inference/TRTInfer.h"

int main() {
    TRTInfer infer("C:/Users/HosnLS/source/repos/20220723_FpsHelper/FPS-Helper/workspace/nanodet-plus-m-1.5x_416.trt", 416, 416);

    auto img = cv::imread("test.png", cv::IMREAD_COLOR);
    cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
    AimBox aimbox;
    infer.Infer(img.data, aimbox);


    time_t t1 = GetTickCount();
    while (true) {
        for (int i = 0; i < 20; ++i) {
            infer.Infer(img.data, aimbox);
        }
        time_t t2 = GetTickCount();
        std::cout << "fps: " << 20000.0 / (t2 - t1) << std::endl;
        t1 = t2;
    }
    return 0;
}