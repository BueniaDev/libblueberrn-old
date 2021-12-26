set(MB88XX_HEADER
	mb88xx.h)

set(MB88XX_SOURCE
	mb88xx.cpp)

set(MB88XX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mb88xx ${MB88XX_SOURCE} ${MB88XX_HEADER})
target_include_directories(mb88xx PUBLIC ${MB88XX_INCLUDE_DIR})
add_library(libmb88xx ALIAS mb88xx)