add_subdirectory(components)

set(LIBBLUEBERRN
	libblueberrn/src/core/libblueberrn.cpp
	libblueberrn/src/core/scheduler.cpp
	libblueberrn/src/drivers/example.cpp
	libblueberrn/src/drivers/invaders.cpp
	libblueberrn/src/drivers/galaxian.cpp
	libblueberrn/src/drivers/pacman.cpp
	libblueberrn/src/drivers/system1.cpp
	PARENT_SCOPE)

set(LIBBLUEBERRN_HEADERS
	libblueberrn/include/libblueberrn/libblueberrn_api.h
	libblueberrn/include/libblueberrn/driver.h
	libblueberrn/include/libblueberrn/scheduler.h
	libblueberrn/include/libblueberrn/driverlist.h
	libblueberrn/include/libblueberrn/libblueberrn.h
	libblueberrn/include/drivers/berrndrivers.h
	libblueberrn/include/drivers/example.h
	libblueberrn/include/drivers/invaders.h
	libblueberrn/include/drivers/galaxian.h
	libblueberrn/include/drivers/pacman.h
	libblueberrn/include/drivers/system1.h
	PARENT_SCOPE)