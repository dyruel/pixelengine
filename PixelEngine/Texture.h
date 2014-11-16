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


enum {
	TEXTURE_NOMIPMAP = 1 << 0,
	TEXTURE_CLAMP	 = 1 << 1,
};

struct Texture {
    std::string     m_name;
    int             m_texId;
    GLuint          m_flags;
    
    Texture()
    :m_name(""), m_texId(-1), m_flags(0) {}
    
    void clear() {
//        if(glIsTexture(m_texId)) {
//            glDeleteTextures(1, &m_texId);
//        }
        
        m_name  = "";
        m_texId = -1;
        m_flags = 0;
    }
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
			return m_textures[filename];
		}
		else {
			return _loadTextureFromFile(filename, flags);
		}
	}

    void getTexture(Texture& tex) {
        tex.m_texId = this->getTexture(tex.m_name, tex.m_flags);
    }
    
};

#endif /* defined(__PixelEngine__Texture__) */
