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

#include "vsdisplay.h"
#include <openslide-features.h>

#include "utils.h"
#include "glutils.h"
#include "shaderlibrary.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

VSDisplay::VSDisplay(int i, int w, int h): id(i), width(w), height(h), stereo(false), leftfirst(true),
											osr1(NULL), osr2(NULL), tex1(NULL), tex2(NULL)
{
	index = id - 1;
	if (index < 0)
		index = 0;
	fastlevel = 4;
}

VSDisplay::~VSDisplay()
{
	if(osr1)
		openslide_close(osr1);
	if(osr2)
		openslide_close(osr2);
}

int VSDisplay::loadVirtualSlide(Img_t img)
{
	if(osr1)
	{
		openslide_close(osr1);
		osr1 = NULL;
	}
	if(osr2)
	{
		openslide_close(osr2);
		osr2 = NULL;
	}

	stereo = false;
	if(img.type == 0)
	{
		cout << "(" << id << "): Open file: " << img.filename1 << endl;
		osr1 = openslide_open(img.filename1.c_str());
		assert(osr1);
	}
	else if (img.type == 1)
	{
		cout << "(" << id << "): Open files: " << img.filename1 << " " << img.filename2 << endl;
		osr1 = openslide_open(img.filename1.c_str());
		osr2 = openslide_open(img.filename2.c_str());
		assert(osr1);
		assert(osr2);
		stereo = true;
	}
	else if (img.type == 2)
	{
		cout << "(" << id << "): Open files: " << img.filename2 << " " << img.filename1 << endl;
		osr1 = openslide_open(img.filename2.c_str());
		osr2 = openslide_open(img.filename1.c_str());
		assert(osr1);
		assert(osr2);
		stereo = true;
	}

	//cout << "(" << id << ") Version: " << openslide_get_version() << endl;
	cout << "(" << id << ") Num level: " << openslide_get_level_count(osr1) << endl;
	openslide_get_level_dimensions(osr1, 0, &level0_w, &level0_h);
	cout << "(" << id << ") Level 0 dimensions: " << level0_w << " " << level0_h << endl;

	maxlevel = openslide_get_level_count(osr1);

	int64_t w_l, h_l;
	for(int i=0; i < maxlevel; i++)
	{	
		openslide_get_level_dimensions(osr1, i, &w_l, &h_l);
		maxdownsample = openslide_get_level_downsample(osr1, i);
		if(h_l < height)
			break;
	}

	fastlevel = 4;
	if(fastlevel >= maxlevel)
		fastlevel = maxlevel - 1;

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

	return 0;
}

int VSDisplay::initDisplay()
{
	// Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    // OpenGL 2.1
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Open a window and create its OpenGL context
    std::ostringstream ss;
	ss << "Virtual Slide Viewer (" << id << ")";
    window = glfwCreateWindow( width, height, ss.str().c_str(), NULL, NULL);
    if( window == NULL ){
        cout << "Failed to open GLFW window: " << stderr << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    //glfwSetKeyCallback(window, key_callback);
    //glfwSetWindowSizeCallback(window, window_size_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW: " << stderr << endl;
        return -1;
    }
    
    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // print GL info
    //GLUtils::dumpGLInfo(true);

	// shaders
	list<string> attributes, uniforms;
	attributes.clear(); uniforms.clear();
    attributes.push_back("position");
    attributes.push_back("texcoor");
    uniforms.push_back("tranMat");
    uniforms.push_back("texScene");
    ShaderLibrary::addShader("ss_display", "shaders/ss_display", attributes, uniforms);
    shaderDisplay = ShaderLibrary::getShader("ss_display");

    attributes.clear(); uniforms.clear();
    attributes.push_back("position");
    attributes.push_back("texcoor");
    uniforms.push_back("tranMat");
    uniforms.push_back("leftFirst");
    uniforms.push_back("texScene1");
    uniforms.push_back("texScene2");
    ShaderLibrary::addShader("ss_display3d", "shaders/ss_display3d", attributes, uniforms);
    shaderDisplay3d = ShaderLibrary::getShader("ss_display3d");

    // quad
    quad = new SSQuad();
    quad->init();
	
	return 0;
}

int VSDisplay::display(int left, int top, int mode)
{
	double downsample = 1.0*level0_w / width;
	return display(left, top, downsample, mode);
}

