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

Display::Display(int i, int w, int h, int numclients): id(i), width(w), height(h), stereo(false), leftfirst(true),
											tex1(NULL), tex2(NULL)
{
	index = id - 1;
	if (index < 0)
		index = 0;

    entire_display_w =  numclients * w;
    entire_display_h = h;
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

    attributes.clear(); uniforms.clear();
    attributes.push_back("position");
    uniforms.push_back("colour");
    ShaderLibrary::addShader("ss_draw_colour", "shaders/ss_draw_colour", attributes, uniforms);
    shaderDrawColour = ShaderLibrary::getShader("ss_draw_colour");

    // quad
    quad = new SSQuad();
    quad->init();

    // init rect display
    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);

    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );
	
	return 0;
}

void Display::drawImage()
{
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
}

void Display::drawBegin()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
}

void Display::drawEnd()
{
    glfwSwapBuffers(window);
}

int Display::loadVirtualSlide(Img_t img)
{
	return 0;
}

int Display::display(int left, int top, DISPLAY_MODE mode, bool minimap)
{
	return 0;
}

int Display::display(int left, int top, double downsample, DISPLAY_MODE mode, bool minimap)
{
	return 0;
}

void Display::drawRect(float x1, float y1, float x2, float y2, Vector3 colour)
{
    float vertices[12];
    vertices[0] = x1; vertices[1] = y1;
    vertices[2] = x2; vertices[3] = y1;
    vertices[4] = x2; vertices[5] = y2;

    vertices[6] = x2; vertices[7] = y2;
    vertices[8] = x1; vertices[9] = y2;
    vertices[10] = x1; vertices[11] = y1;

    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*12, vertices);

    shaderDrawColour->bind();
    shaderDrawColour->transmitUniform("colour", colour);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(rect_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Display::drawMinimap(float nleft, float ntop, float nwidth, float nheight) // values [0 1] compared to img region
{
    // draw img rect
    float img_left = 0.2, img_top = 0.02;
    float img_width = 0.6, img_height = 0.1;
    drawRect(img_left, img_top, img_left + img_width, img_top + img_height, Vector3(0.3, 0.3, 0.3));

    // draw visible area
    nleft = img_left + nleft*img_width;
    ntop = img_top + ntop*img_height;

    nwidth = nwidth*img_width;
    if(nwidth + nleft > img_left + img_width)
        nwidth = img_left + img_width - nleft;
    if(nwidth < 0.001)
        nwidth = 0.001;

    nheight = nheight*img_height;
    if(nheight + ntop > img_top + img_height)
        nheight = img_top + img_height - ntop;
    if(nheight < 0.001)
        nheight = 0.001;

    drawRect(nleft, ntop, nleft + nwidth, ntop + nheight, Vector3(0.8, 0.8, 0.0));
}