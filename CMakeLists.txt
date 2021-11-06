cmake_minimum_required(VERSION 3.19)
project(Yandex_praktikum_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(Yandex_praktikum_cpp main.cpp array_ptr.h log_duration.h document.h paginator.h process_queries.h process_queries.cpp read_input_functions.h read_input_functions.cpp request_queue.cpp request_queue.h search_server.cpp search_server.h string_processing.cpp string_processing.h test_example_functions.cpp test_example_functions.h)