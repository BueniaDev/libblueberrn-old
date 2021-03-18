set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
add_executable(8080test main.cpp)
target_link_libraries(8080test libblueberrn)
