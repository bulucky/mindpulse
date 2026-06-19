cmake_minimum_required(VERSION 3.20)

project(mindpulse
    VERSION   0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.48.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
)

# # MinGW 缺少 GetAddrInfoExCancel 声明，且本项目仅用 127.0.0.1 无需异步 DNS
# set(HTTPLIB_USE_NON_BLOCKING_GETADDRINFO OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(httplib json)

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)


if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc -static-libstdc++ -static
    )
endif()
