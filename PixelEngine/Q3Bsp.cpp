//
//  Bsp.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <cstdio>

#include "Texture.h"
#include "Logger.h"
#include "Q3Bsp.h"


Q3Bsp::Q3Bsp() {
}


Q3Bsp::~Q3Bsp() {
}


bool Q3Bsp::load(const char* filename) {
	FILE * file = NULL;

	ILogger::log("Bsp:: Loading %s ...\n", filename);

	file = fopen(filename, "rb");
	if (!file) {
		ILogger::log("Unable to open the bsp file %s.\n", filename);
		return false;
	}

	fread(&m_header, sizeof(Q3BspHeader), 1, file);

	if (m_header.magic[0] != 'I' || m_header.magic[1] != 'B' ||
		m_header.magic[2] != 'S' || m_header.magic[3] != 'P' ||
		m_header.version != Q3BSP_VERSION)
	{
		ILogger::log("%s is not a quake 3 map.", filename);
		fclose(file);
		return false;
	}

	ILogger::log("-> Magic : %s\n-> Version : %d\n", m_header.magic, m_header.version);


	if (!this->_loadVertexes(file)
		 || !this->_loadMeshVerts(file)
		 || !this->_loadFaces(file)
		 || !this->_loadLightMaps(file)
		 || !this->_loadShaders(file)
		)
	{
		ILogger::log("-> Error while loading data from bsp file %s.\n", filename);
		fclose(file);
		return false;
	}

	// Check extensions
	if (glfwExtensionSupported("GL_ARB_multitexture")) {


	}


	fclose(file);
	ILogger::log("done\n");
	return true;
}

bool Q3Bsp::_loadVertexes(FILE * file) {
	if (!file) {
		return false;
	}

	m_toDraw.nVertices = m_header.entries[LUMP_VERTEXES].length / sizeof(Q3BspVertex);

	m_vertexes.reset(new Q3BspVertex[m_toDraw.nVertices]);

	fseek(file, m_header.entries[LUMP_VERTEXES].offset, SEEK_SET);
	fread(m_vertexes.get(), m_header.entries[LUMP_VERTEXES].length, 1, file);

	m_toDraw.vertices.reset(new Vector4f[m_toDraw.nVertices]);

	for (int i = 0; i < m_toDraw.nVertices; ++i) {
		m_vertexes.get()[i].texcoord[0][1] = 1.0f - m_vertexes.get()[i].texcoord[0][1];
	}

	ILogger::log("--> %d vertexes loaded.\n", m_toDraw.nVertices);

	return true;
}

bool Q3Bsp::_loadMeshVerts(FILE * file) {
	if (!file) {
		return false;
	}

	m_toDraw.nMeshes = m_header.entries[LUMP_MESHVERTS].length / sizeof(Q3BspMeshVert);

	m_meshVerts.reset(new Q3BspMeshVert[m_toDraw.nMeshes]);

	fseek(file, m_header.entries[LUMP_MESHVERTS].offset, SEEK_SET);
	fread(m_meshVerts.get(), m_header.entries[LUMP_MESHVERTS].length, 1, file);

	m_toDraw.meshes.reset(new int[m_toDraw.nMeshes]);

	ILogger::log("--> %d mesh vertexes loaded.\n", m_toDraw.nMeshes);

	return true;
}

bool Q3Bsp::_loadFaces(FILE * file) {
	if (!file) {
		return false;
	}

	m_nFaces = m_header.entries[LUMP_FACES].length / sizeof(Q3BspFace);

	m_faces.reset(new Q3BspFace[m_nFaces]);
	fseek(file, m_header.entries[LUMP_FACES].offset, SEEK_SET);
	fread(m_faces.get(), m_header.entries[LUMP_FACES].length, 1, file);

	ILogger::log("--> %d faces loaded.\n", m_nFaces);

	return true;
}


