add_subdirectory(cpu)
add_subdirectory(machine)

add_library(components INTERFACE)
target_include_directories(components INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(components INTERFACE cpu machine)