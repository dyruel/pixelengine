//
//  Logger.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Logger.h"


ILogger* ILogger::m_logger = NULL;

 void ILogger::log(const char* fmt, ...) {
	char buf[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	ILogger::m_logger->write(buf);
}
