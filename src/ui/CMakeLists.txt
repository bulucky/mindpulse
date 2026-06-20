if(WIN32)
    set(PLATFORM_UI_SOURCES src/tray_win.cpp)
else()
    set(PLATFORM_UI_SOURCES src/tray_stub.cpp)
endif()

add_library(ui STATIC ${PLATFORM_UI_SOURCES})

target_include_directories(ui PUBLIC include)


