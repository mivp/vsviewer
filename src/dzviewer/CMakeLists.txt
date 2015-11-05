SET( srcs 
		pyramid.cpp
		dzdisplay.cpp
		main.cpp
		)

# Build
add_executable( dzviewer ${srcs} )
target_link_libraries( dzviewer vsviewer_lib ${ALL_LIBS} )