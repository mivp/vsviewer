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

#include "display.h"

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

Display::Display(int i, int w, int h): id(i), width(w), height(h), stereo(false), leftfirst(true),
											tex1(NULL), tex2(NULL)
{
	index = id - 1;
	if (index < 0)
		index = 0;
}

Display::~Display()
{
}

int Display::initDisplay()
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

void Display::draw()
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

int Display::loadVirtualSlide(Img_t img)
{
	return 0;
}

int Display::display(int left, int top, DISPLAY_MODE mode)
{
	return 0;
}

int Display::display(int left, int top, double downsample, DISPLAY_MODE mode)
{
	return 0;
}