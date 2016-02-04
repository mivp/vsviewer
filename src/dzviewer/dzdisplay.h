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

#ifndef VSCLIENT_H__
#define VSCLIENT_H__

#include "display.h"
#include "pyramid.h"

#include <omicron/Thread.h>

#include <iostream>
#include <list>
using namespace std;

// main class
class DZDisplay: public Display
{
private: 
	int tilesize;
	int buffersize;

	list<JPEG_t*> level_imgs1;
	list<JPEG_t*> level_imgs2;
	string datadir1, datadir2;
	Pyramid* pyramids[2];

	static list<omicron::Thread*> sImageLoaderThread;
	static int sNumLoaderThreads;

public:
	DZDisplay(int ind, int w, int h, int numclients);
	~DZDisplay();
	
	virtual int loadVirtualSlide(Img_t img);
	virtual int display(int left=0, int top=0, int mode=MODE_REFRESH, bool minimap=false);
	virtual int display(int left, int top, double downsample, int mode=MODE_REFRESH, bool minimap=false); //0: refresh only, 1: fast, 2: normal, 3: high quality

	void setNumThreads(int _thread) {sNumLoaderThreads = _thread;}
	void clearBuffer();
	void setBufferSize(int _buffersize) {buffersize = _buffersize;}
	JPEG_t* loadJPEG(const char* filename);
	int writeJPEG (unsigned char* pixels, int w, int h, const char * filename, int quality = 90);
	int getImageRegion(unsigned char* buffer1, int level, Reg_t region_src, int index=0);
};

#endif
