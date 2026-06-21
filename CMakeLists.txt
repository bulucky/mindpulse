cmake_minimum_required(VERSION 3.20)

project(mindpulse
    VERSION   0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.9.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.x
)

# httplib non-blocking resolver requires GetAddrInfoExCancel, which is
# missing from MinGW headers. MindPulse is localhost-only — blocking is fine.
set(HTTPLIB_USE_NON_BLOCKING_GETADDRINFO OFF)
FetchContent_MakeAvailable(httplib json yaml-cpp)

# googletest and tests/ are only available in dev environments (excluded from release builds by .gitignore)
if(EXISTS ${CMAKE_SOURCE_DIR}/tests AND EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(src)

