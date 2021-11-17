project(example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external)

find_package(SDL2 REQUIRED)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")

include_directories(${LIBBLUEBERRN_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

add_executable(example ${EXAMPLE_SOURCES})

target_link_libraries(example libblueberrn imgui zip toml cmixer)

if (TARGET SDL2::SDL2)
    target_link_libraries(example SDL2::SDL2)
else()
    target_link_libraries(example ${SDL2_LIBRARIES})
endif()

if (APPLE)
    set_target_properties(example PROPERTIES
                         MACOSX_BUNDLE true
                         MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/blueberrn.plist.in
                         OUTPUT_NAME example)
endif()
