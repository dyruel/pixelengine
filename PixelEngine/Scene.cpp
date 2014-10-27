//
//  Scene.cpp
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Scene.h"


#include <iostream>


void DummyNode::render() {
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


void SceneNode::render() {
    SceneNodeList::iterator i = m_children.begin();
    SceneNodeList::iterator end = m_children.end();
    
    while (i != end) {
        glPushMatrix();
        (*i)->render();
        glPopMatrix();
        ++i;
    }
}

void SceneNode::update(GLdouble delta) {
	SceneNodeList::iterator i = m_children.begin();
	SceneNodeList::iterator end = m_children.end();

	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}


bool SceneManager::init() {
	m_root.reset(new SceneNode());
	return true;
}

bool SceneManager::deinit() {

	return true;
}

void SceneManager::render() const {
    if(m_camera && m_root) {
        m_camera->setView();
        m_root->render();
    }
}

void SceneManager::update(GLdouble delta) {
	if (m_camera && m_root) {
		m_camera->update(delta);
		m_root->update(delta);
	}
}