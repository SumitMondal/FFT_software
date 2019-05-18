cmake_minimum_required(VERSION 2.8)
project(P2)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        Complex.cc
        Complex.h
        InputImage.cc
        InputImage.h
        fft2d.cc
        )
add_executable(P2 ${SOURCE_FILES})