/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2015, Monash University
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
**       * Redistributions of source code must retain the above copyright notice,
**          this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the Monash University nor the names of its contributors
**         may be used to endorse or promote products derived from this software
**         without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%  Toan Nguyen- Toan.Nguyen(at)monash.edu
*%
*% Development Team :
*%  http://monash.edu/mivp
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "dzdisplay.h"

#include "utils.h"
#include "glutils.h"
#include "shaderlibrary.h"
#include "tinyxml2.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdio.h>
#include <memory.h>

using namespace std;
using namespace tinyxml2;

DZDisplay::DZDisplay(int i, int w, int h): Display(i, w, h)
{
	buffersize = 16;
	level_imgs1.clear();
	level_imgs2.clear();
}

DZDisplay::~DZDisplay()
{
	clearBuffer();
}

void DZDisplay::clearBuffer()
{
	for(list<JPEG_t*>::iterator it=level_imgs1.begin(); it != level_imgs1.end(); it++)
    {
        JPEG_t* jpeg = *it;
        free(jpeg->pixels);
    }
    level_imgs1.clear();

	for(list<JPEG_t*>::iterator it=level_imgs2.begin(); it != level_imgs2.end(); it++)
    {
        JPEG_t* jpeg = *it;
        free(jpeg->pixels);
    }
    level_imgs2.clear();
}

int DZDisplay::loadVirtualSlide(Img_t img)
{
	// clear buffer
	clearBuffer();

	// parse dzi file
	cout << "Parse dzi file: " << img.filename1 << endl;
	XMLDocument doc;
	doc.LoadFile( img.filename1.c_str() );
	int errorID = doc.ErrorID();
	if(errorID)
	{
		cout << "Fail to open and parse file: " << img.filename1 << endl;
		return -1;
	}
	XMLElement* imageElement = doc.FirstChildElement( "Image" );
	imageElement->QueryIntAttribute( "TileSize", &tilesize );

	double temp_w, temp_h;
	XMLElement* sizeElement = imageElement->FirstChildElement("Size");
	sizeElement->QueryDoubleAttribute( "Width", &temp_w );
	sizeElement->QueryDoubleAttribute( "Height", &temp_h );
	level0_w = int64_t(temp_w);
	level0_h = int64_t(temp_h);
	cout << "TileSize: " << tilesize << " Width: " << level0_w << " Height: " << level0_h << endl;

	stereo = false;
	if(img.type == 0)
	{
		datadir1 = img.filename1.substr(0, img.filename1.size() - 4);
		datadir1.append("_files/");
	}
	else if (img.type == 1)
	{
		datadir1 = img.filename1.substr(0, img.filename1.size() - 4);
		datadir1.append("_files/");
		datadir2 = img.filename2.substr(0, img.filename2.size() - 4);
		datadir2.append("_files/");
		stereo = true;
	}
	else if (img.type == 2)
	{
		datadir1 = img.filename2.substr(0, img.filename2.size() - 4);
		datadir1.append("_files/");
		datadir2 = img.filename1.substr(0, img.filename1.size() - 4);
		datadir2.append("_files/");
		stereo = true;
	}
	else
	{
		cout << "Invalid type" << endl;
		return -1;
	}
	cout << "datadir1: " << datadir1 << " datadir2: " << datadir2 << endl;
	
	maxdownsample = 2.0 * level0_w / 1024;

	// texture
	if(tex1 == NULL)
	{
		tex1 = Texture::newFromNextUnit(width, height, GL_RGB, GL_BGR);
		tex1->init();
	}

	if(stereo && tex2 == NULL)
	{
		tex2 = Texture::newFromNextUnit(width, height, GL_RGB, GL_BGR);
		tex2->init();
	}

	return 0;
}

