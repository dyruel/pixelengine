//
//  Camera.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


#include <GLFW/glfw3.h>
#include <cmath>

#include "Video.h"
#include "Camera.h"

const GLdouble PI = 3.1415926535897932384626433832795;
const GLdouble TO_RADS = PI / 180.0;

Camera::Camera(GLdouble fov, GLdouble aspect, GLdouble near, GLdouble far)
: m_fov(fov), m_far(far), m_aspect(aspect), m_near(near){
    
}

Camera::Camera() {
    GLint viewport[4] = {0,0,0,0};
    
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    m_fov = 60.;
    m_near = 1.;
    m_far = 1000.;
    m_aspect = ((GLdouble) viewport[2] / (GLdouble) viewport[3]);

	m_position[0] = 0.; m_position[1] = 0.; m_position[2] = 0.;
	m_dir[0] = 0.; m_dir[1] = 0.; m_dir[2] = 0.;
	m_up[0] = 0.0;  m_up[1] = 0.0;  m_up[2] = 1.;
}

bool Camera::init() {
    GLdouble fW = .0, fH = .0;
    
    fH = tan( (m_fov / 360.) * PI) * m_near;
    fW = fH * m_aspect;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-fW, fW, -fH, fH, m_near, m_far);
    
    return true;
}

void Camera::setView() {
    Vector3d n, up, u, v;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
	n = m_dir;
	n.normalize();
    
    up = m_up;
    up.normalize();
    
    u = up.crossProduct(n);
    u.normalize();
    
    v = n.crossProduct(u);
 
    GLdouble m[16] = {
        u[0], v[0], n[0], 0.,
        u[1], v[1], n[1], 0.,
        u[2], v[2], n[2], 0.,
        (-u[0]) * m_position[0] + (-u[1]) * m_position[1] + (-u[2]) * m_position[2],
        (-v[0]) * m_position[0] + (-v[1]) * m_position[1] + (-v[2]) * m_position[2],
        (-n[0]) * m_position[0] + (-n[1]) * m_position[1] + (-n[2]) * m_position[2],
        1.
    };

    glMultMatrixd(m);
}


void CameraFree::update(GLdouble delta) {
	GLFWwindow* window = Video::getInstance()->getWindow();
	GLdouble mx, my;
	GLint winHalfWidth, winHalfHeight;
	Vector3d move;
	
	glfwGetCursorPos(window, &mx, &my);
	glfwGetWindowSize(window, &winHalfWidth, &winHalfHeight);
	winHalfWidth = winHalfWidth >> 1;
	winHalfHeight = winHalfHeight >> 1;

	m_hangle += (mx - winHalfWidth) * m_hspeed;
	m_vangle += (my - winHalfHeight) * m_vspeed;

	// Clamp angles
	if (m_vangle < -90.) {
		m_vangle = -90.;
	}
	else if (m_vangle > 90.) {
		m_vangle = 90.;
	}

	if (m_hangle < 0.) {
		m_hangle += 360.;
	}
	else if (m_hangle > 360.) {
		m_hangle += -360.;
	}


	GLdouble sinH = sin(m_hangle * TO_RADS);
	GLdouble cosH = cos(m_hangle * TO_RADS);

	GLdouble sinV = sin(m_vangle * TO_RADS);
//	GLdouble cosV = cos(m_vangle * TO_RADS);

	m_dir[0] = sinH;
	m_dir[1] = cosH;
	m_dir[2] = sinV;

	m_dir.normalize();

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		move = -m_dir;
	} 

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		move = m_dir;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		move[0] = m_dir[1];
		move[1] = -m_dir[0];
	}

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		move[0] = -m_dir[1];
		move[1] = m_dir[0];
	}

	m_position += (move * (m_speed * delta));

	glfwSetCursorPos(window, winHalfWidth, winHalfHeight);
}


void CameraFps::update(GLdouble delta) {
	GLFWwindow* window = Video::getInstance()->getWindow();
	GLdouble mx, my;
	GLint winHalfWidth, winHalfHeight;
	Vector3d move;

	glfwGetCursorPos(window, &mx, &my);
	glfwGetWindowSize(window, &winHalfWidth, &winHalfHeight);
	winHalfWidth = winHalfWidth >> 1;
	winHalfHeight = winHalfHeight >> 1;

	m_hangle += (mx - winHalfWidth) * m_hspeed;
	m_vangle += (my - winHalfHeight) * m_vspeed;

	// Clamp angles
	if (m_vangle < -90.) {
		m_vangle = -90.;
	}
	else if (m_vangle > 90.) {
		m_vangle = 90.;
	}

	if (m_hangle < 0.) {
		m_hangle += 360.;
	}
	else if (m_hangle > 360.) {
		m_hangle += -360.;
	}


	GLdouble sinH = sin(m_hangle * TO_RADS);
	GLdouble cosH = cos(m_hangle * TO_RADS);

	GLdouble sinV = sin(m_vangle * TO_RADS);
	//	GLdouble cosV = cos(m_vangle * TO_RADS);

	m_dir[0] = sinH;
	m_dir[1] = cosH;
	m_dir[2] = sinV;

	m_dir.normalize();

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		move[0] = -m_dir[0];
		move[1] = -m_dir[1];
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		move[0] = m_dir[0];
		move[1] = m_dir[1];
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		move[0] = m_dir[1];
		move[1] = -m_dir[0];
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		move[0] = -m_dir[1];
		move[1] = m_dir[0];
	}

	m_position += (move * (m_speed * delta));

	glfwSetCursorPos(window, winHalfWidth, winHalfHeight);
}