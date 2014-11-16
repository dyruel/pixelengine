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


#include "Texture.h"
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
	SHADER_TCGEN_ENV	= 1 << 12,
	SHADER_FRONTCULL	= 1 << 13,
	SHADER_BACKCULL		= 1 << 14,
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


//class Q3ShaderPass;
//



class Q3ShaderPass {
    friend class Q3ShaderManager;
    friend class Q3ShaderDefault;
    
private:
    
    enum {
        BLEND = 0,
        ALPHA_TEST,
        DEPTHWRITE,
        NO_CULL,
        NUMSTATES
    };
    
    Texture         m_Texture;
    
	unsigned int    m_flags;
    
    // OpenGL data for textures
    float *         m_TexCoordPointer;
    GLsizei         m_Stride;
    
    // OpenGL states
    GLboolean       m_oglStates[NUMSTATES];

	// Blending
	GLenum          m_blendSrc, m_blendDst;

	// Animmap
	float           m_animSpeed;
	int             m_animNumframes;
    Texture         m_animFrames[SHADER_MAX_FRAMES];
	float           m_frame;

	// Alpha func
	GLenum          m_alphaFunc;
	GLclampf        m_alphaFuncRef;

	// Depth func
	GLenum          m_depthFunc;
    
    
    void _saveOglSates();
    void _restoreOglStates();
    
    void _setTexCoordPointer(float * texCoordPointer, GLsizei stride) {
        m_TexCoordPointer = texCoordPointer;
        m_Stride = stride;
    }
    
public:
    
    Q3ShaderPass()
    : m_TexCoordPointer{nullptr}, m_Stride(0)
    { this->clear(); }
    ~Q3ShaderPass() {}

	void clear() {
        m_Texture.clear();
		m_flags             = SHADER_DEPTHWRITE;
		m_blendSrc          = GL_DST_COLOR;
		m_blendDst          = GL_ZERO;
		m_animSpeed         = 0.0f;
		m_animNumframes     = 0;
		m_frame             = 0;
		m_alphaFunc         = 0;
		m_alphaFuncRef      = 0.0f;
		m_depthFunc         = GL_LEQUAL;
	}
    
    void update(double delta);
        
    void begin();
    void end();

};


//FIXME : should extend std::vector<Q3ShaderPass>
class Q3Shader {
    friend class Q3ShaderManager;

protected:

    std::string name;
	unsigned int m_flags;
    std::vector<Q3ShaderPass> m_shaderPasses;

    void _saveOglSates();
    void _restoreOglStates();

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

	const std::string& getName() const { return name;  }
    
    
    void begin();
    
    void end();
};


class Q3ShaderDefault : public Q3Shader  {

public:
	Q3ShaderDefault(const Texture tex[2], float * texCoordPointer[2], const GLsizei stride[2]) {
		Q3ShaderPass shaderPass;

		name = "default";

		m_flags = 0;

		shaderPass.clear();
		shaderPass.m_flags = SHADER_LIGHTMAP | SHADER_DEPTHWRITE;
        shaderPass.m_Texture = tex[1];
		shaderPass.m_depthFunc = GL_LEQUAL;
        shaderPass._setTexCoordPointer(texCoordPointer[1], stride[1]);
		m_shaderPasses.push_back(shaderPass);

		shaderPass.clear();
		shaderPass.m_flags = SHADER_BLENDFUNC | SHADER_DEPTHWRITE;
        shaderPass.m_Texture = tex[0];
		shaderPass.m_blendSrc = GL_DST_COLOR;
		shaderPass.m_blendDst = GL_ZERO;
		shaderPass.m_depthFunc = GL_LEQUAL;
        shaderPass._setTexCoordPointer(texCoordPointer[0], stride[0]);
		m_shaderPasses.push_back(shaderPass);

	};
	virtual ~Q3ShaderDefault() {};
};



class Q3ShaderManager : public Singleton<Q3ShaderManager> {
    friend class Singleton<Q3ShaderManager>;
    
private:
    
    std::map<std::string, Q3Shader> m_shaders;
    
    void _loadCommand();
    
    bool init();
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

    const Q3Shader& getShader(const std::string& name)  {
			return m_shaders.at(name); 
	};

	bool exists(const std::string& name) {
		return m_shaders.find(name) != m_shaders.end();
	}
};

#endif