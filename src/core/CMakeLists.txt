set(CORE_SOURCES
    src/breath_engine.cpp
    src/icon_renderer.cpp
    src/config_manager.cpp
)

add_library(core STATIC ${CORE_SOURCES})

target_include_directories(core PUBLIC include)

target_link_libraries(core PRIVATE yaml-cpp)

