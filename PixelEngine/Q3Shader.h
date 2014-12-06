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


#include <vector>
#include <map>

#include "Opengl.h"
#include "Texture.h"
#include "Singleton.h"
#include "Logger.h"
#include "Vector.h"

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



class Q3ShaderPass {
    
private:
    
    enum {
        BLEND = 0,
        ALPHA_TEST,
        DEPTHWRITE,
        NO_CULL,
        NUMSTATES
    };
    
    Texture                 m_Texture;
    
	unsigned int            m_flags;
    
    // OpenGL states
    GLboolean               m_oglStates[NUMSTATES];

	// Blending
	GLenum                  m_blendSrc, m_blendDst;

	// Animmap
	float                   m_animSpeed;
    std::vector<Texture>    m_animFrames;
	float                   m_frame;

	// Alpha func
	GLenum                  m_alphaFunc;
	GLclampf                m_alphaFuncRef;

	// Depth func
	GLenum                  m_depthFunc;
    
    
    void _saveOglSates();
    void _restoreOglStates();
    
public:
    
    Q3ShaderPass()
    : m_flags(0), m_blendSrc(GL_DST_COLOR), m_blendDst(GL_ZERO),
      m_animSpeed(0.0f), m_frame(0.0f),
      m_alphaFunc(0), m_alphaFuncRef(0.0f), m_depthFunc(GL_LEQUAL) {}
    ~Q3ShaderPass() {}

    
    void setTexture(const Texture& texture) { m_Texture = texture; }
    const Texture& getTexture() const { return m_Texture; }
    void setFlags(const unsigned int& flags) { m_flags = flags; }
    void addFlag(const unsigned int& flag)   { m_flags |= flag; }
    const unsigned int getFlags() const { return m_flags; }
    void setBlending(const GLenum& blendSrc, const GLenum& blendDst) { m_blendSrc = blendSrc; m_blendDst = blendDst; };
    void setAnimation(const float& animSpeed, const std::vector<Texture>& animFrames) {
        m_animSpeed      = animSpeed;
        m_animFrames     = animFrames;
    }
    void setAlphaFunc(const GLenum& alphaFunc, const GLclampf& alphaFuncRef) { m_alphaFunc = alphaFunc; m_alphaFuncRef = alphaFuncRef; }
    void setDepthFunc(const GLenum& depthFunc) { m_depthFunc = depthFunc; }
    
    void init();
    void update(const double& delta);
    
    void start();
    void stop();

};


class Q3Shader {

public:
	Q3Shader()
    : m_name(""), m_flags(0) {};
	virtual ~Q3Shader() {};

	void addShaderPass(const Q3ShaderPass& shaderPass) { assert(m_numQ3ShaderPasses < MAX_SHADER_PASSES); m_q3shaderPasses[m_numQ3ShaderPasses++] = shaderPass; }
    
    void setFlags(const unsigned int& flags) { m_flags = flags; }
    void addFlag(const unsigned int& flag)   { m_flags |= flag; }
	const unsigned int& getFlags() const { return m_flags; }
    
    void setName(const std::string& name) { m_name = name; }
	const std::string& getName() const { return m_name;  }
    
	/*
    Q3Shader& operator=(const Q3Shader& shader)
    {
        if (this == &shader)
            return *this;
        
        m_name  = shader.m_name;
        m_flags = shader.m_flags;
        
		
        for (Q3Shader::const_iterator i = shader.begin(); i != shader.end(); ++i) {
            std::shared_ptr<Q3ShaderPass> shaderPass = std::make_shared<Q3ShaderPass>();
            *shaderPass = *(*i);
            push_back(shaderPass);
        }
        
        return *this;
    }
	*/

    void start();
    void stop();

protected:

	static const s32 MAX_SHADER_PASSES = 10;
	Q3ShaderPass m_q3shaderPasses[MAX_SHADER_PASSES];
	s32 m_numQ3ShaderPasses;

	std::string     m_name;
	unsigned int    m_flags;

	void _saveOglSates();
	void _restoreOglStates();
};


class Q3ShaderManager : public Singleton<Q3ShaderManager> {
    friend class Singleton<Q3ShaderManager>;
public:
    
    virtual ~Q3ShaderManager() {};
    
	s32 exists(const c8* name) { 

	}
    
    bool loadFromFile(const char* filename);

private:

	static const s32	MAX_SHADERS = 2000;
	Q3Shader			m_q3shaders[MAX_SHADERS];
	s32					m_numQ3Shaders;

	void _loadCommand();

	bool init();
	bool deinit() { return true; };

	// Variables used in the parser
	u32 m_readingState;
	u32 m_depth;

	Q3ShaderManager()
		: m_numQ3Shaders(0), m_readingState(0), m_depth(0) {};
	Q3ShaderManager(Q3ShaderManager&);
	void operator =(Q3ShaderManager&);

};

#endif