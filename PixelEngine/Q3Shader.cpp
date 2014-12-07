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
#include "Q3Map.h"



/*
const std::string& Q3ShaderParser::nexttok() {
    bool readComment = false;
    
    while (m_pos != m_end) {
        char c = *m_pos;
        
        if (c == '/' && (m_pos + 1) != end && *(m_pos + 1) == '/') {
            m_pos+=2;
            readComment = true;
            continue;
        }
        
        if(readComment) {
            if(c == '\n' || c == '\r') {
                readComment = false;
            }
            
            ++m_pos;
            continue;
        }
        
        
        ++m_pos;
    }
}
*/

void Q3ShaderPass::_saveOglSates() {
    m_oglStates[BLEND] = glIsEnabled(GL_BLEND);
    m_oglStates[ALPHA_TEST] = glIsEnabled(GL_ALPHA_TEST);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &m_oglStates[DEPTHWRITE]);
    // glIsEnabled(GL_COLOR_ARRAY) ?
}

void Q3ShaderPass::_restoreOglStates() {
    if (m_oglStates[BLEND]) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    else {
        glDisable(GL_BLEND);
    }
    
    if (m_oglStates[ALPHA_TEST]) {
        glEnable(GL_ALPHA_TEST);
    }
    else {
        glDisable(GL_ALPHA_TEST);
    }
    
    if (m_oglStates[DEPTHWRITE]) {
        glDepthMask(GL_TRUE);
    }
    else {
        glDepthMask(GL_FALSE);
    }
}


void Q3ShaderPass::start() {
//	glEnable(GL_TEXTURE_COORD_ARRAY);
	
//	glDisable(GL_TEXTURE_COORD_ARRAY);
//    std::cout << "Shader = " << m_Texture.m_Stride << " " << m_Texture.m_texCoordPointer << std::endl;
    
    this->_saveOglSates();
    
//    if (m_flags & SHADER_ANIMMAP) {
	glBindTexture(GL_TEXTURE_2D, m_frames[(int)m_frame].m_texId);
 //   }
 //   else  {
//		glBindTexture(GL_TEXTURE_2D, m_texIds[0]);
//    }
    
    if (m_flags & SHADER_BLENDFUNC) {
        glEnable(GL_BLEND);
        glBlendFunc(m_blendSrc, m_blendDst);
    }
    else {
        glDisable(GL_BLEND);
    }
    
    if (m_flags & SHADER_ALPHAFUNC) {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(m_alphaFunc, m_alphaFuncRef);
    }
    else {
        glDisable(GL_ALPHA_TEST);
    }
    
    glDepthFunc(m_depthFunc);
    
    if (m_flags & SHADER_DEPTHWRITE) {
        glDepthMask(GL_TRUE);
    }
    else {
        glDepthMask(GL_FALSE);
    }
}


void Q3ShaderPass::stop() {
    this->_restoreOglStates();
}






void Q3ShaderPass::update(const double& delta) {
    if (m_flags & SHADER_ANIMMAP) {
        m_frame += delta * m_animSpeed;
		if (((int)m_frame) >= m_numFrames)
            m_frame = 0.f;
    }
}


/*
inline void Q3Shader::_saveOglSates() {
    m_oglStates[CULL_FACE] = glIsEnabled(GL_CULL_FACE);
}

inline void Q3Shader::_restoreOglStates() {
    if (m_oglStates[CULL_FACE]) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
}
*/


void Q3Shader::start() {
    if (m_flags & SHADER_NOCULL) {
        glDisable(GL_CULL_FACE);
    }
    
}


void Q3Shader::stop() {
    if (m_flags & SHADER_NOCULL) {
        glEnable(GL_CULL_FACE);
    }
}



