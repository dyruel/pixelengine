//
//  File.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__File__
#define __PixelEngine__File__

#include <string>
#include <physfs.h>

#include "Logger.h"
#include "Singleton.h"
#include "Types.h"

class FileSystem : public Singleton<FileSystem> {
	friend class Singleton<FileSystem>;

public:

	enum {
		FILE_READ = 0,
		FILE_WRITE
	};

	~FileSystem() {
		if (isOpened()) {
			close();
		}

		if (!PHYSFS_deinit()) {
			ILogger::log("FileSystem :: Error while deinitializing PHYSFS.\n");
		}
	};

	bool init() { 
		int r = PHYSFS_init(0);

		if (!r) {
			ILogger::log("FileSystem :: Error while initializing PHYSFS.\nPHYSFS said: %s\n",  PHYSFS_getLastError());
			return false;
		}
			
		r = PHYSFS_setSaneConfig("pixelengine","pixelengine","pk3",0,1);

		return r != 0; 
	}

	int mount(const char* dir, const char* mountPoint = "/", int appendToPath = 1) { 
		int r = PHYSFS_mount(dir, mountPoint, appendToPath);

		if (!r) {
			ILogger::log("FileSystem :: Error while mounting %s.\nPHYSFS said: %s\n", dir, PHYSFS_getLastError());
		}

		return r;
	}

	int open(const char *filename, int mode = FILE_READ) {
		int r = 0;
		m_filename = filename;

		switch (mode)
		{
		case FILE_READ:
			m_file = PHYSFS_openRead(filename);
			break;
		case FILE_WRITE:
			m_file = PHYSFS_openWrite(filename);
			break;
		}

		if (m_file) {
			return 1;
		}

		ILogger::log("FileSystem :: Error while opening %s.\nPHYSFS said: %s\n", filename, PHYSFS_getLastError());

		return  0;
	};

	int close() { 
		int r = PHYSFS_close(m_file); 
		m_filename = "";

		if (r) 
			m_file = NULL; 
		else
			ILogger::log("FileSystem :: The file %s was not closed properly.\nPHYSFS said: %s\n", m_filename.c_str(), PHYSFS_getLastError());

		return r;
	};

	bool isOpened() {
		return m_file != NULL;
	}

	int seek(PHYSFS_uint64 pos) { 

//		std::cout << "Seek: " << PHYSFS_fileLength(m_file) << " " << pos << std::endl;

		int r = PHYSFS_seek(m_file, pos);

		if (!r) {
			ILogger::log("FileSystem :: Error while seeking in %s.\nPHYSFS said: %s\n", m_filename.c_str(), PHYSFS_getLastError());
		}

//		ILogger::log("Seek ok\n");

		return r;
	};

	PHYSFS_sint64 read(void* buffer, u32 objSize, u32 objCount) {

		std::cout << PHYSFS_tell(m_file) + objSize*objCount << " " << PHYSFS_fileLength(m_file) << std::endl;

		if (PHYSFS_tell(m_file) + objSize*objCount > PHYSFS_fileLength(m_file)) {
			ILogger::log("FileSystem :: Reading beyond the file.\n");
			return 0;
		}

		int r = PHYSFS_read(m_file, buffer, objSize, objCount);

		if (!r) {
			ILogger::log("FileSystem :: Error while seeking in %s.\nPHYSFS said: %s\n", m_filename.c_str(), PHYSFS_getLastError());
		}

		return r;
	}

	//PHYSFS_uint64 tell() { return PHYSFS_tell(m_file); };

	//int eof() { return PHYSFS_eof(m_file); };

	private:
		std::string m_filename;
		PHYSFS_File * m_file;

		FileSystem()
			:m_file(NULL), m_filename(""){};
		FileSystem(FileSystem&);
		void operator =(FileSystem&);

};

#endif /* defined(__PixelEngine__File__) */