SET( srcs 
		osdisplay.cpp
		main.cpp
		)

# Build common
add_executable( osviewer ${srcs} )
target_link_libraries( osviewer vsviewer_lib ${ALL_LIBS} )
