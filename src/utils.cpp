#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <math.h>

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
    std::string sub;
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
    control.left = control.left + abs(int64_t((numprocs-1)*cwidth/2-control.left))*ratio;
    control.top = control.top + abs(int64_t(cheight/2-control.top))*ratio;
}

int Utils::resetParameters(int64_t w, int64_t h, int cwidth, int cheight, int numprocs, Control_t& control)
{
    control.downsample = 1.0 * w / ((numprocs-1)*cwidth);
    if (control.downsample < 1.0)
        control.downsample = 1.0;
    control.left = ((numprocs-1)*cwidth - w/control.downsample) / 2;
    control.top = (cheight - h/control.downsample) / 2;
}

unsigned char* Utils::loadJPEG(const char* filename, unsigned int &width, unsigned int &height)
{
    unsigned char* rowptr[1];    // pointer to an array
    struct jpeg_decompress_struct info; //for our jpeg info
    struct jpeg_error_mgr err;          //the error handler
    int channels;

    FILE* file = fopen(filename, "rb");  //open the file

    info.err = jpeg_std_error(& err);     
    jpeg_create_decompress(& info);   //fills info structure

    //if the jpeg file doesn't load
    if(!file)
        return NULL;

    jpeg_stdio_src(&info, file);    
    jpeg_read_header(&info, TRUE);   // read jpeg file header

    jpeg_start_decompress(&info);    // decompress the file

    width = info.output_width;
    height = info.output_height;
    channels = info.num_components;
    int data_size = width * height * 3;
    
    unsigned char* pixels = (unsigned char *)malloc(data_size);
    while (info.output_scanline < info.output_height) // loop
    {
        // Enable jpeg_read_scanlines() to fill our jdata array
        rowptr[0] = (unsigned char *)pixels +  // secret to method
                3 * info.output_width * info.output_scanline; 
        jpeg_read_scanlines(&info, rowptr, 1);
    }

    jpeg_finish_decompress(&info);   //finish decompressing

    fclose(file);                    //close the file

    return pixels;
}

JPEG_t* Utils::loadJPEG(const char* filename)
{
    JPEG_t* jpeg = new JPEG_t;
    unsigned char* pixels;
    pixels = loadJPEG(filename, jpeg->width, jpeg->height);
    if(!pixels)
        return NULL;
    jpeg->data_size = jpeg->width*jpeg->height*3;
    return jpeg;
}

int Utils::writeJPEG (unsigned char* pixels, int w, int h, const char * filename, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE * outfile;               
    unsigned char* row_pointer[1];      
    int row_stride = w*3;        

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        cout << "Cannot open file " << filename << " to write" << endl;
        cout << stderr << endl;
        return -1;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = w;
    cinfo.image_height = h;
    cinfo.input_components = 3;  
    cinfo.in_color_space = JCS_RGB;       

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = pixels + cinfo.next_scanline * row_stride;
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);

    jpeg_destroy_compress(&cinfo);

    return 0;
}