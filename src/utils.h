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
};

#endif // UTILS_H
