set(GTEST_SOURCES
    test_main.cpp
    test_state_machine.cpp
    test_config_manager.cpp
    test_tray_integration.cpp
    test_breath_engine_test.cpp
    test_icon_renderer_test.cpp
)

add_executable(mindpulse_tests
    ${GTEST_SOURCES}
)

target_include_directories(mindpulse_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(mindpulse_tests PRIVATE
    ui
    core
    gtest
    server
    yaml-cpp
    gtest_main
    httplib::httplib
    nlohmann_json::nlohmann_json
)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    target_link_libraries(mindpulse_tests PRIVATE ws2_32)
endif()

include(GoogleTest)
gtest_discover_tests(mindpulse_tests)