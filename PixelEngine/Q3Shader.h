//
//  Q3Shader.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

// Doc : http://www.heppler.com/shader/

#ifndef PixelEngine_Q3Shader_h
#define PixelEngine_Q3Shader_h

#include <GLFW/glfw3.h>
#include <vector>
#include <map>

#include "Singleton.h"


#define SHADER_MAX_FRAMES 10


enum {
	SHADER_NOCULL		= 1 << 0,
	SHADER_TRANSPARENT	= 1 << 1,
	SHADER_DEPTHWRITE	= 1 << 2, 
	SHADER_SKY			= 1 << 3,
	SHADER_NOMIPMAPS	= 1 << 4,
	SHADER_NEEDCOLOURS	= 1 << 5,
	SHADER_DEFORMVERTS	= 1 << 6,
	SHADER_LIGHTMAP		= 1 << 7,
	SHADER_BLENDFUNC	= 1 << 8,
	SHADER_ALPHAFUNC	= 1 << 9,
	SHADER_TCMOD		= 1 << 10,
	SHADER_ANIMMAP		= 1 << 11,
	SHADER_TCGEN_ENV	= 1 << 12
};

enum {
	SP_READ_NAME        = 1 << 0,
	SP_BEGIN_NODE       = 1 << 1,
	SP_END_NODE         = 1 << 2,
    SP_READ_COMMENT     = 1 << 3,
    SP_FIND_NAME        = 1 << 4,
    SP_FIND_VALUE       = 1 << 5,
    SP_READ_VALUE       = 1 << 6,
    SP_END_COM          = 1 << 7,
    SP_BEGIN_COM        = 1 << 8,
};


typedef	struct {
	GLuint m_texId;
	unsigned int m_flags;

	GLenum m_blendSrc, m_blendDst;

	float m_animSpeed;
	int m_animNumframes;
	GLuint m_animFrames[SHADER_MAX_FRAMES];
	float m_frame;

	void clear() {
		m_texId = 0;
		m_flags = 0;
		m_blendSrc = GL_DST_COLOR;
		m_blendDst = GL_ZERO;
		m_animNumframes = 0;
		m_frame = 0;
	}

} Q3ShaderPass;


class Q3Shader {
    friend class Q3ShaderManager;


protected:

    std::string name;
	unsigned int m_flags;
    std::vector<Q3ShaderPass> m_shaderPasses;



public:
	Q3Shader() { };
	virtual ~Q3Shader() {};

	void addShaderPass(const Q3ShaderPass& shaderPass) { m_shaderPasses.push_back(shaderPass); };
	std::vector<Q3ShaderPass>& getShaderPasses() { return m_shaderPasses; };
    virtual void clear() {
        name.clear();
        m_flags = 0;
        m_shaderPasses.clear();
    }

	const unsigned int getFlags() const { return m_flags; }

};


class Q3ShaderDefault : public Q3Shader  {

public:
	Q3ShaderDefault(int texId) {
		Q3ShaderPass shaderPass;

		m_flags = 0;

//		shaderPass.flags = SHADER_LIGHTMAP | SHADER_DEPTHWRITE;
		shaderPass.m_flags = 0;
		shaderPass.m_texId = -1;
		m_shaderPasses.push_back(shaderPass);

		shaderPass.m_flags = SHADER_BLENDFUNC;
		shaderPass.m_texId = texId;
		shaderPass.m_blendSrc = GL_DST_COLOR;
		shaderPass.m_blendDst = GL_ZERO;

		m_shaderPasses.push_back(shaderPass);

	};
	virtual ~Q3ShaderDefault() {};
};



class Q3ShaderManager : public Singleton<Q3ShaderManager> {
    friend class Singleton<Q3ShaderManager>;
    
private:
    
    std::map<std::string, Q3Shader> m_shaders;
    
    void _loadCommand();
    
    bool init() { return true; };
    bool deinit() { return true; };
    
    // Variables used in the parser
    unsigned int m_readingState;
    unsigned int m_depth;
    
    Q3ShaderManager() {};
    Q3ShaderManager(Q3ShaderManager&);
    void operator =(Q3ShaderManager&);
    
public:
    
    virtual ~Q3ShaderManager() {};
    
    bool loadFromFile(const char* filename);
    
    const std::map<std::string, Q3Shader>& getShaders() const { return m_shaders; };

    Q3Shader& getShader(const std::string& name)  {
			return m_shaders.at(name); 
	};

	bool exists(const std::string& name) {
		return m_shaders.find(name) != m_shaders.end();
	}
};

#endif