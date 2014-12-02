//
//  Scene.cpp
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


#include <iostream>

#include "Scene.h"

void CDummyNode::render() {
    
    glBegin(GL_QUADS);
    
    glColor3ub(255,0,0);
    glVertex3d(1,1,1);
    glVertex3d(1,1,-1);
    glVertex3d(-1,1,-1);
    glVertex3d(-1,1,1);
    
    glColor3ub(0,255,0);
    glVertex3d(1,-1,1);
    glVertex3d(1,-1,-1);
    glVertex3d(1,1,-1);
    glVertex3d(1,1,1);
    
    glColor3ub(0,0,255); 
    glVertex3d(-1,-1,1);
    glVertex3d(-1,-1,-1);
    glVertex3d(1,-1,-1);
    glVertex3d(1,-1,1);
    
    glColor3ub(255,255,0); 
    glVertex3d(-1,1,1);
    glVertex3d(-1,1,-1);
    glVertex3d(-1,-1,-1);
    glVertex3d(-1,-1,1);
    
    glColor3ub(0,255,255);
    glVertex3d(1,1,-1);
    glVertex3d(1,-1,-1);
    glVertex3d(-1,-1,-1);
    glVertex3d(-1,1,-1);
    
    glColor3ub(255,0,255);
    glVertex3d(1,-1,1);
    glVertex3d(1,1,1);
    glVertex3d(-1,1,1);
    glVertex3d(-1,-1,1);
    
    glEnd();


}


void CInternalSceneNode::render() {
	iterator i = this->begin();
    iterator end = this->end();
    
    while (i != end) {
        glPushMatrix();
        (*i)->render();
        glPopMatrix();
        ++i;
    }
}

void CInternalSceneNode::update(const f64& delta) {
	iterator i = this->begin();
	iterator end = this->end();

	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}

/*
bool SceneManager::init() {
	return true;
}

bool SceneManager::deinit() {

	return true;
}
*/

void CSceneManager::render() const {
    if(m_camera /*&& m_root*/) {
		m_camera->setView();
		//m_root->render();
    }
}

void CSceneManager::update(const f64& delta) {
	if (m_camera /*&& m_root*/) {
		m_camera->update(delta);
		//m_root->update(delta);
	}
}