//
//  Video.h
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__Video__
#define __PixelEngine__Video__

#include <cassert>
#include <iostream>
#include <string>

#include "Types.h"
#include "Opengl.h"
#include "Singleton.h"
#include "Logger.h"



class CVideo : public Singleton<CVideo> {
    friend class Singleton<CVideo>;

public:
	~CVideo();

	static const s32 MAX_IVBO = 1024;
    
	void setOglDefaultState();

    void beginFrame() const;

	void endFrame() const;

	bool createIVBO(u32* id, const GLsizeiptr& sizeVbo0, const GLvoid * data0, const GLsizeiptr& sizeVbo1, const GLvoid * data1, const GLenum& usage = GL_STATIC_DRAW);
	bool prepareIVBO(const u32& id);
	void drawIVBO();
	void releaseIVBO();

	GLFWwindow* getWindow() const { return m_window; };

    bool windowShouldClose();

private:

	CVideo();
	CVideo(const CVideo&);
	void operator =(const CVideo&);

	typedef struct {
		GLuint vboIds[2];
	} SVboPair;

	GLFWwindow*		m_window = NULL;
	FileLogger		fileLogger;

	std::string		m_windowTitle;
	SVboPair		m_IVBOs[CVideo::MAX_IVBO];
	u32				m_numIVBOs;
	s32				m_windowWidth;
	s32				m_windowHeight;

	bool init();
};


#endif /* defined(__PixelEngine__Video__) */