bool Q3ShaderManager::loadFromFile(const char* filename) {
	std::ifstream file(filename, std::ios::in);
	std::stringstream sstream;
    Q3Shader  shader;
	Q3ShaderPass  shaderPass;
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
    
	//shader = &m_q3shaders[m_numQ3Shaders];
	//shaderPass = &shader->m_q3shaderPasses[shader->m_numQ3ShaderPasses];
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
					/*
					if (shader.m_shaderPasses.size() > 0 &&
						!(shader.m_flags & SHADER_DEPTHWRITE) &&
						!(shader.m_flags & SHADER_TRANSPARENT) &&
						!(shader.m_flags & SHADER_SKY)) 
					{
						shader.m_shaderPasses[0].m_flags |= SHADER_DEPTHWRITE;
					}
					*/

                    (*this)[shader.getName()] = shader;
					shader.clear();

                    //shader = std::make_shared<Q3Shader>();

					//assert(m_numQ3Shaders+1 < MAX_SHADERS);
					/*
					if (m_numQ3Shaders + 1 < MAX_SHADERS) {
						++m_numQ3Shaders;
						shader = &m_q3shaders[m_numQ3Shaders];
					}
					else {
						ILogger::log("WARNING: Q3Shader:: The maximum number of shaders is attained. No more shaders will be loaded.\n");
						return true;
					}
					*/
					//m_q3shaders[m_numQ3Shaders++] = shader;
                }
                else { // end of a shader pass

					if (shaderPass.m_numFrames > Q3ShaderPass::MAX_ANIM_FRAMES) {
						std::cout << shader.getName() << " " << filename << std::endl;
					}


					shader.addShaderPass(shaderPass);
					shaderPass.clear();

					/*
					if (shader->m_numQ3ShaderPasses + 1 < Q3Shader::MAX_SHADER_PASSES) {
						++(shader->m_numQ3ShaderPasses);
						shaderPass = &(shader->m_q3shaderPasses[shader->m_numQ3ShaderPasses]);
					}
					else {
						ILogger::log("WARNING: Q3Shader:: The maximum number of shader passes is attained. No more shader pass will be loaded for the shader %s.\n", shader->getName().c_str());
						return true;
					}
					*/
                    //shader.addShaderPass(shaderPass);

					//shaderPass = std::make_shared<Q3ShaderPass>();
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
                        shader.setName(args[0]);
                    }
					else if (m_depth == 1){ // Global command
						if (!args[0].compare("cull")) {

							if (args.size() > 1) {
								if (!args[1].compare("none") || !args[1].compare("disable")) {
									shader.addFlag(SHADER_NOCULL);
								}
								else if (!args[1].compare("front")){
									shader.addFlag(SHADER_FRONTCULL);
								}
								else if (!args[1].compare("back")){
									shader.addFlag(SHADER_BACKCULL);
								}
							}

						}
						else if (!args[0].compare("surfaceparm")) {
							if (!args[1].compare("trans")) {
								shader.addFlag(SHADER_TRANSPARENT);
							}
							else if (!args[1].compare("sky")) {
								shader.addFlag(SHADER_SKY);
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with surfaceparm\n", shader.getName().c_str(), args[1].c_str());
							}
						}
						else if (!args[0].compare("skyparms")) {

						}
						else {
							ILogger::log("Q3Shader::Syntax error in %s: The keyword %s does not exist.\n", shader.getName().c_str(), args[0].c_str());
						}
					}
					else { // Shader pass command
						if (!args[0].compare("animmap")) {
							shaderPass.addFlag(SHADER_ANIMMAP);
							Texture	 frame;

							std::vector<std::string>::iterator j = args.begin() + 2;
							while (j != args.end()){
								frame.m_name = *j;
								shaderPass.addFrame(frame);
								++j;
							}
							
							shaderPass.setAnimSpeed(atof(args[1].c_str()));
						}
						else if (!args[0].compare("blendfunc")) {
							shaderPass.addFlag(SHADER_BLENDFUNC);

							if (args.size() == 2) {
								if (!args[1].compare("blend")) {
									shaderPass.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
								}
								else if (!args[1].compare("filter")) {
									shaderPass.setBlending(GL_DST_COLOR, GL_ZERO);
								}
								else if (!args[1].compare("add")) {
									shaderPass.setBlending(GL_ONE, GL_ONE);
								}
								else {
									ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with blendfunc\n", shader.getName().c_str(), args[1].c_str());
								}
							}
							else
							{
                                GLenum blend[2];
                                
								for (int j = 1; j < 3; ++j) {
									if (!args[j].compare("gl_zero"))
										blend[j-1] = GL_ZERO;
									else if (!args[j].compare("gl_one"))
										blend[j-1] = GL_ONE;
									else if (!args[j].compare("gl_dst_color"))
										blend[j-1] = GL_DST_COLOR;
									else if (!args[j].compare("gl_one_minus_src_alpha"))
										blend[j-1] = GL_ONE_MINUS_SRC_ALPHA;
									else if (!args[j].compare("gl_src_alpha"))
										blend[j-1] = GL_SRC_ALPHA;
									else if (!args[j].compare("gl_src_color"))
										blend[j-1] = GL_SRC_COLOR;
									else if (!args[j].compare("gl_one_minus_dst_color"))
										blend[j-1] = GL_ONE_MINUS_DST_COLOR;
									else if (!args[j].compare("gl_one_minus_src_color"))
										blend[j-1] = GL_ONE_MINUS_SRC_COLOR;
									else if (!args[j].compare("gl_dst_alpha"))
										blend[j-1] = GL_DST_ALPHA;
									else if (!args[j].compare("gl_one_minus_dst_alpha"))
										blend[j-1] = GL_ONE_MINUS_DST_ALPHA;
									else {
										ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with blendfunc\n", shader.getName().c_str(), args[j].c_str());
									}
								}
                                
								shaderPass.setBlending(blend[0], blend[1]);
                                
							}
						}
						else if (!args[0].compare("map")) {
							if (!args[1].compare("$lightmap")) {
								shaderPass.addFlag(SHADER_LIGHTMAP);
							}
							else {
                                Texture texture;
                                texture.m_name = args[1];
								shaderPass.addFrame(texture);
							}

						}
						else if (!args[0].compare("rgbgen")) {

						}
						else if (!args[0].compare("tcgen")) {

						}
						else if (!args[0].compare("tcmod")) {

						}
						else if (!args[0].compare("clampmap")) {
                            Texture texture;
                            texture.m_name = args[1];
                            texture.m_flags = TEXTURE_CLAMP;
							shaderPass.addFrame(texture);
						}
						else if (!args[0].compare("alphafunc")) {
							shaderPass.addFlag(SHADER_ALPHAFUNC);

							if (!args[1].compare("gt0")) {
								shaderPass.setAlphaFunc(GL_GREATER, 0.0f);
							}
							else if (!args[1].compare("ge128")) {
								shaderPass.setAlphaFunc(GL_GEQUAL, 0.5f);
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with alphafunc\n", shader.getName().c_str(), args[1].c_str());
							}

						}
						else if (!args[0].compare("depthfunc")) {
							if (!args[1].compare("equal")) {
								shaderPass.setDepthFunc(GL_EQUAL);
							}
							else {
								ILogger::log("Q3Shader::Syntax error in %s: The keyword %s is not valid with depthfunc\n", shader.getName().c_str(), args[1].c_str());
							}
						}
						else if (!args[0].compare("depthwrite")) {
							shaderPass.addFlag(SHADER_DEPTHWRITE);
						}
						else {
							ILogger::log("Q3Shader::Syntax error in %s: The keyword %s does not exist\n", shader.getName().c_str(), args[0].c_str());
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


bool Q3ShaderManager::init() {

    this->loadFromFile("scripts/base.shader");
    this->loadFromFile("scripts/base_button.shader");
    this->loadFromFile("scripts/base_floor.shader");
    this->loadFromFile("scripts/base_light.shader");
    this->loadFromFile("scripts/base_object.shader");
    this->loadFromFile("scripts/base_support.shader");
    this->loadFromFile("scripts/base_trim.shader");
    this->loadFromFile("scripts/base_wall.shader");
    this->loadFromFile("scripts/common.shader");
    this->loadFromFile("scripts/ctf.shader");
    this->loadFromFile("scripts/eerie.shader");
    this->loadFromFile("scripts/gfx.shader");
    this->loadFromFile("scripts/gothic_block.shader");
    this->loadFromFile("scripts/gothic_floor.shader");
    this->loadFromFile("scripts/gothic_light.shader");
    this->loadFromFile("scripts/gothic_trim.shader");
    this->loadFromFile("scripts/gothic_wall.shader");
    this->loadFromFile("scripts/hell.shader");
    this->loadFromFile("scripts/liquid.shader");
    this->loadFromFile("scripts/menu.shader");
    this->loadFromFile("scripts/models.shader");
    this->loadFromFile("scripts/organics.shader");
    this->loadFromFile("scripts/sfx.shader");
    this->loadFromFile("scripts/shrine.shader");
    this->loadFromFile("scripts/skin.shader");
    this->loadFromFile("scripts/sky.shader");

//	std::cout << this->size() << std::endl;
    
    return true;
}
