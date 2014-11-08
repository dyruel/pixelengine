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

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	ilEnable(IL_TYPE_SET);
	ilTypeFunc(IL_UNSIGNED_BYTE);

	ilEnable(IL_FORMAT_SET);
	ilFormatFunc(IL_RGB);

	return true;
}

bool TextureManager::deinit() {

	return true;
}

bool TextureManager::loadTextures(const std::vector<std::string>& files, std::unique_ptr<GLuint[]>& ids) {

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


GLuint TextureManager::loadTextureFromFile(std::string filename) {
	ILuint ImgId = 0;
	GLuint id;
	
	ilGenImages(1, &ImgId);
	ilBindImage(ImgId);

	if (!ilLoadImage(filename.c_str())) {
		ilDeleteImage(ImgId);
		return 0;
	}

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT),
				0, GL_RGBA, GL_UNSIGNED_BYTE,
				ilGetData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ilBindImage(0);
	ilDeleteImage(ImgId);

	return id;
}

//void TextureManager::bind(GLuint id) {
//    glBindTexture(GL_TEXTURE_2D, id);
//}

