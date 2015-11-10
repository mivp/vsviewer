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

#include "osdisplay.h"

#include "utils.h"
#include "glutils.h"
#include "shaderlibrary.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdio.h>
#include <memory.h>

using namespace std;
using namespace omicron;

//global
openslide_t *osrs[2];
int tilesize;

Lock sImageQueueLock;
bool sShutdownLoaderThread = false;
int OSDisplay::sNumLoaderThreads = 2;
list<Thread*> OSDisplay::sImageLoaderThread;
list<JPEG_t*> sImageQueue;

class ImageLoaderThread: public Thread
{
public:
    ImageLoaderThread()
    {}

    virtual void threadProc()
    {
        cout << "ImageLoaderThread: start" << endl;

        while(!sShutdownLoaderThread)
        {
          	if(sImageQueue.size() > 0)
          	{
          		sImageQueueLock.lock();
          		if(sImageQueue.size() > 0)
                {
                	JPEG_t* jpeg = sImageQueue.front();
                    sImageQueue.pop_front();
                    sImageQueueLock.unlock();

                    jpeg->pixels = Utils::loadTile(osrs[jpeg->leftright], jpeg->level, 
                    								jpeg->row, jpeg->col, tilesize,	jpeg->width, jpeg->height);
                    jpeg->data_size = jpeg->width*jpeg->height*sizeof(uint32_t);

                    if(!sShutdownLoaderThread)
                    {
                    	jpeg->ready = true;
                        //cout << "  Image is ready to use!!!" << endl;
                    }
                }
                else
                {
                	sImageQueueLock.unlock();
                }

          	}
            osleep(1);
        }
        cout << "ImageLoaderThread: shutdown" << endl;
    }
};

// OSDisplay
OSDisplay::OSDisplay(int i, int w, int h): Display(i, w, h)
{
	buffersize = 16;
	level_imgs1.clear();
	level_imgs2.clear();

	osrs[0] = NULL;
	osrs[1] = NULL;

	pyramids[0] = new Pyramid();
	pyramids[1] = new Pyramid();

	tilesize = 1024;

	if(i == 0)
		sNumLoaderThreads = 1;

	if(sImageLoaderThread.size() == 0)
    {
    	for(int i = 0; i < sNumLoaderThreads; i++)
	    {
	        Thread* t = new ImageLoaderThread();
	     	t->start();
	        sImageLoaderThread.push_back(t);;
	    }
    }
}

OSDisplay::~OSDisplay()
{
	clearBuffer();

	if(osrs[0])
		openslide_close(osrs[0]);
	if(osrs[1])
		openslide_close(osrs[1]);

	delete pyramids[0];
	delete pyramids[1];
}

