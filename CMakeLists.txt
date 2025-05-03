cmake_minimum_required(VERSION 3.10)
project(ContactMe C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)  # Enforce static linking globally

# Windows-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

    # Suppress MinGW pragma warnings
    if(NOT MSVC)
        add_compile_options(-Wno-unknown-pragmas)
    endif()
endif()

# Fetch and configure cJSON
include(FetchContent)
FetchContent_Declare(
        cJSON
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG v1.7.15
)

# Disable unnecessary cJSON features
set(ENABLE_CJSON_STRICT OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(CJSON_USE_LOCALES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cJSON)

# Add main executable
add_executable(ContactMe main.c)

# Include headers
target_include_directories(ContactMe PRIVATE
        ${cJSON_SOURCE_DIR}
)

# Link statically to libraries
target_link_libraries(ContactMe PRIVATE
        ws2_32
        cjson
)

# Set reasonable compiler warnings
if(MSVC)
    target_compile_options(ContactMe PRIVATE /W4)
else()
    target_compile_options(ContactMe PRIVATE -Wall -Wextra)
endif()