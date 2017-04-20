# Virtual slide viewer for CAVE2

## Requisition

* [openmpi](http://www.open-mpi.org/)

## How to compile

Tested on MacOS 10.10 and OpenSuSE 12.3
```
cd vsviewer
mkdir build
cd build
cmake ..
make
```

## How to run dzviewer

```
GO_DESKTOP_DZ [-b buffersize] [-t numthreads] [-i dzi_file] [-l dzi_file_l dzi_file_r]
GO_CAVE2_DZ [-b buffersize] [-t numthreads] [-i dzi_file] [-l dzi_file_l dzi_file_r]
```
* default values: buffersize = 16, numthreads = 2
* check GO_DESKTOP_DZ_DEMO and GO_CAVE2_DZ_DEMO for examples

## Usage

- keyboard: use LEFT, RIGHT, UP, DOWN to pan; PAGE_UP/PAGE_DOWN to zoom; 'n'/'b' to go to next/previous image
- PS3 move controller: use LEFT, RIGHT, UP, DOWN button to pan; X/O buttons to zoom; L1 button to go to next image

## Note: Convert to PyramidalTIFF with ImageMagick

- convert input.tiff -compress jpeg -quality 90 -define tiff:tile-geometry=256x256 ptif:output.tiff

## Note: Convert to virtual slide to DeepZoom with Vips

- vips dzsave 3755.svs 3755 --tile-size 1024 --depth onetile

## Log

* v1.1.0: async image reading
* v1.0.0: DeepZoom input