JPEG_t* DZDisplay::loadJPEG(const char* filename)
{
	unsigned char* rowptr[1];    // pointer to an array
	struct jpeg_decompress_struct info; //for our jpeg info
	struct jpeg_error_mgr err;          //the error handler
	int channels;

	FILE* file = fopen(filename, "rb");  //open the file

	info.err = jpeg_std_error(& err);     
	jpeg_create_decompress(& info);   //fills info structure

	//if the jpeg file doesn't load
	if(!file) {
		cout << "(" << id << ") Error reading JPEG file: " << filename << endl << stderr << endl; 
	 	return NULL;
	}

	jpeg_stdio_src(&info, file);    
	jpeg_read_header(&info, TRUE);   // read jpeg file header

	jpeg_start_decompress(&info);    // decompress the file

	JPEG_t* jpeg = new JPEG_t;
	jpeg->width = info.output_width;
	jpeg->height = info.output_height;
	channels = info.num_components;
	jpeg->data_size = jpeg->width * jpeg->height * 3;
	
	jpeg->pixels = (unsigned char *)malloc(jpeg->data_size);
	//memset(jpeg->pixels, 0, jpeg->data_size);
	while (info.output_scanline < info.output_height) // loop
	{
		// Enable jpeg_read_scanlines() to fill our jdata array
		rowptr[0] = (unsigned char *)jpeg->pixels +  // secret to method
		        3 * info.output_width * info.output_scanline; 
		jpeg_read_scanlines(&info, rowptr, 1);
	}

	jpeg_finish_decompress(&info);   //finish decompressing

	fclose(file);                    //close the file

	return jpeg;
}

int DZDisplay::writeJPEG (unsigned char* pixels, int w, int h, const char * filename, int quality)
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

JPEG_t* DZDisplay::findOrLoadNewTile(int level, int col, int row, int index)
{
	if(index == 0)
	{
		for(list<JPEG_t*>::iterator it=level_imgs1.begin(); it != level_imgs1.end(); it++)
		{
			JPEG_t* jpeg = *it;
			if(jpeg->level == level && jpeg->col == col && jpeg->row == row)
				return jpeg;
		}
	}
	else
	{
		for(list<JPEG_t*>::iterator it=level_imgs2.begin(); it != level_imgs2.end(); it++)
        {
            JPEG_t* jpeg = *it;
            if(jpeg->level == level && jpeg->col == col && jpeg->row == row)
                    return jpeg;
        }
	}
	//load new
	std::stringstream ss;
	if(index == 0)
		ss << datadir1 << level << "/" << col << "_" << row << ".jpeg";
	else
		ss << datadir2 << level << "/" << col << "_" << row << ".jpeg";
	string filename = ss.str();

	JPEG_t* jpeg = loadJPEG(filename.c_str());
	jpeg->level = level;
	jpeg->col = col;
	jpeg->row = row;
	if(index == 0)
	{
		level_imgs1.push_back(jpeg);
		if(level_imgs1.size() > buffersize)
		{
			JPEG_t* tmp = level_imgs1.front();
			free(tmp->pixels);
			level_imgs1.pop_front();
		}
	}
	else
	{
		level_imgs2.push_back(jpeg);
    	if(level_imgs2.size() > buffersize)
    	{
    		JPEG_t* tmp = level_imgs2.front();
			free(tmp->pixels);
        	level_imgs2.pop_front();
    	}
	}

	return jpeg;
}

int DZDisplay::getImageRegion(unsigned char* buffer, int level, Reg_t region_src, int index)
{
	int first_t_col, first_t_row, last_t_col, last_t_row;
	first_t_col = region_src.left / tilesize;
	first_t_row = region_src.top / tilesize;

	last_t_col = (region_src.left + region_src.width -1) / tilesize;
	last_t_row = (region_src.top + region_src.height -1) / tilesize;

	//cout << "tile (hoz) from: " << first_t_col << " to: " << last_t_col << endl;
	//cout << "tile (ver) from: " << first_t_row << " to: " << last_t_row << endl;

	unsigned char* row_src_ptr;
	unsigned char* row_des_ptr;

	for(int c = first_t_col; c <= last_t_col; c++)
	{
		for(int r = first_t_row; r <= last_t_row; r++)
		{
			JPEG_t* jpeg = findOrLoadNewTile(level, c, r, index);
			//cout << "jpeg width: " << jpeg->width << " height: " << jpeg->height << endl;
			
			//copy data
			int left = region_src.left - c*tilesize;
			if(left < 0)
				left = 0;
			int top = region_src.top - r*tilesize;
			if(top < 0)
				top = 0;
			int right = region_src.left + region_src.width - c*tilesize;
			if(right > tilesize)
				right = tilesize;
			int bottom = region_src.top + region_src.height - r*tilesize;
			if(bottom > tilesize)
				bottom = tilesize;

			int dest_first_row = r*tilesize - region_src.top;
			if(dest_first_row < 0)
				dest_first_row = 0;
			int dest_first_col = c*tilesize - region_src.left;
			if(dest_first_col < 0)
				dest_first_col = 0;

			for(int i=top; i<bottom; i++)
			{
				row_src_ptr = &jpeg->pixels[i*jpeg->width*3 + left*3];
				row_des_ptr = &buffer[dest_first_row*region_src.width*3 + dest_first_col*3];
				memcpy(row_des_ptr, (unsigned char*)row_src_ptr, (right-left)*3);
				dest_first_row++;
			}
		}
	}
	
	return 0;
}

