add_subdirectory(mb14241)
set(INTERFACE_HEADERS
	${MB14241_HEADERS}
	"mb14241.h")

add_library(machine INTERFACE ${INTERFACE_HEADERS})
target_include_directories(machine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})