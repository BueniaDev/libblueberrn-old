add_subdirectory(galaxian)
set(INTERFACE_HEADERS
	${GALAXIAN_HEADERS}
	"galaxian.h")

add_library(video INTERFACE)
target_sources(video INTERFACE ${INTERFACE_HEADERS})
target_include_directories(video INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})