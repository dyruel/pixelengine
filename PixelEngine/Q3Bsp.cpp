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

int Q3Bsp::bbox_index[8][3] =
{
	{ 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
	{ 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
};


Q3Bsp::Q3Bsp()
	: m_cameraCluster(0), m_blankTexId(0) {
}


Q3Bsp::~Q3Bsp() {
	if (m_visData->bits) {
		delete[] m_visData->bits;
	}
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
		 || !this->_loadBspTree(file)
		 || !this->_loadVisData(file)
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

	int n = m_header.entries[LUMP_VERTEXES].length / sizeof(Q3BspVertex);

	m_vertexes.reset(new Q3BspVertex[n]);
	fseek(file, m_header.entries[LUMP_VERTEXES].offset, SEEK_SET);
	fread(m_vertexes.get(), m_header.entries[LUMP_VERTEXES].length, 1, file);

	for (int i = 0; i < n; ++i) {
		m_vertexes[i].texcoord[0][1] = 1.0f - m_vertexes[i].texcoord[0][1];
//		m_vertexes.get()[i].texcoord[1][0] = 1.0f - m_vertexes.get()[i].texcoord[1][0];
	}

	ILogger::log("--> %d vertexes loaded.\n", n);

	return true;
}

bool Q3Bsp::_loadMeshVerts(FILE * file) {
	if (!file) {
		return false;
	}

	int n = m_header.entries[LUMP_MESHVERTS].length / sizeof(Q3BspMeshVert);

	m_meshVerts.reset(new Q3BspMeshVert[n]);

	fseek(file, m_header.entries[LUMP_MESHVERTS].offset, SEEK_SET);
	fread(m_meshVerts.get(), m_header.entries[LUMP_MESHVERTS].length, 1, file);

//	m_toDraw.meshes.reset(new int[m_toDraw.nMeshes]);

	ILogger::log("--> %d mesh vertexes loaded.\n", n);

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

		ILogger::log("--->  %s\n", name.c_str());

		files.push_back(name);
	}

	if (!textureManager->loadTextures(files, m_textureIds)) {
		return false;
	}


	for (int i = 0; i < nShaders; ++i) {
		Q3ShaderDefault shaderDefault(m_textureIds[i]);

		m_shaders[i] = shaderDefault;
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


bool Q3Bsp::_loadBspTree(FILE * file) {
	if (!file) {
		return false;
	}

	int n = m_header.entries[LUMP_NODES].length / sizeof(Q3BspNode);

	m_nodes.reset(new Q3BspNode[n]);
	fseek(file, m_header.entries[LUMP_NODES].offset, SEEK_SET);
	fread(m_nodes.get(), m_header.entries[LUMP_NODES].length, 1, file);

	ILogger::log("--> %d nodes loaded.\n", n);

	n = m_header.entries[LUMP_PLANES].length / sizeof(Q3BspPlane);

	m_planes.reset(new Q3BspPlane[n]);
	fseek(file, m_header.entries[LUMP_PLANES].offset, SEEK_SET);
	fread(m_planes.get(), m_header.entries[LUMP_PLANES].length, 1, file);

	ILogger::log("--> %d planes loaded.\n", n);

	m_nLeafs = m_header.entries[LUMP_LEAFS].length / sizeof(Q3BspLeaf);

	m_leafs.reset(new Q3BspLeaf[m_nLeafs]);
	fseek(file, m_header.entries[LUMP_LEAFS].offset, SEEK_SET);
	fread(m_leafs.get(), m_header.entries[LUMP_LEAFS].length, 1, file);

	ILogger::log("--> %d leafs loaded.\n", m_nLeafs);


	n = m_header.entries[LUMP_LEAFFACES].length / sizeof(Q3BspLeafFace);

	m_leafFaces.reset(new Q3BspLeafFace[n]);
	fseek(file, m_header.entries[LUMP_LEAFFACES].offset, SEEK_SET);
	fread(m_leafFaces.get(), m_header.entries[LUMP_LEAFFACES].length, 1, file);

	ILogger::log("--> %d leaf faces loaded.\n", n);

	n = m_header.entries[LUMP_LEAFBRUSHES].length / sizeof(Q3BspLeafBrush);

	m_leafBrushes.reset(new Q3BspLeafBrush[n]);
	fseek(file, m_header.entries[LUMP_LEAFBRUSHES].offset, SEEK_SET);
	fread(m_leafBrushes.get(), m_header.entries[LUMP_LEAFBRUSHES].length, 1, file);

	ILogger::log("--> %d leaf brushes loaded.\n", n);

	return true;
}


bool Q3Bsp::_loadVisData(FILE * file) {
	if (!file) {
		return false;
	}

	m_visData.reset(new Q3BspVisData);

	fseek(file, m_header.entries[LUMP_VISDATA].offset, SEEK_SET);
	fread(m_visData.get(), 2, sizeof(int), file);

	int size = m_visData->n_clusters * m_visData->sz_clusters;
	m_visData->bits = new unsigned char[size];
	fread(m_visData->bits, 1, size, file);

	ILogger::log("--> vis data loaded.\n", m_nFaces);

	return true;
}

int Q3Bsp::getLeafIndex(const Vector3d& v) const {
	int index = 0;

	while (index >= 0) {
		const Q3BspNode&  node = m_nodes[index];
		const Q3BspPlane& plane = m_planes[node.plane];
		const Vector3f planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);

		const double distance = planeNormal.dotProduct(v) - plane.dist;

		if (distance >= 0) {
			index = node.children[0];
		}
		else {
			index = node.children[1];
		}
	}

	return -(index + 1);
}


bool Q3Bsp::checkClusterVisibility(int from, int to) const {
	unsigned char bits = 0;

	if (!m_visData->bits || from < 0) {
		return true;
	}
	
	bits = m_visData->bits[(from * m_visData->sz_clusters) + (to >> 3)];

	return (bits & (1 << (to & 7))) != 0;
}



void Q3Bsp::_selectFaces(int index) {
	
	if (index < 0) { // Leaf
		int i = -(index + 1);
		Vector4f corners[8];

		// PVS test
		if (!checkClusterVisibility(m_cameraCluster, m_leafs[i].cluster)) {
			return;
		}

		// Clipping test
		unsigned char nIn = 0;
		
		for (int j = 0; j < 8; ++j)
		{
			char isIn = 0;

			corners[j].x = m_leafs[j].bbox[bbox_index[j][0]];
			corners[j].y = m_leafs[j].bbox[bbox_index[j][1]];
			corners[j].z = m_leafs[j].bbox[bbox_index[j][2]];
			corners[j].w = 1.0f;

			corners[j] = m_clipMatrix * corners[j];

			if (corners[j].x > -corners[j].w && corners[j].x < corners[j].w &&
				corners[j].y > -corners[j].w && corners[j].y < corners[j].w &&
				corners[j].z > -corners[j].w && corners[j].z < corners[j].w)
				isIn = 1;

			nIn += isIn;
		}
		
		std::cout << nIn << std::endl;
		if (!nIn)
			return;

		for (int j = 0; j < m_leafs[i].n_leaffaces; ++j) {
			const int f = j + m_leafs[i].leafface;
			m_facesToRender.insert(m_leafFaces[f].face);
		}

	} else { // Node
		const Q3BspNode&  node = m_nodes[index];
		const Q3BspPlane& plane = m_planes[node.plane];
		const Vector3f planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);

//		const double distance = planeNormal.dotProduct(v) - plane.dist;

//		if (distance >= 0) {
		_selectFaces(node.children[0]);
//		}
//		else {
		_selectFaces(node.children[1]);
//		}
	}
}

void Q3Bsp::render() {
//	int i;

	// Step 1 : Select the faces to be rendered
	_selectFaces(0);

	std::cout << m_facesToRender.size() << std::endl;

	// Step 2 : Render previously selected faces
	std::set<int>::iterator faceToRender = m_facesToRender.begin();
	std::set<int>::iterator faceToRenderEnd = m_facesToRender.end();

	while (faceToRender != faceToRenderEnd) {
		Q3BspFace face = m_faces[*faceToRender];
		Q3Shader shader = m_shaders[face.shader];
		const std::vector<Q3ShaderPass>&  shaderPasses = shader.getShaderPasses();

		std::vector<Q3ShaderPass>::const_iterator shaderPasse = shaderPasses.begin();
		std::vector<Q3ShaderPass>::const_iterator shaderPassesEnd = shaderPasses.end();

		if (face.type == FACE_MESH || face.type == FACE_POLYGON) {

			glVertexPointer(3, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes.get()[face.vertex].position));

			while (shaderPasse != shaderPassesEnd) {
				//				glBindTexture(GL_TEXTURE_2D, m_textureIds[face.shader]);

				if ((*shaderPasse).flags & SHADER_LIGHTMAP) {
					glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes[face.vertex].texcoord[1]));
					glBindTexture(GL_TEXTURE_2D, face.lm_index);
				}
				else {
					glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes[face.vertex].texcoord[0]));
					glBindTexture(GL_TEXTURE_2D, (*shaderPasse).texId);
				}

				if ((*shaderPasse).flags & SHADER_BLEND) {
					glEnable(GL_BLEND);
					glBlendFunc((*shaderPasse).blendSrc, (*shaderPasse).blendDst);
				}
				else {
					glDisable(GL_BLEND);
				}

				glDrawElements(GL_TRIANGLES, face.n_meshverts, GL_UNSIGNED_INT, &(m_meshVerts.get()[face.meshvert]));

				++shaderPasse;
			}
		}
		++faceToRender;
	}

	// Step 3
	SceneNodeList::iterator children = m_children.begin();
	SceneNodeList::iterator childrenEnd = m_children.end();

	while (children != childrenEnd) {
		glPushMatrix();
		(*children)->render();
		glPopMatrix();
		++children;
	}

}


void Q3Bsp::update(GLdouble delta) {
	SceneNodeList::iterator i = m_children.begin();
	SceneNodeList::iterator end = m_children.end();

	m_facesToRender.clear();
	m_cameraCluster = m_leafs[getLeafIndex(m_attachedCamera->getPosition())].cluster;

	// Get clip matrix
	float m[16], p[16], r[16];
	glGetFloatv(GL_PROJECTION_MATRIX, p);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	glPushMatrix();
	glLoadMatrixf(m);
	glMultMatrixf(p);
	glGetFloatv(GL_MODELVIEW_MATRIX, r);
	glPopMatrix();

	m_clipMatrix = r;

	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}