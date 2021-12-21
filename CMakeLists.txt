cmake_minimum_required(VERSION 3.19)
project(Yandex_praktikum_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(Yandex_praktikum_cpp main.cpp log_duration.h geo.h input_reader.h stat_reader.h transport_catalogue.h input_reader.cpp stat_reader.cpp transport_catalogue.cpp)