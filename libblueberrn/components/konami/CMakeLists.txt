message(STATUS "Building BeeKonami...")
add_subdirectory(konami/BeeKonami)

set(INTERFACE_HEADERS
	k052109.h
	k051962.h
	k007232.h)

add_library(konami INTERFACE)
target_sources(konami INTERFACE ${INTERFACE_HEADERS})
target_include_directories(konami INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(konami INTERFACE beekonami)