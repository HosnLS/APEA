//
// Created by HosnLS on 2022/11/4.
//

#ifndef FPSHELPER_TRTINFER_H
#define FPSHELPER_TRTINFER_H

#include "NvInferRuntimeCommon.h"
#include "NvInfer.h"

#include "../core/pipeline.h"

class TRTInfer {
private:
    int Width, Height, OutputChannel;
    nvinfer1::ICudaEngine *engine;
    nvinfer1::IExecutionContext *context;
    void *buffers[2];
    int inputIndex;
    int outputIndex;
    cudaStream_t *stream;
    float *InputBuffer, *OutputBuffer;
public:
    TRTInfer(const char *model_path, int w, int h);

    ~TRTInfer();

    bool Infer(void *pData, AimBox &aimbox);
};

#endif //FPSHELPER_TRTINFER_H