void OSDisplay::clearBuffer()
{
	sImageQueueLock.lock();
	sImageQueue.clear();
    sImageQueueLock.unlock();

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

int OSDisplay::loadVirtualSlide(Img_t img)
{
	// clear buffer
	clearBuffer();

	for(int i=0; i<2; i++)
	{
		if(osrs[i])
		{
			openslide_close(osrs[i]);
			osrs[i] = NULL;
		}
	}

	stereo = false;
	if(img.type == 0)
	{
		cout << "(" << id << "): Open file: " << img.filename1 << endl;
		osrs[0] = openslide_open(img.filename1.c_str());
		assert(osrs[0]);
	}
	else if (img.type == 1)
	{
		cout << "(" << id << "): Open files: " << img.filename1 << " " << img.filename2 << endl;
		osrs[0] = openslide_open(img.filename1.c_str());
		osrs[1] = openslide_open(img.filename2.c_str());
		assert(osrs[0]);
		assert(osrs[1]);
		stereo = true;
	}
	else if (img.type == 2)
	{
		cout << "(" << id << "): Open files: " << img.filename2 << " " << img.filename1 << endl;
		osrs[0] = openslide_open(img.filename2.c_str());
		osrs[1] = openslide_open(img.filename1.c_str());
		assert(osrs[0]);
		assert(osrs[1]);
		stereo = true;
	}
	
	//cout << "(" << id << ") Version: " << openslide_get_version() << endl;
	cout << "(" << id << ") Num level: " << openslide_get_level_count(osrs[0]) << endl;
	openslide_get_level_dimensions(osrs[0], 0, &level0_w, &level0_h);
	cout << "(" << id << ") Level 0 dimensions: " << level0_w << " " << level0_h << endl;

	maxlevel = openslide_get_level_count(osrs[0]);

	maxdownsample = 2.0 * level0_w / 1024;

	// texture
	if(tex1 == NULL)
	{
		tex1 = Texture::newFromNextUnit(width, height);
		tex1->init();
	}

	if(stereo && tex2 == NULL)
	{
		tex2 = Texture::newFromNextUnit(width, height);
		tex2->init();
	}

	// indexing
	pyramids[0]->build(osrs[0], tilesize);
	if(id == 1)
		pyramids[0]->print();
	if(stereo)
		pyramids[1]->build(osrs[0], tilesize);

	cout << "(" << id << ") loadVirtualSlide completed!" << endl;
	
	return 0;
}

int OSDisplay::getImageRegion(unsigned char* buffer, int level, Reg_t region_src, int index)
{
	int first_t_col, first_t_row, last_t_col, last_t_row;
	first_t_col = region_src.left / tilesize;
	first_t_row = region_src.top / tilesize;

	last_t_col = (region_src.left + region_src.width -1) / tilesize;
	last_t_row = (region_src.top + region_src.height -1) / tilesize;

	//cout << "tile (hoz) from: " << first_t_col << " to: " << last_t_col << endl;
	//cout << "tile (ver) from: " << first_t_row << " to: " << last_t_row << endl;

	//add to queue
	for(int c = first_t_col; c <= last_t_col; c++)
	{
		for(int r = first_t_row; r <= last_t_row; r++)
		{
			//int ret = pyramids[index]->getValue(level, c, r);
			//if(ret == -1)
			//	cout << "Pyramid problem!!! level:" << level << " c: " << c << " r: " << r << endl;
			if(pyramids[index]->getValue(level, c, r) == 0)
			{
				JPEG_t* jpeg = new JPEG_t;
				jpeg->level = level; jpeg->col = c; jpeg->row = r;
				jpeg->ready = false;
				jpeg->leftright = index;
				std::stringstream ss;
				if(index == 0)
				{
					level_imgs1.push_back(jpeg);
					if(level_imgs1.size() > buffersize)
					{
						JPEG_t* tmp = level_imgs1.front();
						free(tmp->pixels);
						sImageQueueLock.lock();
						pyramids[index]->setValue(tmp->level, tmp->col, tmp->row, 0);
						sImageQueueLock.unlock();
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
						sImageQueueLock.lock();
						pyramids[index]->setValue(tmp->level, tmp->col, tmp->row, 0);
						sImageQueueLock.unlock();
						level_imgs2.pop_front();
					}
				}
				pyramids[index]->setValue(level, c, r, 1);
				sImageQueueLock.lock();
				sImageQueue.push_back(jpeg);
				sImageQueueLock.unlock();
			}
		}
	}

	unsigned char* row_src_ptr;
	unsigned char* row_des_ptr;

	for(int c = first_t_col; c <= last_t_col; c++)
	{
		for(int r = first_t_row; r <= last_t_row; r++)
		{
			JPEG_t* jpeg = NULL;
			if(index == 0)
			{
				for(list<JPEG_t*>::iterator it=level_imgs1.begin(); it != level_imgs1.end(); it++)
				{
					jpeg = *it;
					if(jpeg->level == level && jpeg->col == c && jpeg->row == r)
						break;
				}
			}
			else
			{
				for(list<JPEG_t*>::iterator it=level_imgs2.begin(); it != level_imgs2.end(); it++)
				{
					jpeg = *it;
					if(jpeg->level == level && jpeg->col == c && jpeg->row == r)
						break;
				}
			}
			if(!jpeg)
			{
				cout << "(" << id << ") Cannot find image" << endl;
				return -1;
			}

			while(!jpeg->ready)
			{
				//osleep(1);
			}
			
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
				row_src_ptr = &jpeg->pixels[i*jpeg->width*sizeof(uint32_t) + left*sizeof(uint32_t)];
				row_des_ptr = &buffer[dest_first_row*region_src.width*sizeof(uint32_t) + dest_first_col*sizeof(uint32_t)];
				memcpy(row_des_ptr, (unsigned char*)row_src_ptr, (right-left)*sizeof(uint32_t));
				dest_first_row++;
			}
		}
	}
	
	return 0;
}

int OSDisplay::display(int left, int top, int mode)
{
	double downsample = 1.0*level0_w / width;
	return display(left, top, downsample, mode);
}

int OSDisplay::display(int left, int top, double downsample, int mode)
{
	int level = openslide_get_best_level_for_downsample(osrs[0], downsample);

	int64_t level_w, level_h;
	openslide_get_level_dimensions(osrs[0], level, &level_w, &level_h);
	//cout << "(" << id << ") Level dims: " << level_w << " " << level_h << endl;

	int64_t img_w, img_h;
	img_w = int64_t(level0_w / downsample);
	img_h = int64_t(level0_h / downsample);
	//cout << "(" << id << ") Image dims: " << img_w << " " << img_h << endl;

	double ratio = 1.0*level_w/img_w;
	if(mode==MODE_FAST && level < maxlevel && downsample >= 1 && ratio > 1.0)
	{
		level++;
		openslide_get_level_dimensions(osrs[0], level, &level_w, &level_h);
		ratio = 1.0*level_w/img_w;
	}
	//if(id == 9)
		cout << "(" << id << ") Level: " << level << endl;

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
		cout << "(" << id << ") Display region: ";
		region_dis.print();
	//}

	Reg_t region_src;
	region_src.left = (region_dis.left + index*width - left) * ratio;
	region_src.top = (region_dis.top - top) * ratio;
	region_src.width = region_dis.width*ratio;
	region_src.height = region_dis.height*ratio;
	//if(id ==9)
	//{
		cout << "(" << id << ") Source region: ";
		region_src.print();
	//}
	
	// load and display
	read_time = 0; render_time = 0;

	unsigned char *buffer1, *buffer2;
	buffer1 = (unsigned char*)malloc(region_src.width*region_src.height*sizeof(uint32_t));
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
		buffer2 = (unsigned char*)malloc(region_src.width*region_src.height*sizeof(uint32_t));
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
