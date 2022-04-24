find_package(SDL2 REQUIRED)

set(ZIP_SOURCES
	zip/zip.c)

set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_tables.cpp
	imgui_sdl/imgui_sdl.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
	imgui/
	imgui/backends/
	imgui_sdl/
	${SDL2_INCLUDE_DIRS})

if (WIN32)
    target_link_libraries(imgui PUBLIC imm32)
endif()

if (TARGET SDL2::SDL2)
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
else()
    target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
endif()

add_library(zip STATIC ${ZIP_SOURCES})

target_include_directories(zip PUBLIC
	zip/)

add_library(toml INTERFACE)
target_include_directories(toml INTERFACE toml/)