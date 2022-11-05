//
// Created by HosnLS on 2022/7/24.
//

#ifndef FPSHELPER_ONNXINFER_H
#define FPSHELPER_ONNXINFER_H

#include <onnxruntime_cxx_api.h>

#include "../core/pipeline.h"

class OnnxInfer {
public:
    OnnxInfer(const ORTCHAR_T *model_path, int w, int h, char inferMode);

    ~OnnxInfer();

    bool Infer(void *pData, AimBox &aimbox);

private:
    char InferMode;
    int Width, Height;
    Ort::Env *Env;
    Ort::SessionOptions *SessionOptions;
    Ort::Session *Session;
    float *DataBuffer;
    std::vector<Ort::Value> InputTensors;
};

#endif //FPSHELPER_ONNXINFER_H
