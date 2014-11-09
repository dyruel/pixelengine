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
#include "Texture.h"
#include "Q3Bsp.h"
#include "Q3Shader.h"

const GLdouble deltaTime = .01; // 1/100th seconds


int main(int argc, const char * argv[]){

	std::shared_ptr<Video> video = Video::getInstance();
	std::shared_ptr<TextureManager>  textureManager = TextureManager::getInstance();
	std::shared_ptr<SceneManager> sceneManager = SceneManager::getInstance();
    std::shared_ptr<Q3ShaderManager> shaderManager = Q3ShaderManager::getInstance();

	std::shared_ptr<DummyNode> dummyNode(new DummyNode());
	std::shared_ptr<Camera> camera(new CameraFree());
	std::shared_ptr<Q3Bsp> q3bsp(new Q3Bsp());

	sceneManager->attachCamera(camera);
	sceneManager->addSceneNode(q3bsp);
    
	camera->init();
	camera->getPosition()[0] = 0.;
	camera->getPosition()[1] = 0.;
	camera->getPosition()[2] = 50.;


	shaderManager->loadFromFile("scripts/base.shader");
	shaderManager->loadFromFile("scripts/base_button.shader");
	shaderManager->loadFromFile("scripts/base_floor.shader");
	shaderManager->loadFromFile("scripts/base_light.shader");
	shaderManager->loadFromFile("scripts/base_object.shader");
	shaderManager->loadFromFile("scripts/base_support.shader");
	shaderManager->loadFromFile("scripts/base_trim.shader");
	shaderManager->loadFromFile("scripts/base_wall.shader");
	shaderManager->loadFromFile("scripts/common.shader");
	shaderManager->loadFromFile("scripts/ctf.shader");
	shaderManager->loadFromFile("scripts/eerie.shader");
	shaderManager->loadFromFile("scripts/gfx.shader");
	shaderManager->loadFromFile("scripts/gothic_block.shader");
	shaderManager->loadFromFile("scripts/gothic_floor.shader");
	shaderManager->loadFromFile("scripts/gothic_light.shader");
	shaderManager->loadFromFile("scripts/gothic_trim.shader");
	shaderManager->loadFromFile("scripts/gothic_wall.shader");
	shaderManager->loadFromFile("scripts/hell.shader");
	shaderManager->loadFromFile("scripts/liquid.shader");
	shaderManager->loadFromFile("scripts/menu.shader");
	shaderManager->loadFromFile("scripts/models.shader");
	shaderManager->loadFromFile("scripts/organics.shader");
	shaderManager->loadFromFile("scripts/sfx.shader");
	shaderManager->loadFromFile("scripts/shrine.shader");
	shaderManager->loadFromFile("scripts/skin.shader");
	shaderManager->loadFromFile("scripts/sky.shader");

	if (!q3bsp->load("bsp/q3dm1.bsp")) {
		return 0;
	}

	q3bsp->attachCamera(camera);

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

    
	GLdouble lastTime = (GLdouble) glfwGetTime();

	while (!video->windowShouldClose()) {
        
		GLdouble presentTime = (GLdouble) glfwGetTime();
        
		while (lastTime + deltaTime <= presentTime) {
            
			// Logic stuff here
            
			sceneManager->update(deltaTime);

			lastTime += deltaTime;
		}
        
		// Rendering
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