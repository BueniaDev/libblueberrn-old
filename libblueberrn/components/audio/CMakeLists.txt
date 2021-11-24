add_subdirectory(wsg3)
set(INTERFACE_HEADERS
	${WSG3_HEADERS}
	"wsg3.h")

add_library(audio INTERFACE)
target_sources(audio INTERFACE ${INTERFACE_HEADERS})
target_include_directories(audio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})