bool Q3Bsp::_loadShaders(FILE * file) {
	std::shared_ptr<TextureManager> textureManager = TextureManager::getInstance();
	std::vector<std::string> files;
	int nShaders = 0;

	if (!file) {
		return false;
	}

	nShaders = m_header.entries[LUMP_SHADERS].length / sizeof(Q3BspShader);
	m_bspShaders.reset(new Q3BspShader[nShaders]);
	m_shaders.reset(new Q3Shader[nShaders]);

	fseek(file, m_header.entries[LUMP_SHADERS].offset, SEEK_SET);
	fread(m_bspShaders.get(), m_header.entries[LUMP_SHADERS].length, 1, file);

	ILogger::log("--> %d Shaders\n", nShaders);

	for (int i = 0; i < nShaders; ++i) {
		std::string name(m_bspShaders.get()[i].name);
		Q3ShaderPass shaderPass;

		m_shaders[i].addShaderPasse(shaderPass);

		ILogger::log("--->  %s\n", name.c_str());

		files.push_back(name);
	}

	if (!textureManager->loadTextures(files, m_textureIds)) {
		return false;
	}

	return true;
}


bool Q3Bsp::_loadLightMaps(FILE * file) {
	int nLightMaps = 0;
	GLfloat white[4] = { 1.f, 1.f, 1.f, 1.0f };

	if (!file) {
		return false;
	}

	nLightMaps = m_header.entries[LUMP_LIGHTMAPS].length / sizeof(Q3BspLightMap);
	m_lightMaps.reset(new Q3BspLightMap[nLightMaps]);

	fseek(file, m_header.entries[LUMP_LIGHTMAPS].offset, SEEK_SET);
	fread(m_lightMaps.get(), m_header.entries[LUMP_LIGHTMAPS].length, 1, file);

	ILogger::log("--> %d LightMaps\n", nLightMaps);

	m_lmIds.reset(new GLuint[nLightMaps]);
	glGenTextures(nLightMaps, &m_lmIds[0]);

	for (int i = 0; i<nLightMaps; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_lmIds[i]);

		gluBuild2DMipmaps(GL_TEXTURE_2D, 
						  GL_RGBA8, 128, 128,
						  GL_RGB, GL_UNSIGNED_BYTE, 
						  m_lightMaps.get()[i].map);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glGenTextures(1, &m_blankTexId);
	glBindTexture(GL_TEXTURE_2D, m_blankTexId);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, 1, 1, GL_RGB, GL_FLOAT, white);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
}


void Q3Bsp::render() {
	int i;
	SceneNodeList::iterator children = m_children.begin();
	SceneNodeList::iterator childrenEnd = m_children.end();

	for (i = 0; i < m_nFaces; ++i) {
		Q3BspFace face = m_faces.get()[i];
		Q3Shader shader = m_shaders.get()[face.shader];
		const std::vector<Q3ShaderPass>&  shaderPasses = shader.getShaderPasses();

		std::vector<Q3ShaderPass>::const_iterator shaderPasse = shaderPasses.begin();
		std::vector<Q3ShaderPass>::const_iterator shaderPassesEnd = shaderPasses.end();

		if (face.type == FACE_MESH || face.type == FACE_POLYGON) {

			glVertexPointer(3, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes.get()[face.vertex].position));
			
			while (shaderPasse != shaderPassesEnd) {
				glBindTexture(GL_TEXTURE_2D, m_textureIds[face.shader]);

				glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes.get()[face.vertex].texcoord));

				glDrawElements(GL_TRIANGLES, face.n_meshverts, GL_UNSIGNED_INT, &(m_meshVerts.get()[face.meshvert]));

				++shaderPasse;
			}
		}
	}

	while (children != childrenEnd) {
		glPushMatrix();
		(*children)->render();
		glPopMatrix();
		++children;
	}



//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);
}


void Q3Bsp::update(GLdouble delta) {
	SceneNodeList::iterator i = m_children.begin();
	SceneNodeList::iterator end = m_children.end();



	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}