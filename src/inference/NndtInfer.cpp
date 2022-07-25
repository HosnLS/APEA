//
// Created by HosnLS on 2022/7/24.
//


#include <cpu_provider_factory.h>
#include <tensorrt_provider_factory.h>

#include "NndtInfer.h"

NndtInfer::NndtInfer(const ORTCHAR_T *model_path, int w, int h, char inferMode) {
    InferMode = inferMode;
    Width = w;
    Height = h;

    Env = new Ort::Env(ORT_LOGGING_LEVEL_FATAL, "infer");
    SessionOptions = new Ort::SessionOptions();

    if (InferMode == 't')OrtSessionOptionsAppendExecutionProvider_Tensorrt(*SessionOptions, 0);
    else if (InferMode == 'g')OrtSessionOptionsAppendExecutionProvider_CUDA(*SessionOptions, 0);
    else OrtSessionOptionsAppendExecutionProvider_CPU(*SessionOptions, 0);
    SessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // nanodet format: BGR
    Session = new Ort::Session(*Env, model_path, *SessionOptions);

    Ort::MemoryInfo MemoryInfo = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                                            OrtMemType::OrtMemTypeDefault);
    DataBuffer = new float[Width * Height * 3];
    std::array<int64_t, 4> inputShape = {1, 3, Height, Width};
    InputTensors.emplace_back(Ort::Value::CreateTensor<float>(
            MemoryInfo,
            DataBuffer,
            Width * Height * 3,
            inputShape.data(),
            inputShape.size()
    ));
}

bool NndtInfer::Infer(void *pData, AimBox &aimbox) {
    for (int row = 0; row < Height; ++row) {
        for (int col = 0; col < Width; ++col) {
            DataBuffer[row * Width + col] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 0]) - 103.53f) / 57.375f;
            DataBuffer[row * Width + col + Width * Height] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 1]) - 116.28f) / 57.12f;
            DataBuffer[row * Width + col + Width * Height * 2] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 2]) - 123.675f) / 58.395f;
        }
    }

    std::vector<const char *> inputNames = {"data"};
    std::vector<const char *> outputNames = {"output"};
    std::vector<Ort::Value> outputTensors;
    outputTensors = Session->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(), //输入节点名
            InputTensors.data(),     //input tensors
            1,
            outputNames.data(), //输出节点名
            1
    );
    const float *outputData = outputTensors[0].GetTensorData<float>();

    int outputFeat = 0;
    std::array<int, 4> stride{8, 16, 32, 64};
    for(auto st: stride){
        int xm = (Width + st - 1) / st;
        int ym = (Height + st - 1) / st;
        outputFeat += xm * ym;
    }
    int maxInd = 0;
    float maxVal = outputData[0];
    for (int i = 0; i < outputFeat; ++i)
        if (outputData[i * 112] > maxVal) {
            maxVal = outputData[i * 112];
            maxInd = i;
        }
    float prob = 1 / (1 + exp(-maxVal));

    std::array<float, 32> box{};
    for (int i = 0; i < 32; ++i)
        box[i] = exp(outputData[maxInd * 112 + 80 + i]);
    for (int i = 0; i < 4; ++i) {
        float sum = 0;
        for (int j = 0; j < 8; ++j)
            sum += box[i * 8 + j];
        for (int j = 0; j < 8; ++j)
            box[i * 8 + j] /= sum;
    }
    std::array<float, 4> boxSmp{};
    for (int i = 0; i < 4; ++i) {
        boxSmp[i] = box[i * 8 + 0] * 0 + box[i * 8 + 1] * 1 + box[i * 8 + 2] * 2 + box[i * 8 + 3] * 3
                    + box[i * 8 + 4] * 4 + box[i * 8 + 5] * 5 + box[i * 8 + 6] * 6 + box[i * 8 + 7] * 7;
    }


    int curInd = maxInd;
    for(auto st: stride){
        int xm = (Width + st - 1) / st;
        int ym = (Height + st - 1) / st;
        if(curInd >= xm * ym)
            curInd -= xm * ym;
        else{
            aimbox.xl = round((curInd % xm - boxSmp[0]) * st);
            aimbox.xr = round((curInd % xm + boxSmp[2]) * st);
            aimbox.yl = round((curInd / xm - boxSmp[1]) * st);
            aimbox.yr = round((curInd / xm + boxSmp[3]) * st);
            aimbox.prob = prob;
            break;
        }
    }
    return true;
}

NndtInfer::~NndtInfer() {
    delete Session;
    delete SessionOptions;
    delete Env;
    delete[] DataBuffer;
}


