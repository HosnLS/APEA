//
// Created by HosnLS on 2022/11/4.
//
#include <iostream>
#include <fstream>
#include <array>

#include "TRTInfer.h"

using namespace nvinfer1;

class Logger : public ILogger
{
    void log(Severity severity, const char* msg) noexcept override
    {
        if (severity != Severity::kINFO)
            std::cout << msg << std::endl;
    }
} gLogger;

TRTInfer::TRTInfer(const char *model_path, int w, int h) {
    Width = w;
    Height = h;

    OutputChannel = 0;
    std::array<int, 4> stride{8, 16, 32, 64};
    for (auto st: stride) {
        int xm = (Width + st - 1) / st;
        int ym = (Height + st - 1) / st;
        OutputChannel += xm * ym;
    }
    // OutputFeat: 112;

    char* trtModelStream{ nullptr }; //指针函数,创建保存engine序列化文件结果
    size_t size{ 0 };

// read model from the engine file
    std::ifstream file(model_path, std::ios::binary);
    if (file.good()) {
        file.seekg(0, std::ifstream::end);
        size = file.tellg();
        file.seekg(0, std::ifstream::beg);
        trtModelStream = new char[size];
        file.read(trtModelStream, size);
        file.close();
    }
//    create a runtime (required for deserialization of model) with NVIDIA's logger
    Logger gLogger;
    IRuntime* runtime = createInferRuntime(gLogger); //反序列化方法
//    deserialize engine for using the char-stream
    engine = runtime->deserializeCudaEngine(trtModelStream, size, nullptr);
//    一个engine可以有多个execution context，并允许将同一套weights用于多个推理任务。
//     可以在并行的CUDA streams流中按每个stream流一个engine和一个context来处理图像。
//     每个context在engine相同的GPU上创建。
    runtime->destroy(); //顺道销毁runtime，释放内存

    context = engine->createExecutionContext(); // create execution context -- required for inference executions


    // Pointers to input and output device buffers to pass to engine.
    // Engine requires exactly IEngine::getNbBindings() number of buffers.

    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()
    inputIndex = engine->getBindingIndex("data");
    outputIndex = engine->getBindingIndex("output");

    // Create GPU buffers on device -- allocate memory for input and output
    cudaMalloc(&buffers[inputIndex], 1 * (3 * Width * Height) * sizeof(float));
    cudaMalloc(&buffers[outputIndex], OutputChannel * 112 * sizeof(float));

    // create CUDA stream for simultaneous CUDA operations
    stream = new cudaStream_t;
    cudaStreamCreate(stream);

    InputBuffer = new float[Width * Height * 3];
    OutputBuffer = new float[OutputChannel * 112];
}

TRTInfer::~TRTInfer() {
    delete OutputBuffer;
    delete InputBuffer;
    // Release stream and buffers (memory)
    cudaStreamDestroy(*stream);
    cudaFree(buffers[outputIndex]);
    cudaFree(buffers[inputIndex]);

    context->destroy();
}

bool TRTInfer::Infer(void *pData, AimBox &aimbox) {
    // Normalize data
    for (int row = 0; row < Height; ++row) {
        for (int col = 0; col < Width; ++col) {
            InputBuffer[row * Width + col] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 0]) - 103.53f) / 57.375f;
            InputBuffer[row * Width + col + Width * Height] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 1]) - 116.28f) / 57.12f;
            InputBuffer[row * Width + col + Width * Height * 2] =
                    (float(((unsigned char *) pData)[row * Width * 4 + col * 4 + 2]) - 123.675f) / 58.395f;
        }
    }


    // copy input from host (CPU) to device (GPU)  in stream
    cudaMemcpyAsync(buffers[inputIndex],
                    (void*)InputBuffer,
                    1 * (3 * Width * Height) * sizeof(float),
                    cudaMemcpyHostToDevice,
                    *stream);

    // execute inference using context provided by engine
    context->enqueueV2(buffers,
                     *stream,
                     nullptr);

    // copy output back from device (GPU) to host (CPU)
    cudaMemcpyAsync(OutputBuffer,
                    buffers[outputIndex],
                    OutputChannel * 112 * sizeof(float),
                    cudaMemcpyDeviceToHost,
                    *stream);

    // synchronize the stream to prevent issues (block CUDA and wait for CUDA operations to be completed)
    cudaStreamSynchronize(*stream);


    int maxInd = 0;
    float maxVal = OutputBuffer[0];
    for (int i = 0; i < OutputChannel; ++i)
        if (OutputBuffer[i * 112] > maxVal) {
            maxVal = OutputBuffer[i * 112];
            maxInd = i;
        }
    float prob = 1 / (1 + exp(-maxVal));

    std::array<float, 32> box{};
    for (int i = 0; i < 32; ++i)
        box[i] = exp(OutputBuffer[maxInd * 112 + 80 + i]);
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

    std::array<int, 4> stride{8, 16, 32, 64};
    int curInd = maxInd;
    for (auto st: stride) {
        int xm = (Width + st - 1) / st;
        int ym = (Height + st - 1) / st;
        if (curInd >= xm * ym)
            curInd -= xm * ym;
        else {
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
