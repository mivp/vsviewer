#Virtual slide viewer for CAVE2#

##Log##
* v1.0.01: DeepZoom input

##Required packages##
* openmpi (http://www.open-mpi.org/)

##How to compile##
Tested on MacOS 10.10 and OpenSuSE 12.3
* Go to vsviewer directory
* Create build directory
* Go to build directory
* run: cmake ..
* run: make

##How to run dzviewer##
* run: GO_DESKTOP_DZ [-i dzi_file] [-l dzi_file_l dzi_file_r]
* run: GO_CAVE2_DZ [-i dzi_file] [-l dzi_file_l dzi_file_r]
* check GO_DESKTOP_DZ_DEMO and GO_CAVE2_DZ_DEMO for examples

##Usage##
* keyboard: use LEFT, RIGHT, UP, DOWN to pan; PAGE_UP/PAGE_DOWN to zoom; 'n' to go to next image
* PS3 move controller: use LEFT, RIGHT, UP, DOWN button to pan; X/O buttons to zoom; L1 button to go to next image

##Note: Convert to PyramidalTIFF with ImageMagick##
convert input.tiff -compress jpeg -quality 90 -define tiff:tile-geometry=256x256 ptif:output.tiff

##Note: Convert to virtual slide to DeepZoom with Vips##
vips dzsave 3755.svs 3755 --tile-size 1024 --depth onetile