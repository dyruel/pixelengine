//
//  Camera.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Camera_h
#define PixelEngine_Camera_h

#include <GLFW/glfw3.h>

#include "Vector.h"

// Camera
class Camera {
    
protected:
    GLdouble m_fov, m_aspect, m_near, m_far;
    
    Vector3d m_position;
	Vector3d m_dir;
    Vector3d m_up;

public:
    Camera();
    Camera(GLdouble fov, GLdouble aspect, GLdouble near, GLdouble far);
    virtual ~Camera() {};
    
    virtual bool init();
    virtual void setView();
	virtual void update(GLdouble delta) {};
    
    Vector3d& getPosition()  { return m_position; };
	Vector3d& getDir() { return m_dir; };
    Vector3d& getUpVector()  { return m_up; };
};


// CameraFree
class CameraFree : public Camera {

protected:

	GLdouble m_speed;
	GLdouble m_vspeed;
	GLdouble m_hspeed;

	// Last mouse position
	GLdouble m_mousex;
	GLdouble m_mousey;

	// Rotation angles
	GLdouble m_vangle;
	GLdouble m_hangle;

public:

	CameraFree() : m_speed(200.0), m_mousex(0.0), m_mousey(0.0), m_vangle(0.0), m_hangle(0.0), m_vspeed(0.2), m_hspeed(0.2) {}
	virtual ~CameraFree() {};

	virtual void update(GLdouble delta);
};

// CameraFps
class CameraFps : public CameraFree {

public:

	CameraFps() {}
	virtual ~CameraFps() {};

	void update(GLdouble delta);
};

// CameraOrtho
class CameraOrtho : public Camera {
    
public:
    
    bool init();
    void setView();
    void update(GLdouble delta);
    
    CameraOrtho() {}
    virtual ~CameraOrtho() {};
    
};


#endif
