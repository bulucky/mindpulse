set(SERVER_SOURCES
    src/server.cpp
)

add_library(server STATIC ${SERVER_SOURCES})

target_include_directories(server PUBLIC include)

target_link_libraries(server PRIVATE 
    core
    httplib::httplib
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_compile_definitions(server PUBLIC _WIN32_WINNT=0x0A00)
endif()
