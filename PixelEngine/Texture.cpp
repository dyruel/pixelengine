//
//  Texture.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


#include "Texture.h"


bool TextureManager::init() {
	ilInit();
	iluInit();
	/*
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	*/
	/*
	ilEnable(IL_TYPE_SET);
	ilTypeFunc(IL_UNSIGNED_BYTE);

	ilEnable(IL_FORMAT_SET);
	ilFormatFunc(IL_RGB);
	*/
	return true;
}

bool TextureManager::deinit() {

	return true;
}

# if 0
bool TextureManager::_loadTextures(const std::vector<std::string>& files, std::unique_ptr<GLuint[]>& ids) {

	GLsizei n = (GLsizei) files.size();
	std::vector<std::string>::const_iterator i = files.begin();
	std::vector<std::string>::const_iterator end = files.end();
    GLuint k = 0;
//	GLuint * ids = NULL;
	ILuint * imgIds = NULL;
	
	ids.reset(new GLuint[n]);
	glGenTextures(n, &ids[0]);

	imgIds = new GLuint[n];
	ilGenImages(n, imgIds);

    while (i != end) {
		std::string tga(*i);
		std::string jpg(*i);
		bool isLoaded = false;

		tga.append(".tga");
		jpg.append(".jpg");
//		Texture texture;

		ilBindImage(k);

		if (!ilLoadImage(tga.c_str())) {
			if(ilLoadImage(jpg.c_str()))
				isLoaded = true;
		} else
			isLoaded = true;

		if (isLoaded) {
			//texture.name = (*i).c_str();
			//texture.texId = ids[k];
			//texture.width = ilGetInteger(IL_IMAGE_WIDTH);
			//texture.height = ilGetInteger(IL_IMAGE_HEIGHT);
			//texture.type = ilGetInteger(IL_IMAGE_FORMAT);
			//texture.bpp = ilGetInteger(IL_IMAGE_BPP);
			//texture.imageData = ilGetData();

			glBindTexture(GL_TEXTURE_2D, ids[k]);
            
			gluBuild2DMipmaps(GL_TEXTURE_2D, 
							  ilGetInteger(IL_IMAGE_BPP), 
							  ilGetInteger(IL_IMAGE_WIDTH),
							  ilGetInteger(IL_IMAGE_HEIGHT),
							  ilGetInteger(IL_IMAGE_FORMAT),
							  GL_UNSIGNED_BYTE, 
							  ilGetData());
            
			/*glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
				ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
				ilGetData());*/
			//glTexImage2D(GL_TEXTURE_2D, 0, texture.bpp,
			//	texture.width,
			//	texture.height,
			//	0, texture.type, GL_UNSIGNED_BYTE,
			//	texture.imageData);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//			list.push_back(texture);
		}

		++i;
		++k;
    }


	ilDeleteImages(n, imgIds);
	//if (ids)
	//	delete[] ids;

	if (imgIds)
		delete[] imgIds;
    
    return true;
}
#endif

GLuint TextureManager::_loadTextureFromFile(const std::string& filename, int flags) {
	ILuint imgId = 0;
	GLuint id = 0;
	bool isLoaded = false;

	ilGenImages(1, &imgId);
	ilBindImage(imgId);

	std::string rawName = filename.substr(0, filename.find_last_of("."));

	std::string tga(rawName);
	tga.append(".tga");

	std::string jpg(rawName);
	jpg.append(".jpg");
	
	if (!ilLoad(IL_TGA, filename.c_str()) && !ilLoad(IL_JPG, filename.c_str())) {
//		ILenum err = ilGetError();
//		iluErrorString(err);
//		ILogger::log("Error  %s\n", iluErrorString(err));

		if (!ilLoad(IL_TGA, tga.c_str())) {
			if (ilLoad(IL_JPG, jpg.c_str()))
				isLoaded = true;
		}
		else {
			isLoaded = true;
		}
	}
	else {
		
		isLoaded = true;
	}


	if (isLoaded) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
//		std::cout << id << std::endl;

		if (flags & TEXTURE_CLAMP) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		if (flags & TEXTURE_NOMIPMAP) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 
						ilGetInteger(IL_IMAGE_BPP),
						ilGetInteger(IL_IMAGE_WIDTH),
						ilGetInteger(IL_IMAGE_HEIGHT),
						0, 
						ilGetInteger(IL_IMAGE_FORMAT), 
						GL_UNSIGNED_BYTE,
						ilGetData());
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			gluBuild2DMipmaps(GL_TEXTURE_2D,
				ilGetInteger(IL_IMAGE_BPP),
				ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT),
				ilGetInteger(IL_IMAGE_FORMAT),
				GL_UNSIGNED_BYTE,
				ilGetData());
		}

		m_textures[filename] = id;
	}

	ilBindImage(0);
	ilDeleteImage(imgId);

//	ILogger::log("no hit for %s %u\n", filename, m_textures[filename]);
//	std::cout << filename.c_str() << " " << id << std::endl;

	return id;
}
 


