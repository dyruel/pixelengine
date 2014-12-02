//
//  Video.cpp
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Video.h"


CVideo::CVideo()
	: m_numIVBOs(0), m_windowTitle("Pixel Engine"), m_windowWidth(1024), m_windowHeight(768) {

}


CVideo::~CVideo() {
    ILogger::log("Video :: Start engine deinitialization...");
    glfwDestroyWindow(m_window);
    glfwTerminate();
    ILogger::log("done.\n");
}

bool CVideo::init() {

	ILogger::setLogger(&fileLogger);

	ILogger::log("Video :: Start engine initialization...\n");

    if (!glfwInit()) {
		ILogger::log("Error during the initialization of GLFW.");
        return false;
    }
    
    glfwSwapInterval(0); // disable vsync
    
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

	//glGenBuffers();

	this->setOglDefaultState();
    
    GLint frameWidth = 0, frameHeight = 0;
    glfwGetFramebufferSize(m_window, &frameWidth, &frameHeight);
    glViewport(0, 0, frameWidth, frameHeight);

	glfwSetCursorPos(m_window, m_windowWidth >> 1, m_windowHeight >> 1);
    
	//std::cout << glGetString(GL_VERSION) << std::endl; 
    //printf("%s\n", glGetString(GL_VERSION));

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		ILogger::log("Error : failed to initialize GLEW.");
		return false;
	}
	ILogger::log("Using GLEW Version: %s.", glewGetString(GLEW_VERSION));

	if (!GLEW_VERSION_3_0) {
		ILogger::log("OpenGL 3.0 or above is required.");
		return false;
	}

	for (u32 i = 0; i < CVideo::MAX_IVBO; ++i) {
		m_IVBOs[i].vboIds[0] = 0;
		m_IVBOs[i].vboIds[1] = 0;
	}

	ILogger::log("done.\n");
    return true;
}



void CVideo::setOglDefaultState() {
	glClearDepth(1.0f);
	glCullFace(GL_FRONT);
	glColor4f(1.f, 1.f, 1.f, 1.f);
    glClearColor(.5f, .5f, .5f, 1.f);
	glEnable(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	
}



void CVideo::beginFrame() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void CVideo::endFrame() const {
    glFlush();
    glfwSwapBuffers(m_window);
}

bool CVideo::createIVBO(u32* id, const GLsizeiptr& sizeVbo0, const GLvoid * data0, const GLsizeiptr& sizeVbo1, const GLvoid * data1, const GLenum& usage) {
	GLuint vboIds[2];

	if (m_numIVBOs >= CVideo::MAX_IVBO) {
		ILogger::log("CVideo :: The maximum number of IVBOS is attained.");
		return false;
	}

	*id = m_numIVBOs++;

	glGenBuffers(2, vboIds);

	m_IVBOs[*id].vboIds[0] = vboIds[0];
	m_IVBOs[*id].vboIds[1] = vboIds[1];

	glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVbo0, 0, usage);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVbo0, data0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeVbo1, 0, usage);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeVbo1, data1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}


bool CVideo::prepareIVBO(const u32& id) {
	assert(id >= 0 && id < CVideo::MAX_IVBO);

	if (m_IVBOs[id].vboIds[0] == 0 || m_IVBOs[id].vboIds[1] == 0) {
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_IVBOs[id].vboIds[0]);
	//	glTexCoordPointer(3, GL_FLOAT, sizeof(Q3Vertex), 0);
	//glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IVBOs[id].vboIds[1]);


}

void CVideo::drawIVBO() {
	//glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, 0);
}


void CVideo::releaseIVBO() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

bool CVideo::windowShouldClose() {
    return glfwWindowShouldClose(m_window) != 0;
}

