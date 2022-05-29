cmake_minimum_required(VERSION 3.19)
project(Yandex_praktikum_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        Yandex_praktikum_cpp
        main.cpp
        geo.h
        input_reader.h
        input_reader.cpp
        json.h
        json.cpp
        stat_reader.h
        stat_reader.cpp
        svg.h
        svg.cpp
        transport_catalogue.h
        transport_catalogue.cpp
)