message(STATUS "Building Bee8080...")
add_subdirectory(8080/Bee8080)

message(STATUS "Building BeeZ80...")
add_subdirectory(z80/BeeZ80)

message(STATUS "Building Bee6502...")
add_subdirectory(m6502/Bee6502)

message(STATUS "Building Botsashi...")
add_subdirectory(m68k/Botsashi)

message(STATUS "Building Botnami...")
add_subdirectory(m680x/Botnami)

add_subdirectory(mb88xx)

set(INTERFACE_HEADERS
	intel8080.h
	zilogz80.h
	mos6502.h
	motorola68k.h
	motorola6809.h
	hitachi6309.h
	konami1.h
	konami2.h
	mb88xx.h)

add_library(cpu INTERFACE)
target_sources(cpu INTERFACE ${INTERFACE_HEADERS})
target_include_directories(cpu INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cpu INTERFACE libbee8080 libbeez80 libbee6502)
target_link_libraries(cpu INTERFACE libmb88xx libbotsashi libbotnami)