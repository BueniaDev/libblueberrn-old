add_subdirectory(wsg)
set(AUDIO_HEADERS
	${WSG_HEADERS}
	libblueberrn/audio/wsg.h
	libblueberrn/audio/namco54.h
	PARENT_SCOPE)

set(AUDIO_SOURCES
	libblueberrn/audio/namco54.cpp
	PARENT_SCOPE)