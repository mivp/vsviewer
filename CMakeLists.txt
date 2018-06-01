cmake_minimum_required(VERSION 2.8.3) 

project(vsviewer)

message("CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

# CHECK
if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

include(FindPkgConfig)

# OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

# OpenMPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# Omicron
#include(ExternalProject)
#include(${CMAKE_SOURCE_DIR}/external/UseOmicron.cmake)
#include_directories(${OMICRON_INCLUDE_DIRS})
include_directories(/cave/omegalib/omegalib13.1-cxx11/src/omicron/include /cave/omegalib/omegalib13.1-cxx11/build/src/omicron/omicron/include)
link_directories(/cave/omegalib/omegalib13.1-cxx11/build/bin) 

# libjpeg
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

# Set output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set shader path
add_definitions(-DSHADER_DIR=\"${CMAKE_SOURCE_DIR}/data/shaders/\")

file (COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/bin )

include_directories(
	external/glfw-3.0.3/include/GLFW/
	external/glew-1.9.0/include/
	src/
	src/math/
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_303
	GLEW_190
	${MPI_LIBRARIES}
	#${OMICRON_LIB}
	-lomicron
	${JPEG_LIBRARIES}
)

# Compile apps
add_subdirectory (external)

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
add_subdirectory (src)
add_subdirectory (src/dzviewer)
#add_subdirectory (src/vsviewer)
