message(STATUS "Building Bee8080...")
add_subdirectory(8080/Bee8080)

message(STATUS "Building BeeZ80...")
add_subdirectory(z80/BeeZ80)

set(INTERFACE_HEADERS
	intel8080.h
	zilogz80.h)

add_library(cpu INTERFACE)
target_sources(cpu INTERFACE ${INTERFACE_HEADERS})
target_include_directories(cpu INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cpu INTERFACE libbee8080 libbeez80)