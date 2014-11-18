//
//  Video.cpp
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Video.h"



Video::Video()
	: m_windowTitle("Pixel Engine"), m_windowWidth(1024), m_windowHeight(768) {

}


Video::~Video() {
    ILogger::log("Video :: Start engine deinitialization...");
    glfwDestroyWindow(m_window);
    glfwTerminate();
    ILogger::log("done.\n");
}

bool Video::init() {

	ILogger::setLogger(&fileLogger);

	ILogger::log("Video :: Start engine initialization...");

    if (!glfwInit()) {
		ILogger::log("Error during the initialization of GLFW.");
        return false;
    }
    
//	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), glfwGetPrimaryMonitor(), NULL);
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, NULL);
    if (!m_window) {
        glfwTerminate();
		ILogger::log("Error during the creation of the window.");
        return false;
    }
    
    glfwMakeContextCurrent(m_window);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
	// FOG test
	/*
	GLfloat fogColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);            
	glFogf(GL_FOG_DENSITY, 0.35f);              
	glHint(GL_FOG_HINT, GL_DONT_CARE);         
	glFogf(GL_FOG_START, 100.0f);             
	glFogf(GL_FOG_END, 1000.0f);
	glEnable(GL_FOG);
	*/
	// FOG test


	/*
	glEnable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	*/

	this->setOglDefaultState();
    
    GLint frameWidth = 0, frameHeight = 0;
    glfwGetFramebufferSize(m_window, &frameWidth, &frameHeight);
    glViewport(0, 0, frameWidth, frameHeight);

	glfwSetCursorPos(m_window, m_windowWidth >> 1, m_windowHeight >> 1);
    

    //printf("%s\n", glGetString(GL_VERSION));

	ILogger::log("done.\n");
    return true;
}



void Video::setOglDefaultState() {
	glClearDepth(1.0f);
	glCullFace(GL_FRONT);
	glColor4f(1.f, 1.f, 1.f, 1.f);
    //glClearColor(.5f, .5f, .5f, 1.f);
	glEnable(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	
}



void Video::beginScene() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Video::endScene() const {
    glFlush();
    glfwSwapBuffers(m_window);
}





bool Video::windowShouldClose() {
    return glfwWindowShouldClose(m_window) != 0;
}

