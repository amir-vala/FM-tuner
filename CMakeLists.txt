cmake_minimum_required(VERSION 3.16)
project(VirtualTuner LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
add_definitions(-D_WIN32_WINNT=0x0601) # Target Windows 7

include_directories(
    include
)

add_executable(VirtualTuner
    src/main.cpp
    src/DiscoveryServer.cpp
)

target_link_libraries(VirtualTuner
    ws2_32
)

set_target_properties(VirtualTuner PROPERTIES
    OUTPUT_NAME "VirtualTuner"
)
