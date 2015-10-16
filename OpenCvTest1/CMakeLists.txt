cmake_minimum_required(VERSION 2.8.4)
project(OpenCvTest1)


set(SOURCE_FILES main.cpp)
find_package(OpenCV REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable(OpenCvTest1 ${SOURCE_FILES})

target_link_libraries(OpenCvTest1  ${OpenCV_LIBS} )
