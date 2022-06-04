cmake_minimum_required(VERSION 3.19)
project(Yandex_praktikum_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        Yandex_praktikum_cpp
        main.cpp
        geo.h
        json.h
        json.cpp
        svg.h
        svg.cpp
        transport_catalogue.h
        transport_catalogue.cpp
        domain.h domain.cpp json_reader.h json_reader.cpp request_handler.h request_handler.cpp geo.cpp)