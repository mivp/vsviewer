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

#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "glinclude.h"
#include "texture.h"
#include "shader.h"
#include "ssquad.h"

#include <iostream>

using namespace std;

enum DISPLAY_MODE { MODE_REFRESH=0, MODE_FAST, MODE_NORMAL, MODE_HIGH };

struct Reg_t{
	int64_t left,top;
	int64_t width,height;
	void print()
	{
		cout << left << " " << top << " " << width << " " << height << endl;
	}
};

struct Img_t{
	int type; // 0: single image, 1: stereo l first, 2: stereo r first
	string filename1;
	string filename2;
};

class Display
{
protected: 
	int id, index, width, height;
	int maxlevel;
	double maxdownsample;
	
	uint64_t level0_w, level0_h;
	
	bool stereo, leftfirst;
	Texture *tex1, *tex2;
	Shader* shaderDisplay;
	Shader* shaderDisplay3d;
	SSQuad* quad;
	Matrix4 tranMat;

public:
	GLFWwindow* window;
	uint read_time, render_time;

public:
	Display(int ind, int w, int h);
	~Display();

	void getLevel0Size(int64_t &w, int64_t &h) { w = level0_w; h = level0_h; }
	double getMaxDownsample() { return maxdownsample; }

	int initDisplay();
	void draw();

	virtual int loadVirtualSlide(Img_t img);
	virtual int display(int left=0, int top=0, DISPLAY_MODE mode=MODE_REFRESH);
	virtual int display(int left, int top, double downsample, DISPLAY_MODE mode=MODE_REFRESH);
};

#endif
