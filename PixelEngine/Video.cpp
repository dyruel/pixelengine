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

}

bool Video::init() {

	ILogger::setLogger(&fileLogger);

	ILogger::log("Video :: Start engine initialization...");

    if (!glfwInit()) {
		ILogger::log("Error during the initialization of GLFW.");
        return false;
    }
    
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, NULL);
    if (!m_window) {
        glfwTerminate();
		ILogger::log("Error during the creation of the window.");
        return false;
    }
    
    glfwMakeContextCurrent(m_window);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
	// FOG test
	GLfloat fogColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);            
	glFogf(GL_FOG_DENSITY, 0.35f);              
	glHint(GL_FOG_HINT, GL_DONT_CARE);         
	glFogf(GL_FOG_START, 100.0f);             
	glFogf(GL_FOG_END, 1000.0f);
	glEnable(GL_FOG);
	// FOG test

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glViewport(0, 0, m_windowWidth, m_windowHeight);

	glfwSetCursorPos(m_window, m_windowWidth >> 1, m_windowHeight >> 1);

	ILogger::log("done.\n");
    return true;
}

bool Video::deinit() {
	ILogger::log("Video :: Start engine deinitialization...");
	glfwDestroyWindow(m_window);
	glfwTerminate();
	ILogger::log("done.\n");
	return true;
}


void Video::beginScene() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Video::endScene() const {
    glFlush();
    glfwSwapBuffers(m_window);
}





bool Video::windowShouldClose() {
    return glfwWindowShouldClose(m_window);
}

