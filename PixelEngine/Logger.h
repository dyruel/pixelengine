//
//  Logger.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__Logger__
#define __PixelEngine__Logger__

#include <fstream> 
#include <iostream>

#include <cstdio>
#include <cstdarg>

class ILogger {
private:
	virtual void write(const std::string& msg) = 0;
	static ILogger* m_logger;

public:
	ILogger() {};
	virtual ~ILogger() {};

	static void setLogger(ILogger* logger) { ILogger::m_logger = logger; };

	static void log(const char* fmt, ...);

};


class FileLogger : public ILogger  {

private:

	std::ofstream m_file;

	void write(const std::string& msg) { m_file << msg.c_str(); };


public:
	FileLogger(const std::string& filename = "log.txt") : m_file(filename) {};
	~FileLogger() { /*m_file.close();*/ };
};



#endif /* defined(__PixelEngine__Logger__) */