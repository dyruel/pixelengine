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



bool Q3ShaderManager::loadFromFile(const char* filename) {
	std::ifstream file(filename, std::ios::in);
	std::stringstream sstream;
    Q3Shader shader;

	ILogger::log("Q3Shader:: Loading %s ...\n", filename);

	if (!file.is_open()) {
		ILogger::log("Unable to open the shader script %s.\n", filename);
		return false;
	}

	sstream << file.rdbuf();
	std::string buffer(sstream.str());
	std::string::iterator bufferEnd = buffer.end();

	m_readingState = SP_FIND_NAME;
    
    shader.name = "";
    shader.m_flags = 0;
    shader.m_shaderPasses.clear();

	for (std::string::iterator i = buffer.begin(); i != bufferEnd; ++i) {
		char c = *i;

        // Check for comments
        if (c == '/' && (i + 1) != bufferEnd && *(i + 1) == '/') {
            ++i;
            m_readingState = SP_READ_COMMENT;
            continue;
        }
        
        switch (m_readingState) {
            case SP_FIND_NAME:
                if(c == '\n' || c == ' ' || c == '\t' || c == '\r') {
                    continue;
                }
                
                m_readingState = SP_READ_NAME;
                
                break;
                
            case SP_READ_NAME:
                
                break;
                
            case SP_FIND_VALUE:
                
                break;
                
            case SP_READ_VALUE:
                
                break;
                
            case SP_READ_COMMENT:
                if(c == '\n')
                    m_readingState = SP_FIND_NAME;
                continue;
                break;
                
            default:
                break;
        }
        

		switch (c) {
        case '\n':
        case ' ':
        case '\t':
        case '\r':
                if(m_readingState == SP_FIND_NAME || m_readingState == SP_FIND_VALUE) {
                    continue;
                }
                
                if(m_readingState == SP_READ_NAME)
                    m_readingState = SP_FIND_VALUE;
			break;
                
        case '{':
                ++m_depth;
                
                break;
        case '}':
                if(m_depth > 0)
                    --m_depth;
                else {
                    ILogger::log("Q3Shader:: Syntax error in %s \n", filename);
                    return false;
                }
                
                if(m_depth == 0) {
                    m_shaders[shader.name] = shader;
                    
                    shader.name = "";
                    shader.m_flags = 0;
                    shader.m_shaderPasses.clear();
                }
                    
                break;
		default:
                if (m_readingState == SP_FIND_NAME)
                    m_readingState = SP_READ_NAME;
                else if (m_readingState == SP_FIND_VALUE)
                    m_readingState = SP_READ_VALUE;
                
                
                if(m_readingState == SP_READ_NAME) {
                    if(shader.name.empty()) {
                        
                    }
                }
			break;
		}

	}

	file.close();
	return true;
}