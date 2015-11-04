#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

using namespace std;

char* Utils::getFileContent(string path)
{
    FILE* fp;
    char* content = NULL;
    long length;

    fp = fopen( path.c_str(), "rb" );
    if (fp)
    {
        fseek( fp, 0, SEEK_END );
        length = ftell( fp );
        fseek( fp, 0, SEEK_SET );
        content = new char [length+1];
        fread( content, sizeof( char ), length, fp );
        fclose( fp );
        content[length] = '\0';
    }

    return content;
}

int Utils::fileExist (const char *filename)
{
    struct stat buffer;   
    return (stat (filename, &buffer) == 0);
}

double Utils::random(double min, double max)
{
    return (rand() / static_cast<double>(RAND_MAX)) * (max - min) + min;
}

uint Utils::getTime()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

string Utils::getParametersString(Control_t control, int mode)
{
    std::ostringstream ss;
    ss << "U " << mode << " " << int64_t(control.left) << " " << int64_t(control.top) << " " << control.downsample;
    return ss.str();
}

void Utils::parseParametersString(const char* buff, int& mode, int64_t& l, int64_t& t, double& ds)
{
    istringstream iss(buff);
    string sub;
    iss >> sub; 
    iss >> sub; mode = atoi(sub.c_str());
    iss >> sub; l = atoi(sub.c_str());
    iss >> sub; t = atoi(sub.c_str());
    iss >> sub; ds = atof(sub.c_str());
}

void Utils::pan(int direction, double amount, Control_t& control)
{
    switch(direction)
    {
        case(0): // LEFT
            control.left -= amount;
            break;
        case(1): // RIGHT
            control.left += amount;
            break;
        case(2): // UP
            control.top -= amount;
            break;
        case(3):
            control.top += amount;
            break;
    }
}

void Utils::zoom(int64_t w, int64_t h, int cwidth, int cheight, int numprocs, double maxdownsample, double amount, Control_t& control)
{
    double prev_w = 1.0 * w / control.downsample;
    
    control.downsample += amount;
    if(control.downsample < 0.3)
        control.downsample = 0.3;
    if(control.downsample > maxdownsample)
        control.downsample = maxdownsample;

    double cur_w = 1.0 * w / control.downsample;
    double ratio = (prev_w-cur_w) / prev_w;
    control.left = control.left + abs((numprocs-1)*cwidth/2-control.left)*ratio;
    control.top = control.top + abs(cheight/2-control.top)*ratio;
}

int Utils::resetParameters(int64_t w, int64_t h, int cwidth, int cheight, int numprocs, Control_t& control)
{
    control.downsample = 1.0 * w / ((numprocs-1)*cwidth);
    if (control.downsample < 1.0)
        control.downsample = 1.0;
    control.left = ((numprocs-1)*cwidth - w/control.downsample) / 2;
    control.top = (cheight - h/control.downsample) / 2;
}
