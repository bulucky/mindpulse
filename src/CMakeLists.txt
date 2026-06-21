set(MAIN_SOURCES
    main.cpp
)

# Windows resource file: embeds the application icon into the .exe
if(WIN32)
    list(APPEND MAIN_SOURCES mindpulse.rc)
endif()

add_executable(${PROJECT_NAME} WIN32
    ${MAIN_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ui
    core
    server
    yaml-cpp
    httplib::httplib
    nlohmann_json::nlohmann_json
)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
    # MinGW headers lack GetAddrInfoExCancel — disable httplib non-blocking resolver
    if(MINGW)
        add_compile_definitions(CPPHTTPLIB_NO_NON_BLOCKING_GETADDRINFO)
    endif()
endif()

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc -static-libstdc++ -static
    )
endif()

add_subdirectory(ui)
add_subdirectory(core)
add_subdirectory(server)
