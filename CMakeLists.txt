if (WIN32)
    cmake_minimum_required(VERSION 3.12)
elseif (APPLE)
    cmake_minimum_required(VERSION 3.17)
elseif (UNIX)
    cmake_minimum_required(VERSION 3.17)
endif ()
project(RapidOcrOnnx)

include("VC-LTL helper for cmake.cmake")
# Output BIN JNI CLIB
if (NOT DEFINED OCR_OUTPUT)
    set(OCR_OUTPUT "BIN")
    message(STATUS "No OCR_OUTPUT, defaulting to BIN")
endif ()
option(OCR_BENCHMARK "build benchmark" ON)
set(OCR_BENCHMARK ON)
#set(OCR_OUTPUT "BIN")

if (NOT DEFINED OCR_ONNX)
    set(OCR_ONNX "CPU")
    message(STATUS "No OCR_ONNX, defaulting to CPU")
endif ()
#set(OCR_OUTPUT "GPU")

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DUNICODE -D_UNICODE)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-Wall -g -O0")
else ()
    add_definitions("-Wall")
endif ()

# OnnxRuntime
if (OCR_ONNX STREQUAL "CPU")
    include(${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-static/OnnxRuntimeWrapper.cmake)
elseif (OCR_ONNX STREQUAL "CUDA") # CUDA
    include(${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-gpu/OnnxRuntimeWrapper.cmake)
endif ()
find_package(OnnxRuntime REQUIRED)
if (OnnxRuntime_FOUND)
    message(STATUS "OnnxRuntime_LIBS: ${OnnxRuntime_LIBS}")
    message(STATUS "OnnxRuntime_INCLUDE_DIRS: ${OnnxRuntime_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "onnxruntime Not Found!")
endif (OnnxRuntime_FOUND)

# OpenCV
set(BUILD_SHARED_LIBS false)
# include(${CMAKE_CURRENT_SOURCE_DIR}/opencv-static/OpenCVWrapperConfig.cmake)
# find_package(OpenCV REQUIRED)
# if (OpenCV_FOUND)
#     message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
#     message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
# else ()
#     message(FATAL_ERROR "opencv Not Found!")
# endif (OpenCV_FOUND)
if (CMAKE_CL_64)
    message("配置WINDOWS OpenCV x64 路径: ${CMAKE_CURRENT_LIST_DIR}/opencv-static/windows-x64")
    set(OpenCV_DIR "${CMAKE_CURRENT_LIST_DIR}/opencv-static/windows-x64")
    link_directories(${OpenCV_DIR}/x64/vc16/staticlib)
    include(${OpenCV_DIR}/x64/vc16/staticlib/OpenCVConfig.cmake)

else ()
    message("配置WINDOWS OpenCV x86 路径: ${CMAKE_CURRENT_LIST_DIR}/opencv-static/windows-x86")
    set(OpenCV_DIR "${CMAKE_CURRENT_LIST_DIR}/opencv-static/windows-x86")
    link_directories(${OpenCV_DIR}/x86/vc16/staticlib)
    include(${OpenCV_DIR}/x86/vc16/staticlib/OpenCVConfig.cmake)
endif ()

include_directories(${OpenCV_DIR}/include)
set(OpenCV_LIBS opencv_core;opencv_features2d;opencv_flann;opencv_highgui;opencv_imgcodecs;opencv_imgproc;opencv_photo;opencv_video)

# project include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# source
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(OCR_COMPILE_CODE ${OCR_SRC})

# JNI
if (OCR_OUTPUT STREQUAL "JNI")
    find_package(JNI REQUIRED)
    if (JNI_FOUND)
        message("JNI FOUND")
        message(STATUS "JNI_LIBS: ${JNI_LIBS}")
        message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
        include_directories(${JNI_INCLUDE_DIRS})
    else ()
        message(FATAL_ERROR "JNI Not Found!")
    endif ()
endif ()

if (OCR_OUTPUT STREQUAL "JNI") # JNI
    add_library(RapidOcrOnnx SHARED ${OCR_COMPILE_CODE})
    target_compile_definitions(RapidOcrOnnx PRIVATE __JNI__)
    target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS} ${JNI_LIBS})
elseif (OCR_OUTPUT STREQUAL "CLIB") # CLIB
    add_library(RapidOcrOnnx SHARED ${OCR_COMPILE_CODE})
    target_compile_definitions(RapidOcrOnnx PRIVATE __CLIB__)
    target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})
elseif (OCR_OUTPUT STREQUAL "BIN") # BIN
    add_executable(RapidOcrOnnx ${OCR_COMPILE_CODE})
    target_compile_definitions(RapidOcrOnnx PRIVATE __EXEC__)
    target_link_libraries(RapidOcrOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS})
endif ()

install(TARGETS RapidOcrOnnx EXPORT RapidOcrOnnx)
if (OCR_OUTPUT STREQUAL "CLIB") # CLIB
    file(GLOB OCR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    install(FILES ${OCR_INCLUDE} DESTINATION include)
endif ()

if (OCR_ONNX STREQUAL "CUDA")
    target_compile_definitions(RapidOcrOnnx PRIVATE __CUDA__)
endif ()

# benchmark
if (OCR_BENCHMARK AND (OCR_OUTPUT STREQUAL "BIN"))
    add_executable(benchmark benchmark/benchmark.cpp
            src/clipper.cpp
            src/CrnnNet.cpp
            src/DbNet.cpp
            src/getopt.cpp
            src/OcrLite.cpp
            src/OcrUtils.cpp)
    target_link_libraries(benchmark ${OnnxRuntime_LIBS} ${OpenCV_LIBS})
    target_compile_definitions(benchmark PRIVATE __EXEC__)

    if (OCR_ONNX STREQUAL "CUDA")
        target_compile_definitions(benchmark PRIVATE __CUDA__)
    endif ()
    install(TARGETS benchmark EXPORT benchmark
            ARCHIVE DESTINATION staticlib
            LIBRARY DESTINATION sharedlib
            RUNTIME DESTINATION bin)
endif ()

# Windows Link CRT
if (OCR_BUILD_CRT STREQUAL "True")
    include(${CMAKE_CURRENT_SOURCE_DIR}/OcrCRTLinkage.cmake)
endif ()


if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set_target_properties(RapidOcrOnnx PROPERTIES OUTPUT_NAME "LunaOCR64")
else()
	
	set_target_properties(RapidOcrOnnx PROPERTIES OUTPUT_NAME "LunaOCR32")
endif()