//
//  Texture.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__Texture__
#define __PixelEngine__Texture__

#define GLFW_INCLUDE_GLU

#include <GLFW/glfw3.h>
#include <IL/il.h>
#include <iostream>
#include <vector>
#include <memory>

#include "Singleton.h"

typedef	struct {
	std::string name;
    GLubyte	* imageData;									// Image Data (Up To 32 Bits)
    GLuint	bpp;											// Image Color Depth In Bits Per Pixel
    GLuint	width;											// Image Width
    GLuint	height;											// Image Height
    GLuint	texId;											// Texture ID Used To Select A Texture
    GLuint	type;											// Image Type (GL_RGB, GL_RGBA)
} Texture;

//typedef std::vector<Texture> TextureList;


class TextureManager : public Singleton<TextureManager> {
	friend class Singleton<TextureManager>;

private:

	bool init();
	bool deinit();

	TextureManager() {};
	TextureManager(TextureManager&);
	void operator =(TextureManager&);

public:

	virtual ~TextureManager() {};

	bool loadTextures(const std::vector<std::string>& files, std::unique_ptr<GLuint[]>& ids);

	GLuint loadTextureFromFile(std::string filename);
    
//    void bind(GLuint id);
};

#endif /* defined(__PixelEngine__Texture__) */
