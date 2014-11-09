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
    std::string token = "";

	ILogger::log("Q3Shader:: Loading %s ...\n", filename);

	if (!file.is_open()) {
		ILogger::log("Unable to open the shader script %s.\n", filename);
		return false;
	}

	sstream << file.rdbuf();
	std::string buffer(sstream.str());
	std::string::iterator bufferEnd = buffer.end();

	m_readingState = SP_FIND_NAME;
    m_depth = 0;
    
    shader.name = "";
    shader.m_flags = 0;
    shader.m_shaderPasses.clear();


    std::string::iterator i = buffer.begin();
    while (i != bufferEnd) {
		char c = *i;

        // Check for comments
        if (m_readingState != SP_READ_COMMENT && c == '/' && (i + 1) != bufferEnd && *(i + 1) == '/') {
            i+=2;
            m_readingState = SP_READ_COMMENT;
            continue;
        }
        
        // Check for new node
        if(c == '{') {
            ++m_depth;
            m_readingState = SP_FIND_NAME;
            ++i;
        }
        else if(c == '}') {
            if(m_depth > 0) {
                --m_depth;
//                std::cout << m_depth << std::endl;
            }
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
            
            m_readingState = SP_FIND_NAME;
            ++i;
        }
        
        
        // Parse the node
        switch (m_readingState) {
                
            case SP_FIND_NAME:
                if(c == '\n' || c == ' ' || c == '\t' || c == '\r') {
                    i++;
                }
                else {
                    m_readingState = SP_READ_NAME;
                }
                
                break;
                
            case SP_READ_NAME:
                if(c == '\n' || c == ' ' || c == '\t' || c == '\r') {
                    
                    if(m_depth == 0) {
//                        std::cout << token << std::endl;
                        shader.name = token;
                    }
                    else {
                        
                    }
                    
                    token = "";
                    m_readingState = SP_FIND_VALUE;
                }
                else {
                    //                std::cout << c << std::endl;
                    token.push_back(c);
                    ++i;
                }

                
                break;
                
            case SP_FIND_VALUE:
                
                if(c == '\n' || c == '\r'){
                    m_readingState = SP_FIND_NAME;
                    ++i;
                }
                else if(c == ' ' || c == '\t') {
                    ++i;
                }
                else {
                    m_readingState = SP_READ_VALUE;
                }
                
                break;
                
            case SP_READ_VALUE:
                
                if(c == ' ' || c == '\t') {
                    token = "";
                    m_readingState = SP_FIND_VALUE;
                    ++i;
                }
                else if(c == '\n' || c == '\r'){
                    m_readingState = SP_FIND_NAME;
                    ++i;
                }
                else {
                    ++i;
                }
                
//                std::cout << *i << std::endl;
                break;
                
            case SP_READ_COMMENT:
                if(c == '\n')
                    m_readingState = SP_FIND_NAME;
                ++i;
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