int VSDisplay::display(int left, int top, double downsample, int mode)
{
	if(mode == MODE_REFRESH) // refresh
	{
		draw();
		return 0;
	}

	int level = openslide_get_best_level_for_downsample(osr1, downsample);
	//cout << "(" << id << ") Best level: " << level << endl;
	if(mode == MODE_FAST)
	{
		if(level < fastlevel)
			level = fastlevel;
		if(level > maxlevel)
			level = maxlevel;
	}
	else if (mode == MODE_NORMAL)
	{
		if(level < fastlevel && downsample >= 1.0)
			level++;
	}

	int64_t level_w, level_h;
	openslide_get_level_dimensions(osr1, level, &level_w, &level_h);
	//cout << "(" << id << ") Level dims: " << level_w << " " << level_h << endl;

	int64_t img_w, img_h;
	img_w = int64_t(level0_w / downsample);
	img_h = int64_t(level0_h / downsample);
	//cout << "(" << id << ") Image dims: " << img_w << " " << img_h << endl;

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
	region_dis.height = (img_h < height) ? img_h : height;
	//cout << "(" << id << ") Display region: ";
	//region_dis.print();

	double ratio = 1.0*level_w/img_w;
	//cout << "(" << id << ") Ratio (level/img): " << ratio << endl;
	Reg_t region_src;
	region_src.left = (region_dis.left + index*width - left) * downsample;
	region_src.top = (region_dis.top - top) * downsample;
	region_src.width = region_dis.width*ratio;
	region_src.height = region_dis.height*ratio;
	//cout << "(" << id << ") Source region: ";
	
	read_time = 0; render_time = 0;
	uint start_time;

	unsigned char *buffer1, *buffer2;
	start_time = Utils::getTime();
	buffer1 = (unsigned char*)malloc(region_src.width*region_src.height*sizeof(uint32_t));
	if(!buffer1)
	{
		cout << "(" << id << ") Cannot allocate memory for buffer1!" << endl;
		return -1;
	}
	openslide_read_region (osr1, (uint32_t*)buffer1, region_src.left, region_src.top,
							 level, region_src.width, region_src.height);
	read_time += Utils::getTime() - start_time;
	if(!buffer1)
	{
		cout << "(" << id << ") Cannot read region 1!" << endl;
		return -1;
	}
	start_time = Utils::getTime();
	tex1->update(buffer1, region_src.width, region_src.height);
	render_time += Utils::getTime() - start_time;

	if(stereo)
	{
		start_time = Utils::getTime();
		buffer2 = (unsigned char*)malloc(region_src.width*region_src.height*sizeof(uint32_t));
		if(!buffer2)
		{
			cout << "(" << id << ") Cannot allocate memory for buffer2!" << endl;
			return -1;
		}
		openslide_read_region (osr2, (uint32_t*)buffer2, region_src.left, region_src.top,
								 level, region_src.width, region_src.height);
		read_time += Utils::getTime() - start_time;
		if(!buffer2)
		{
			cout << "(" << id << ") Cannot read region 2!" << endl;
			return -1;
		}
		start_time = Utils::getTime();
		tex2->update(buffer2, region_src.width, region_src.height);
		render_time += Utils::getTime() - start_time;
		leftfirst = (region_dis.top % 2 == 0) ? true : false;
	}

	tranMat.identity();
	tranMat.scale(Vector3(1.0*region_dis.width/width, 1.0*region_dis.height/height, 1.0));
	tranMat.translate(Vector3( 2.0*(region_dis.left+region_dis.width/2 - width/2)/width, 
							-2.0*(region_dis.top+region_dis.height/2 - height/2)/height, 0));
	start_time = Utils::getTime();
	draw();
	render_time += Utils::getTime() - start_time;

	free(buffer1);
	if(stereo)
		free(buffer2);
	
	return 0;
}

void VSDisplay::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	if(!stereo)
	{
		shaderDisplay->bind();
		tex1->bind();
		shaderDisplay->transmitUniform("texScene", tex1);
		shaderDisplay->transmitUniform("tranMat", tranMat);
	}
	else
	{
		shaderDisplay3d->bind();
		tex1->bind();
		shaderDisplay3d->transmitUniform("texScene1", tex1);
		tex2->bind();
		shaderDisplay3d->transmitUniform("texScene2", tex2);
		shaderDisplay3d->transmitUniform("tranMat", tranMat);
		shaderDisplay3d->transmitUniform("leftFirst", leftfirst);
	}
	
    quad->draw();

	glfwSwapBuffers(window);
}
