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



bool Q3Shader::loadFromFile(const char* filename) {
	std::ifstream file(filename, std::ios::in);
	std::stringstream sstream;

	ILogger::log("Q3Shader:: Loading %s ...\n", filename);

	if (!file.is_open()) {
		ILogger::log("Unable to open the shader script %s.\n", filename);
		return false;
	}

	sstream << file.rdbuf();
	std::string buffer(sstream.str());
	std::string::iterator bufferEnd = buffer.end();

	m_readingState = SHADER_LOAD_NAME;

	for (std::string::iterator i = buffer.begin(); i != bufferEnd; ++i) {
		char c = *i;

		// Skip white spaces
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			continue;

		// Skip comments
		if (c == '/' && (i + 1) != bufferEnd && *(i + 1) == '/') {
			for (; (*i) != '\n' && i != bufferEnd; ++i);
			continue;
		}

		switch (m_readingState) {
		case SHADER_LOAD_NAME:
			break;
		case SHADER_LOAD_START:
			break;
		case SHADER_LOAD_PASS:
			break;
		}

	}

	file.close();
	return true;
}