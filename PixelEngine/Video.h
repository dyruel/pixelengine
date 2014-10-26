//
//  Video.h
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __Escape__Video__
#define __Escape__Video__

#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

//#include "glext.h"
#include "Singleton.h"
#include "Logger.h"
#include "Scene.h"

class Video : public Singleton<Video> {
	friend class Singleton <Video>;

private:
    
	GLFWwindow * m_window = NULL;
	FileLogger fileLogger;

    std::string m_windowTitle;
    int m_windowWidth;
    int m_windowHeight;
    
	bool init();
	bool deinit();
        
    Video();
	Video(Video&);
	void operator =(Video&);

public:

	virtual ~Video();

    void beginScene() const;
    void endScene() const;

	GLFWwindow* getWindow() const { return m_window; };

    bool windowShouldClose();
};

#endif /* defined(__Escape__Video__) */
