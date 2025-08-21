cmake_minimum_required(VERSION 3.16)
project(VirtualRadioSource LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)

# Adjust these paths
set(FFMPEG_ROOT "C:/ffmpeg")
set(BASECLASSES_ROOT "C:/SDK/baseclasses")

include_directories(
    ${BASECLASSES_ROOT}
    ${FFMPEG_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
    ${FFMPEG_ROOT}/lib
)

add_library(VirtualRadioSource SHARED
    src/dllmain.cpp
    src/Register.cpp
    src/VirtualRadioSource.cpp
    src/FfmpegDecoder.cpp
    src/RingBuffer.cpp
    resources/VirtualRadioSource.rc
)

target_link_libraries(VirtualRadioSource
    strmbase.lib              # built from BaseClasses (Debug: strmbasd.lib)
    avformat.lib avcodec.lib avutil.lib swresample.lib
    ws2_32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib shlwapi.lib
)

set_target_properties(VirtualRadioSource PROPERTIES
    OUTPUT_NAME "VirtualRadioSource"
)
