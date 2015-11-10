#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <iostream>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

extern "C" {
#include "jpeglib.h" 
}    
#include <jerror.h>

#include <openslide.h>

using namespace std;

/*!
 * Utils
 * @static
 */
struct Control_t
{
	double left;
	double top;
	double downsample;
	Control_t()
	{
		left = top = 0;
		downsample = 1;
	}
};

struct JPEG_t
{
    int level;
    int col, row;
    unsigned char* pixels;
    unsigned int width, height;
    unsigned int data_size;

    //for async loading
    bool ready;
    string path;
    int leftright; //0: left, 1: right
    JPEG_t()
    {
        leftright = 0;
        ready = false; pixels = NULL;
    }
};

class Utils
{
public:

    static char* getFileContent(string path);

    static int fileExist (const char *filename);

    static double random(double min, double max);

    static uint getTime();

    static string getParametersString(Control_t control, int mode = 0);

    static void parseParametersString(const char* buff, int& mode, int64_t& l, int64_t& t, double& ds);

    static void pan(int direction, double amount, Control_t& control);

    static void zoom(int64_t w, int64_t h, int cwidth, int cheight, int numprocs, double maxdownsample, double amount, Control_t& control);

    static int resetParameters(int64_t w, int64_t h, int cwidth, int cheight, int numprocs, Control_t& control);

    static JPEG_t* loadJPEG(const char* filename);

    static unsigned char* loadJPEG(const char* filename, unsigned int &width, unsigned int &height);

    static int writeJPEG (unsigned char* pixels, int w, int h, const char * filename, int quality);

    static unsigned char* loadTile(openslide_t *osr, int level, int row, int col, const int tilesize,
                                    unsigned int &width, unsigned int &height);
};

#endif // UTILS_H
