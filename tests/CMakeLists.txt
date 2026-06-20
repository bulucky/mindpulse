set(GTEST_SOURCES
    test_main.cpp
    test_breath_engine_test.cpp
    test_icon_renderer_test.cpp
    test_config_manager.cpp
    test_tray_integration.cpp
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
    yaml-cpp
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(mindpulse_tests)