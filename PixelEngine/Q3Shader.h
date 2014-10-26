//
//  Q3Shader.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


#ifndef PixelEngine_Q3Shader_h
#define PixelEngine_Q3Shader_h

#include <GLFW/glfw3.h>
#include <vector>

/* Shader flags */
enum
{
	SHADER_NOCULL = 1 << 0,
	SHADER_TRANSPARENT = 1 << 1,
	SHADER_DEPTHWRITE = 1 << 2,  /* Also used for pass flag */
	SHADER_SKY = 1 << 3,
	SHADER_NOMIPMAPS = 1 << 4,
	SHADER_NEEDCOLOURS = 1 << 5,
	SHADER_DEFORMVERTS = 1 << 6
};

/* Shaderpass flags */
enum
{
	SHADER_LIGHTMAP = 1 << 0,
	SHADER_BLEND = 1 << 1,
	SHADER_ALPHAFUNC = 1 << 3,
	SHADER_TCMOD = 1 << 4,
	SHADER_ANIMMAP = 1 << 5,
	SHADER_TCGEN_ENV = 1 << 6
};


typedef	struct {
	int texId;
	unsigned int flags;

	GLenum blendSrc, blendDst;

} Q3ShaderPass;


class Q3Shader {

protected:

	unsigned int m_flags;
	std::vector<Q3ShaderPass> m_shaderPasses;

public:
	Q3Shader() { };
	virtual ~Q3Shader() {};

	void addShaderPasse(const Q3ShaderPass& shaderPass) { m_shaderPasses.push_back(shaderPass); };
	const std::vector<Q3ShaderPass>& getShaderPasses() const { return m_shaderPasses; };

};


class Q3ShaderDefault : public Q3Shader  {

public:
	Q3ShaderDefault(int texId) {
		Q3ShaderPass shaderPass;

		m_flags = 0;

		shaderPass.flags = SHADER_LIGHTMAP | SHADER_DEPTHWRITE;
		shaderPass.texId = -1;
		m_shaderPasses.push_back(shaderPass);

		shaderPass.flags = SHADER_BLEND;
		shaderPass.texId = texId;
		shaderPass.blendSrc = GL_DST_COLOR;
		shaderPass.blendDst = GL_ZERO;

		m_shaderPasses.push_back(shaderPass);

	};
	virtual ~Q3ShaderDefault() {};
};

#endif