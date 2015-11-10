SET( srcs 
		math/vector3.cpp
		math/matrix4.cpp 
		utils.cpp
		glutils.cpp
		ssquad.cpp
		texture.cpp
		shader.cpp
		shaderlibrary.cpp
		tinyxml2.cpp
		pyramid.cpp 
		display.cpp
		pyramid.cpp
		)

# Build common
add_library( vsviewer_lib SHARED ${srcs} )
target_link_libraries( vsviewer_lib ${ALL_LIBS} )
