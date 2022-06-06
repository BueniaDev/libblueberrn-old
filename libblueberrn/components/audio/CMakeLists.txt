message(STATUS "Building BeePSG...")
add_subdirectory(psg/BeePSG)
add_subdirectory(fm/BeeNuked)

set(INTERFACE_HEADERS
	ay8910.h
	ym2151.h)

add_library(audio INTERFACE)
target_sources(audio INTERFACE ${INTERFACE_HEADERS})
target_include_directories(audio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(audio INTERFACE libbeepsg libbeenuked)