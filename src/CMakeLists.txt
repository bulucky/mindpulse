if(WIN32)
    set(PLATFORM_UI_SOURCES ui/tray_win.cpp)
else()
    set(PLATFORM_UI_SOURCES ui/tray_stub.cpp)
endif()

set(MAIN_SOURCES
    main.cpp
    core/breath_engine.cpp
    core/icon_renderer.cpp
    ${PLATFORM_UI_SOURCES}
)

add_executable(${PROJECT_NAME}
    ${MAIN_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
