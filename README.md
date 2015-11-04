#Virtual slide viewer for CAVE2#

##Required packages##
- openmpi (http://www.open-mpi.org/)

##How to compile##
Tested on MacOS 10.10 and OpenSuSE 12.3
- Go to vsviewer directory
- Create build directory
- Go to build directory
- run: cmake ..
- run: make

##How to run dzviewer##
- go to build/bin directory
- run: ./GO_DESKTOP_DZ [-i dzi_file] [-l dzi_file_l dzi_file_r]
- run: ./GO_CAVE2_DZ [-i dzi_file] [-l dzi_file_l dzi_file_r]
- check GO_DESKTOP_DZ_DEMO and GO_CAVE2_DZ_DEMO for examples

##Tips: Convert to PyramidalTIFF with ImageMagick##
convert input.tiff -compress jpeg -quality 90 -define tiff:tile-geometry=256x256 ptif:output.tiff

##Tips: Convert to virtual slide to DeepZoom with Vips##
vips dzsave 3755.svs 3755 --tile-size 1024 --depth onetile