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
//                    token.clear();
                    m_readingState = SP_READ_NAME;
                }
                
                break;
                
                
            case SP_READ_NAME:
                
                if(c == ' ' || c == '\t') {
                    args.push_back(token);
                    token.clear();
                    m_readingState = SP_FIND_VALUE;
                }
                else {
                    token.push_back(c);
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
                    token.push_back(c);
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
                    m_shaders[shader.name] = shader;
                    shader.clear();
                }
                else {
                    // end of a shader pass
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
                    if(m_depth == 0) {
                        shader.name = args[0];
                    }
                    
                    std::cout << "# ";
                    std::vector<std::string>::iterator m = args.begin();
                    std::vector<std::string>::iterator end = args.end();
                    
                    while (m != end) {
                        std::cout <<  (*m) << " ";
                        ++m;
                    }
                    
                    std::cout << std::endl;

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