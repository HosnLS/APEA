cmake_minimum_required(VERSION 3.22)
project(FpsHelper)

set(CMAKE_CXX_STANDARD 20)

# import onnxruntime library
find_package(OnnxRuntime REQUIRED PATHS "E:/Library/onnxruntime-1.12.0")
include_directories(${OnnxRuntime_INCLUDE_DIRS})
# import opencv library
find_package(OpenCV REQUIRED PATHS "E:/Library/OpenCV/build")
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(FpsHelper
        "src/inference/NndtInfer.cpp"
        "src/inference/NndtInfer.h"
        "src/capture/DXGICapture.cpp"
        "src/capture/DXGICapture.h"
        "src/core/pipeline.h"
        "src/core/pipeline.cpp"
        "src/paint/GDIPaint.cpp"
        "src/paint/GDIPaint.h")
#target_include_directories(FpsHelper PRIVATE ${OnnxRuntime_INCLUDE_DIRS})
target_link_libraries(FpsHelper ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES})

add_executable(captureTest
        "src/capture/DXGICapture.cpp"
        "src/capture/DXGICapture.h"
        "src/test/captureTest.cpp")
target_link_libraries(captureTest ${OpenCV_LIBRARIES})

add_executable(inferenceTest
        "src/test/inferTest.cpp"
        "src/inference/NndtInfer.cpp"
        "src/inference/NndtInfer.h")
target_link_libraries(inferenceTest ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES})

add_executable(paintTest
        "src/test/paintTest.cpp"
        "src/paint/GDIPaint.cpp"
        "src/paint/GDIPaint.h")