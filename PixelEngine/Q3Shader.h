//
//  Q3Shader.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


#ifndef PixelEngine_Q3Shader_h
#define PixelEngine_Q3Shader_h

#include <vector>


typedef	struct {
	int texId;

} Q3ShaderPass;




class Q3Shader {

protected:

	std::vector<Q3ShaderPass> m_shaderPasses;

public:
	Q3Shader(int numPasses = 1) { /*Q3ShaderPass shaderPass; m_shaderPasses.reserve(numPasses); m_shaderPasses.push_back(shaderPass);*/ };
	virtual ~Q3Shader() {};

	void addShaderPasse(const Q3ShaderPass& shaderPass) { m_shaderPasses.push_back(shaderPass); };
	const std::vector<Q3ShaderPass>& getShaderPasses() const { return m_shaderPasses; };

};

#endif