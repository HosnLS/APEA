//
// Created by HosnLS on 2022/7/24.
//

#ifndef FPSHELPER_NNDTINFER_H
#define FPSHELPER_NNDTINFER_H

#include <onnxruntime_cxx_api.h>

#include "../core/pipeline.h"

class NndtInfer {
public:
    NndtInfer(const ORTCHAR_T* model_path, int w, int h, char inferMode);
    ~NndtInfer();
    bool Infer(void* pData, AimBox& aimbox);

private:
    char InferMode;
    int Width, Height;
    Ort::Env *Env;
    Ort::SessionOptions *SessionOptions;
    Ort::Session *Session;
    float* DataBuffer;
    std::vector<Ort::Value> InputTensors;
};

#endif //FPSHELPER_NNDTINFER_H
