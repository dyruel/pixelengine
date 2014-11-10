//
//  Q3Shader.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include "Q3Shader.h"
#include "Logger.h"
#include "Texture.h"

bool Q3ShaderManager::loadFromFile(const char* filename) {
	std::ifstream file(filename, std::ios::in);
	std::stringstream sstream;
    Q3Shader shader;
	Q3ShaderPass shaderPass;
    std::string token = "";
    
    std::vector<std::string> args;

	ILogger::log("Q3Shader:: Loading %s ...\n", filename);

	if (!file.is_open()) {
		ILogger::log("Unable to open the shader script %s.\n", filename);
		return false;
	}

	sstream << file.rdbuf();
	std::string buffer(sstream.str());
	std::string::iterator bufferEnd = buffer.end();

    bool readComment = false;
	m_readingState = SP_BEGIN_COM;
    m_depth = 0;
    
    shader.clear();
	shaderPass.clear();
    args.clear();


    std::string::iterator i = buffer.begin();
    
    while (i != bufferEnd) {
		char c = *i;

        // Check for comments
        if (m_readingState != SP_READ_COMMENT && c == '/' && (i + 1) != bufferEnd && *(i + 1) == '/') {
            i+=2;
            readComment = true;
        }
        
        if(readComment) {
            if(c == '\n' || c == '\r') {
                readComment = false;
            }
            
            ++i;
            continue;
        }
        
        // Check for new node
        if(!readComment && c == '{') {
            m_readingState = SP_BEGIN_NODE;
            ++i;

        }
        else if(!readComment && c == '}') {
            m_readingState = SP_END_NODE;
            ++i;
        }
        
        
        // Check the end of a command
        if(!readComment && (c == '\n' || c == '\r') && (m_readingState == SP_READ_NAME || m_readingState == SP_READ_VALUE || m_readingState == SP_FIND_VALUE)) {
            m_readingState = SP_END_COM;
            ++i;
        }
        
        
        // Parse a command
        switch (m_readingState) {
                
            case SP_FIND_NAME:
                
                if(c == '\n' || c == ' ' || c == '\t' || c == '\r') {
                    ++i;
                }
                else {
                    m_readingState = SP_READ_NAME;
                }
                
                break;
                
                
            case SP_READ_NAME:
                
                if(c == ' ' || c == '\t') {
//					std::transform(token.begin(), token.end(), token.begin(), ::tolower);
                    args.push_back(token);
                    token.clear();
                    m_readingState = SP_FIND_VALUE;
                }
                else {
                    token.push_back(tolower(c));
                    ++i;
                }
                
                break;
                
                
            case SP_FIND_VALUE:

                if(c == ' ' || c == '\t') {
                    ++i;
                }
                else {
                    m_readingState = SP_READ_VALUE;
                }
                
                break;
                
                
            case SP_READ_VALUE:
                
                if(c == ' ' || c == '\t') {
                    args.push_back(token);
                    token.clear();
                    m_readingState = SP_FIND_VALUE;
                    ++i;
                }
                else {
					token.push_back(tolower(c));
                    ++i;
                }
                
                break;
                
                
            case SP_BEGIN_NODE:
                
                ++m_depth;
                
                if(m_depth > 2) {
                    ILogger::log("Q3Shader:: Syntax error in %s \n", filename);
                    return false;
                }
                
                m_readingState = SP_BEGIN_COM;
                
                break;
                
                
            case SP_END_NODE:
                
                if(m_depth > 0) {
                    --m_depth;
                }
                else {
                    ILogger::log("Q3Shader:: Syntax error in %s \n", filename);
                    return false;
                }
                
                if(m_depth == 0) {
					
					if (shader.m_shaderPasses.size() > 0 &&
						!(shader.m_flags & SHADER_DEPTHWRITE) &&
						!(shader.m_flags & SHADER_TRANSPARENT) &&
						!(shader.m_flags & SHADER_SKY)) 
					{
						shader.m_shaderPasses[0].m_flags |= SHADER_DEPTHWRITE;
					}
					
                    m_shaders[shader.name] = shader;
                    shader.clear();
                }
                else { // end of a shader pass
					shader.addShaderPass(shaderPass);
					shaderPass.clear();
                }
                
              
                m_readingState = SP_BEGIN_COM;
                
                break;
                
  
            case SP_BEGIN_COM:
                m_readingState = SP_FIND_NAME;
                break;
                
                
            case SP_END_COM:
                
                if(!token.empty()) {
                    args.push_back(token);
                    token.clear();
                }
                
                if(!args.empty()) {

                    if(m_depth == 0) { // We've read the shader's name
                        shader.name = args[0];
                    }
					else if (m_depth == 1){ // Global command
						if (!args[0].compare("cull")) {

							if (args.size() > 1) {
								if (!args[1].compare("none") || !args[1].compare("disable")) {
									shader.m_flags |= SHADER_NOCULL;
								}
								else if (!args[1].compare("front")){
									shader.m_flags |= SHADER_FRONTCULL;
								}
								else if (!args[1].compare("back")){
									shader.m_flags |= SHADER_BACKCULL;
								}
							}

						}
						else if (!args[0].compare("surfaceparm")) {
							if (!args[1].compare("trans")) {
								shader.m_flags |= SHADER_TRANSPARENT;
							}
							else if (!args[1].compare("sky")) {
								shader.m_flags |= SHADER_SKY;
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with surfaceparm\n", shader.name.c_str(), args[1].c_str());
							}
						}
						else if (!args[0].compare("skyparms")) {

						}
						else {
							ILogger::log("Q3Shader::Syntax error in %s: The keyword %s does not exist.\n", shader.name.c_str(), args[0].c_str());
						}
					}
					else { // Shader pass command
						if (!args[0].compare("animmap")) {
							shaderPass.m_flags |= SHADER_ANIMMAP;
							shaderPass.m_animSpeed = atof(args[1].c_str());

							std::vector<std::string>::iterator j = args.begin() + 2;

							unsigned int k = 0;
							while (j != args.end() && k < SHADER_MAX_FRAMES){

								
								shaderPass.m_animFrames[k] = TextureManager::getInstance()->getTexture(*j);
//								std::cout << *j << " " << shaderPass.m_animFrames[k] << std::endl;
								++j;
								++k;
							}
							shaderPass.m_animNumframes = k;

						}
						else if (!args[0].compare("blendfunc")) {
							shaderPass.m_flags |= SHADER_BLENDFUNC;

							if (args.size() == 2)
							{
								if (!args[1].compare("blend"))
								{
									shaderPass.m_blendSrc = GL_SRC_ALPHA;
									shaderPass.m_blendDst = GL_ONE_MINUS_SRC_ALPHA;
								}
								else if (!args[1].compare("filter"))
								{
									shaderPass.m_blendSrc = GL_DST_COLOR;
									shaderPass.m_blendDst = GL_ZERO;
								}
								else if (!args[1].compare("add"))
								{
									shaderPass.m_blendSrc = shaderPass.m_blendDst = GL_ONE;
								}
								else {
									ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with blendfunc\n", shader.name.c_str(), args[1].c_str());
								}
							}
							else
							{						
								for (int j = 1; j < 3; ++j)
								{
									GLenum& blend = j == 1 ? shaderPass.m_blendSrc : shaderPass.m_blendDst;

									if (!args[j].compare("gl_zero"))
										blend = GL_ZERO;
									else if (!args[j].compare("gl_one"))
										blend = GL_ONE;
									else if (!args[j].compare("gl_dst_color"))
										blend = GL_DST_COLOR;
									else if (!args[j].compare("gl_one_minus_src_alpha"))
										blend = GL_ONE_MINUS_SRC_ALPHA;
									else if (!args[j].compare("gl_src_alpha"))
										blend = GL_SRC_ALPHA;
									else if (!args[j].compare("gl_src_color"))
										blend = GL_SRC_COLOR;
									else if (!args[j].compare("gl_one_minus_dst_color"))
										blend = GL_ONE_MINUS_DST_COLOR;
									else if (!args[j].compare("gl_one_minus_src_color"))
										blend = GL_ONE_MINUS_SRC_COLOR;
									else if (!args[j].compare("gl_dst_alpha"))
										blend = GL_DST_ALPHA;
									else if (!args[j].compare("gl_one_minus_dst_alpha"))
										blend = GL_ONE_MINUS_DST_ALPHA;
									else {
										ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with blendfunc\n", shader.name.c_str(), args[j].c_str());
									}
										
								}
							}
						}
						else if (!args[0].compare("map")) {
							if (!args[1].compare("$lightmap")) {
								shaderPass.m_flags |= SHADER_LIGHTMAP;
							}
							else {
								shaderPass.m_texId = TextureManager::getInstance()->getTexture(args[1]);
							}

						}
						else if (!args[0].compare("rgbgen")) {

						}
						else if (!args[0].compare("tcgen")) {

						}
						else if (!args[0].compare("tcmod")) {

						}
						else if (!args[0].compare("clampmap")) {
							shaderPass.m_texId = TextureManager::getInstance()->getTexture(args[1], TEXTURE_CLAMP);
						}
						else if (!args[0].compare("alphafunc")) {
							shaderPass.m_flags |= SHADER_ALPHAFUNC;

							if (!args[1].compare("gt0"))
							{
								shaderPass.m_alphaFunc = GL_GREATER;
								shaderPass.m_alphaFuncRef = 0.0f;
							}
							else if (!args[1].compare("ge128"))
							{
								shaderPass.m_alphaFunc = GL_GEQUAL;
								shaderPass.m_alphaFuncRef = 0.5f;
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with alphafunc\n", shader.name.c_str(), args[1].c_str());
							}

						}
						else if (!args[0].compare("depthfunc")) {
							if (!args[1].compare("equal")) {
								shaderPass.m_depthFunc = GL_EQUAL;
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with depthfunc\n", shader.name.c_str(), args[1].c_str());
							}
						}
						else if (!args[0].compare("depthwrite")) {
							shaderPass.m_flags |= SHADER_DEPTHWRITE;
						}
						else {
							ILogger::log("Q3Shader::Syntax error in %s: The keyword %s does not exist\n", shader.name.c_str(), args[0].c_str());
						}


					}
					/*
                    std::cout << "# ";
                    std::vector<std::string>::iterator m = args.begin();
                    std::vector<std::string>::iterator end = args.end();
                    
                    while (m != end) {
                        std::cout <<  (*m) << " ";
                        ++m;
                    }
                    
                    std::cout << std::endl;
					*/
                }
                
                args.clear();
                m_readingState = SP_BEGIN_COM;
                break;
                

            default:
                break;
        }
        
        
        
	}
    
    if(m_depth > 0) {
        ILogger::log("Q3Shader:: Syntax error in %s \n", filename);
        return false;
    }
    

	file.close();
	return true;
}