int DZDisplay::display(int left, int top, int mode)
{
	double downsample = 1.0*level0_w / width;
	return display(left, top, downsample, mode);
}

int DZDisplay::display(int left, int top, double downsample, int mode)
{
	// get best level
	int64_t img_w, img_h;
	img_w = level0_w / downsample;
	img_h = level0_h / downsample;
	//cout << "img_w: " << img_w << " img_h: " << img_h << endl;

	int64_t c_left = width*index;
	int64_t c_top = 0;
	if(c_left + width < left || c_left > left + img_w || top > height || top+img_h < c_top)
	{
		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glfwSwapBuffers(window);

		return -1;
	}

	Reg_t region_dis;
	region_dis.left = (left > c_left) ? left : c_left;
	region_dis.left -= index*width;
	region_dis.top = (top > c_top) ? top : c_top;
	region_dis.width = width - region_dis.left;
	if(region_dis.width > left + img_w - c_left)
		region_dis.width = left + img_w - c_left;
	region_dis.height = (img_h < height) ? img_h : height;
	if(region_dis.height > top + img_h)
		region_dis.height = top + img_h;
	//if(id == 9)
	//{
	//	cout << "(" << id << ") Display region: ";
	//	region_dis.print();
	//}

	//
	maxlevel = 0;
	while(pow(2,maxlevel)*tilesize <= level0_w)
		maxlevel++;
	//cout << "maxlevel: " << maxlevel << endl;

	int level;
	for(level=0; level < maxlevel; level++)
		if(pow(2,level)*tilesize > img_w)
			break;

	double ratio = 1.0*level0_w / pow(2, (maxlevel-level)) / img_w;

	if(mode==MODE_FAST && level > 0 && downsample >= 1 && ratio > 1.0)
	{
		level--;
		ratio = 1.0*level0_w / pow(2, (maxlevel-level)) / img_w;
	}
	if(id == 9)
		cout << "(" << id << ") Level: " << level << endl;

	Reg_t region_src;
	region_src.left = (region_dis.left + index*width - left) * ratio;
	region_src.top = (region_dis.top - top) * ratio;
	region_src.width = region_dis.width*ratio;
	region_src.height = region_dis.height*ratio;
	//if(id ==9)
	//{
	//	cout << "(" << id << ") Source region: ";
	//	region_src.print();
	//}

	// load and display
	read_time = 0; render_time = 0;

	unsigned char *buffer1, *buffer2;
	buffer1 = (unsigned char*)malloc(region_src.width*region_src.height*3);
	if(!buffer1)
	{
		cout << "(" << id << ") Cannot allocate memory for buffer1!" << endl;
		return -1;
	}
	unsigned int start_t = Utils::getTime();
	getImageRegion(buffer1, level, region_src, 0);
	read_time += Utils::getTime() - start_t;

	if(stereo)
	{
		buffer2 = (unsigned char*)malloc(region_src.width*region_src.height*3);
		if(!buffer2)
		{
			cout << "(" << id << ") Cannot allocate memory for buffer2!" << endl;
			return -1;
		}
		unsigned int start_t = Utils::getTime();
		getImageRegion(buffer2, level, region_src, 1);
		read_time += Utils::getTime() - start_t;
	}

	//write to test
	//if(id == 2)
	//	writeJPEG(buffer1, region_src.width, region_src.height, "testdata/test.jpeg", 90);

	start_t = Utils::getTime();
	tex1->update(buffer1, region_src.width, region_src.height);
	if(stereo)
		tex2->update(buffer2, region_src.width, region_src.height);

	tranMat.identity();
	tranMat.scale(Vector3(1.0*region_dis.width/width, 1.0*region_dis.height/height, 1.0));
	tranMat.translate(Vector3( 2.0*(region_dis.left+region_dis.width/2 - width/2)/width, 
							-2.0*(region_dis.top+region_dis.height/2 - height/2)/height, 0));
	draw();
	render_time += Utils::getTime() - start_t;
	
	free(buffer1);
	if(stereo)
		free(buffer2);

	return 0;
}
