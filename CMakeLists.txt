cmake_minimum_required(VERSION 3.22)
project(FpsHelper)

set(CMAKE_CXX_STANDARD 20)

# import opencv library
find_package(OpenCV REQUIRED PATHS "E:/Library/OpenCV/build")
include_directories(${OpenCV_INCLUDE_DIRS})
# import onnxruntime library
find_package(OnnxRuntime REQUIRED PATHS "E:/Library/OnnxRuntime")
include_directories(${OnnxRuntime_INCLUDE_DIRS})
message(STATUS "OnnxRuntime version: ${OnnxRuntime_VERSION}")
# import tensorrt library
find_package(TensorRT REQUIRED PATHS "E:/Library/TensorRT")
include_directories(${TensorRT_INCLUDE_DIRS})
message(STATUS "TensorRT version: ${TensorRT_VERSION}")
# import cuda
set(CUDAToolkit_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
find_package(CUDAToolkit REQUIRED)
include_directories(${CUDAToolkit_INCLUDE_DIRS})
message(STATUS "CUDAToolkit version: ${CUDAToolkit_VERSION}")

add_executable(FpsHelper
        "src/capture/DXGICapture.cpp"
        "src/capture/DXGICapture.h"
        "src/inference/TRTInfer.cpp"
        "src/inference/TRTInfer.h"
        "src/core/pipeline.h"
        "src/core/pipeline.cpp"
        "src/paint/GDIPaint.cpp"
        "src/paint/GDIPaint.h")
# CUDA::cudart
target_link_libraries(FpsHelper ${OpenCV_LIBS} ${TensorRT_LIBS} CUDA::cudart)

add_executable(captureTest
        "src/capture/DXGICapture.cpp"
        "src/capture/DXGICapture.h"
        "src/test/captureTest.cpp")
target_link_libraries(captureTest ${OpenCV_LIBS})

add_executable(inferenceTest
        "src/test/onnxTest.cpp"
        "src/inference/OnnxInfer.cpp"
        "src/inference/OnnxInfer.h")
target_link_libraries(inferenceTest ${OpenCV_LIBS} ${OnnxRuntime_LIBS})

add_executable(paintTest
        "src/test/paintTest.cpp"
        "src/paint/GDIPaint.cpp"
        "src/paint/GDIPaint.h")

add_executable(trtTest
        "src/inference/TrtInfer.cpp"
        "src/inference/TrtInfer.h"
        "src/test/trtTest.cpp")
target_link_libraries(trtTest ${OpenCV_LIBS} ${TensorRT_LIBS} CUDA::cudart)