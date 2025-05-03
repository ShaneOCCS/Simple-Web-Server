cmake_minimum_required(VERSION 3.0)
project(cJSON C)

option(ENABLE_CJSON_STRICT "Enable strict compilation" OFF)

add_library(cjson cJSON.c)
target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
