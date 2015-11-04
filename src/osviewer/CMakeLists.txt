# OpenSlide
PKG_SEARCH_MODULE(OPENSLIDE REQUIRED openslide)
include_directories(${OPENSLIDE_INCLUDE_DIRS})

SET( srcs 
		vsdisplay.cpp
		dzdisplay.cpp
		main.cpp
		)

# Build common
add_executable( vsviewer ${srcs} )
target_link_libraries( vsviewer vsviewer_lib ${ALL_LIBS} ${OPENSLIDE_LIBRARIES} )
