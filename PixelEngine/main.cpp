//
//  main.cpp
//  Escape
//
//  Created by morgan on 20/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "main.h"

#include "Video.h"
#include "Scene.h"
#include "Texture.h"
#include "Bsp.h"

/*
 
 Windjammers like games
 Graphism like BaraBariBall de Sportsfriends
 3 classes de personnages : rapide, equilibré, lourd
 
 */

const GLdouble deltaTime = .01; // 1/100th seconds

TextureList textureList;

int main(int argc, const char * argv[]){

    Video* video = Video::getInstance();
    TextureManager *  textureManager = TextureManager::getInstance();
	SceneManager * sceneManager = SceneManager::getInstance();



    std::shared_ptr<DummyNode> dummyNode(new DummyNode());
	std::shared_ptr<CameraFree> camera(new CameraFree());
	std::shared_ptr<Bsp> bsp(new Bsp());

	sceneManager->setCamera(camera);
	sceneManager->addSceneNode(bsp);
    
    camera->init();
	bsp->load("test.bsp");
    
	GLdouble lastTime = (GLdouble) glfwGetTime();

    while (!video->windowShouldClose()) {
        
		GLdouble presentTime = (GLdouble) glfwGetTime();
        
        while (lastTime + deltaTime <= presentTime) {
            
            // Logic stuff here
            
			sceneManager->update(deltaTime);

            lastTime += deltaTime;
        }
        
        video->beginScene();
		sceneManager->render();
		video->endScene();

		if (glfwGetKey(Video::getInstance()->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(Video::getInstance()->getWindow(), GL_TRUE);
		}
        
        glfwPollEvents();
    }

	SceneManager::drop();
	TextureManager::drop();
	Video::drop();
	    
    return 0;
}