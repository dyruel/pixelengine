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
//#include <IL/ilu.h>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "Logger.h"
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


enum {
	TEXTURE_NOMIPMAP = 1 << 0,
	TEXTURE_CLAMP	 = 1 << 1,
};

//typedef std::vector<Texture> TextureList;


class TextureManager : public Singleton<TextureManager> {
	friend class Singleton<TextureManager>;

private:

	bool init();
	bool deinit();

	std::map<std::string, GLuint> m_textures;

	TextureManager() {};
	TextureManager(TextureManager&);
	void operator =(TextureManager&);


	GLuint _loadTextureFromFile(const std::string& filename, int flags);


public:

	virtual ~TextureManager() {};

# if 0
	bool _loadTextures(const std::vector<std::string>& files, std::unique_ptr<GLuint[]>& ids);
#endif

	GLuint getTexture(const std::string& filename, int flags = 0) {
		if (m_textures.find(filename) != m_textures.end()) {
//			ILogger::log("hit for %s %d\n", filename.c_str(), m_textures[filename]);
			return m_textures[filename];
		}
		else {
//			ILogger::log("no hit for %s %d\n", filename.c_str(), _loadTextureFromFile(filename));
			return _loadTextureFromFile(filename, flags);
		}
	}
    
};

#endif /* defined(__PixelEngine__Texture__) */
