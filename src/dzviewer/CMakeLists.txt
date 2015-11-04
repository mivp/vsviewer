# libjpeg
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

SET( srcs 
		dzdisplay.cpp
		main.cpp
		)

# Build
add_executable( dzviewer ${srcs} )
target_link_libraries( dzviewer vsviewer_lib ${ALL_LIBS} ${JPEG_LIBRARIES} )