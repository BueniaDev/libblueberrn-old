project(example)

add_subdirectory(external)

find_package(SDL2 REQUIRED)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DSDL_MAIN_HANDLED")

include_directories(${LIBBLUEBERRN_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

add_executable(example ${EXAMPLE_SOURCES})

target_link_libraries(example libblueberrn imgui zip)

if (TARGET SDL2::SDL2)
    target_link_libraries(example SDL2::SDL2)
else()
    target_link_libraries(example ${SDL2_LIBRARIES})
endif()
