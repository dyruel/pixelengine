//
//  Video.h
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__Video__
#define __PixelEngine__Video__

#include <iostream>
#include <string>


#include "Opengl.h"
#include "Singleton.h"
#include "Logger.h"


class CVideo : public Singleton<CVideo> {
    friend class Singleton<CVideo>;

public:
	~CVideo();
    

	void setOglDefaultState();

    void beginFrame() const;

	void endFrame() const;

	GLFWwindow* getWindow() const { return m_window; };

    bool windowShouldClose();

private:

	GLFWwindow * m_window = NULL;
	FileLogger fileLogger;

	std::string m_windowTitle;
	int m_windowWidth;
	int m_windowHeight;

	bool init();

	CVideo();
	CVideo(const CVideo&);
	void operator =(const CVideo&);
};


#endif /* defined(__PixelEngine__Video__) */
