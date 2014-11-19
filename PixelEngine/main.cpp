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




int main(int argc, const char * argv[]){

	std::shared_ptr<Video> video = Video::getInstance();
//	std::shared_ptr<TextureManager>  textureManager = TextureManager::getInstance();
	std::shared_ptr<SceneManager> sceneManager = SceneManager::getInstance();

	std::shared_ptr<DummyNode> dummyNode(new DummyNode());
	std::shared_ptr<Camera> camera(new CameraFree());
	std::shared_ptr<Q3Level> q3bsp(new Q3Level());

	sceneManager->attachCamera(camera);
	sceneManager->addSceneNode(q3bsp);
//    sceneManager->addSceneNode(dummyNode);
    
	camera->init();
	camera->getPosition()[0] = 0.;
	camera->getPosition()[1] = 0.;
	camera->getPosition()[2] = 50.;

	if (!q3bsp->load("bsp/q3dm1.bsp")) {
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

    
    const GLdouble deltaTime = 10.; // 10ms
	GLdouble lastTime = (GLdouble) glfwGetTime()*1000;
    GLdouble timer = (GLdouble) glfwGetTime();
    GLint updates = 0;
    GLint frames = 0;
    
//    std::cout << lastTime << std::endl;

	while (!video->windowShouldClose()) {
  
		GLdouble presentTime = (GLdouble) glfwGetTime()*1000;
        frames++;
        
		while (lastTime + deltaTime <= presentTime) {
            
			sceneManager->update(deltaTime);

			lastTime += deltaTime;
            updates++;
		}
        
		// Rendering
		video->beginScene();
		sceneManager->render();
		video->endScene();
        
        
        if(((GLdouble) glfwGetTime()) - timer > 1.) {
            std::cout << updates << " ups, " << frames << " fps" << std::endl;
            updates = 0;
            frames = 0;
            timer += 1.;
        }

		if (glfwGetKey(Video::getInstance()->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(Video::getInstance()->getWindow(), GL_TRUE);
		}
        
		glfwPollEvents();
	}

    return 0;
}