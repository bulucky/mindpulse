set(GTEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/breath_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/icon_renderer.cpp
    test_main.cpp
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
    gtest
    gtest_main
)

include(GoogleTest)

gtest_discover_tests(mindpulse_tests) 