//
//  main.cpp
//  Escape
//
//  Created by morgan on 20/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <iostream>

#include "main.h"

#include "Video.h"
#include "Scene.h"
#include "Q3Level.h"
#include "File.h"

int main(int argc, const char * argv[]){

	CVideo* video = CVideo::getInstance();
	CSceneManager* sceneManager = CSceneManager::getInstance(); // Each manager should have its own memory block

	//std::shared_ptr<DummyNode> dummyNode(new DummyNode());
	std::shared_ptr<Camera> camera(new CameraFree()); 
	std::shared_ptr<CQ3Map> q3bsp(new CQ3Map()); 

	//sceneManager->attachCamera(camera);
	//1sceneManager->addSceneNode(q3bsp);
//    sceneManager->addSceneNode(dummyNode);
    
	camera->init();
	camera->getPosition()[0] = 0.;
	camera->getPosition()[1] = 0.;
	camera->getPosition()[2] = 50.;

	
	if (!FileSystem::getInstance()->mount("data/pak0.pk3")) {
		ILogger::log("The file pak0.pk3 was not found.\n");
		return 0;
	}

	/*
	char ** i = PHYSFS_getSearchPath();
	for (; *i != NULL; i++) {
		ILogger::log("[%s] is in the search path.\n", *i);
	}
	//PHYSFS_freeList(i);
	*/

	if (!q3bsp->load("maps/q3dm1.bsp")) {
		return 0;
	}
	
	
//	Q3Shader shader;

	/*
    const std::map<std::string, Q3Shader>& shaders = shaderManager->getShaders();
    
    std::map<std::string, Q3Shader>::const_iterator i = shaders.begin();
    std::map<std::string, Q3Shader>::const_iterator end = shaders.end();
    
    while (i != end) {
        std::cout <<  (*i).first << " : "  << std::endl;
        ++i;
    }
    */

//	initVBOs();
//	bufferVBOs();

    
    const GLdouble deltaTime = 10.; // 10ms
	GLdouble lastTime = (GLdouble) glfwGetTime()*1000;
    GLdouble timer = (GLdouble) glfwGetTime();
    GLint updates = 0;
    GLint frames = 0;

	GLdouble renderTime = 0.0;
	GLdouble updateTime = 0.0;
    

	while (!video->windowShouldClose()) {
		
		
		GLdouble presentTime = (GLdouble) glfwGetTime()*1000;
        frames++;
        
		while (lastTime + deltaTime <= presentTime) {
            
			updateTime = glfwGetTime() * 1000;
			//sceneManager->update(deltaTime);
//			q3bsp->update(deltaTime);
			updateTime = glfwGetTime() * 1000 - updateTime;

			lastTime += deltaTime;
            updates++;
		}
        
		// Rendering
		renderTime = glfwGetTime() * 1000;
		video->beginFrame();
//		q3bsp->render();
		//sceneManager->render();
		video->endFrame();
		renderTime = glfwGetTime() * 1000 - renderTime;
        
        if(((GLdouble) glfwGetTime()) - timer > 1.) {
			std::cout << (1.0 / updates) * 1000 << " ms/u, " << (1.0 / frames) * 1000 << " ms/f, " << 0 << "ms (pushing time) " << renderTime << " " << updateTime << std::endl;
            updates = 0;
            frames = 0;
            timer += 1.;
        }


        
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cout << gluErrorString(err) << std::endl;
		}
		

		if (glfwGetKey(CVideo::getInstance()->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(CVideo::getInstance()->getWindow(), GL_TRUE);
		}
		
		glfwPollEvents();
	}

    return 0;
}