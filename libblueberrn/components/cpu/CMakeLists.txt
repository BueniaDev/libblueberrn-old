add_subdirectory(8080/Bee8080)
set(INTERFACE_HEADERS
	"intel8080.h")

add_library(cpu INTERFACE ${INTERFACE_HEADERS})
target_include_directories(cpu INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cpu INTERFACE bee8080)