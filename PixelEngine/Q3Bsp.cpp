//
//  Bsp.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <cstdio>

#include "Video.h"
#include "Q3Bsp.h"
#include "Texture.h"
#include "Logger.h"



enum {
	CLIP_X_LEFT		= 1 << 0,
	CLIP_X_RIGHT	= 1 << 1,
	CLIP_Y_LEFT		= 1 << 2,
	CLIP_Y_RIGHT	= 1 << 3,
	CLIP_Z_LEFT		= 1 << 4,
	CLIP_Z_RIGHT	= 1 << 5,
};


int Q3Bsp::bbox_index[8][3] =
{
	{ 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
	{ 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
};


Q3Bsp::Q3Bsp()
	: m_cameraCluster(0), m_Delta(0.), m_currentShaderPass(0) {
}


Q3Bsp::~Q3Bsp() {
	if (m_visData && m_visData->bits) {
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
		 || !this->_loadEntities(file)
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

//	for (int i = 0; i < n; ++i) {
//		m_vertexes[i].texcoord[0][1] = 1.0f - m_vertexes[i].texcoord[0][1];
//		m_vertexes.get()[i].texcoord[1][0] = 1.0f - m_vertexes.get()[i].texcoord[1][0];
//	}

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

	// Load and tesselate patches
	for (int i = 0; i < m_nFaces; ++i) {
		if (m_faces[i].type != FACE_PATCH)
			continue;

		int maxPatchesX = (m_faces[i].size[0] - 1) >> 1;
		int maxPatchesY = (m_faces[i].size[1] - 1) >> 1;

		m_patches[i].m_numPatches = maxPatchesX * maxPatchesY;
		m_patches[i].m_bezierPatches.reset(new Q3BezierPatch[m_patches[i].m_numPatches]);
		
		for (int y = 0; y < maxPatchesY; ++y)
		{
			for (int x = 0; x < maxPatchesX; ++x)
			{
				for (int a = 0; a < 3; ++a)
				{
					for (int b = 0; b < 3; ++b)
					{
						m_patches[i].m_bezierPatches[y*maxPatchesX + x].m_anchors[a * 3 + b].x = m_vertexes[m_faces[i].vertex + (2 * y*m_faces[i].size[0] + 2 * x) + (a * m_faces[i].size[0] + b)].position[0];
						m_patches[i].m_bezierPatches[y*maxPatchesX + x].m_anchors[a * 3 + b].y = m_vertexes[m_faces[i].vertex + (2 * y*m_faces[i].size[0] + 2 * x) + (a * m_faces[i].size[0] + b)].position[1];
						m_patches[i].m_bezierPatches[y*maxPatchesX + x].m_anchors[a * 3 + b].z = m_vertexes[m_faces[i].vertex + (2 * y*m_faces[i].size[0] + 2 * x) + (a * m_faces[i].size[0] + b)].position[2];
					}
				}

				m_patches[i].m_bezierPatches[y*maxPatchesX + x].tesselate();
			}
		}

	}

	return true;
}


bool Q3Bsp::_loadShaders(FILE * file) {
	std::shared_ptr<TextureManager> textureManager = TextureManager::getInstance();
//	std::vector<std::string> files;
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

	m_shaderManager = Q3ShaderManager::getInstance();

	m_shaderManager->loadFromFile("scripts/base.shader");
	m_shaderManager->loadFromFile("scripts/base_button.shader");
	m_shaderManager->loadFromFile("scripts/base_floor.shader");
	m_shaderManager->loadFromFile("scripts/base_light.shader");
	m_shaderManager->loadFromFile("scripts/base_object.shader");
	m_shaderManager->loadFromFile("scripts/base_support.shader");
	m_shaderManager->loadFromFile("scripts/base_trim.shader");
	m_shaderManager->loadFromFile("scripts/base_wall.shader");
	m_shaderManager->loadFromFile("scripts/common.shader");
	m_shaderManager->loadFromFile("scripts/ctf.shader");
	m_shaderManager->loadFromFile("scripts/eerie.shader");
	m_shaderManager->loadFromFile("scripts/gfx.shader");
	m_shaderManager->loadFromFile("scripts/gothic_block.shader");
	m_shaderManager->loadFromFile("scripts/gothic_floor.shader");
	m_shaderManager->loadFromFile("scripts/gothic_light.shader");
	m_shaderManager->loadFromFile("scripts/gothic_trim.shader");
	m_shaderManager->loadFromFile("scripts/gothic_wall.shader");
	m_shaderManager->loadFromFile("scripts/hell.shader");
	m_shaderManager->loadFromFile("scripts/liquid.shader");
	m_shaderManager->loadFromFile("scripts/menu.shader");
	m_shaderManager->loadFromFile("scripts/models.shader");
	m_shaderManager->loadFromFile("scripts/organics.shader");
	m_shaderManager->loadFromFile("scripts/sfx.shader");
	m_shaderManager->loadFromFile("scripts/shrine.shader");
	m_shaderManager->loadFromFile("scripts/skin.shader");
	m_shaderManager->loadFromFile("scripts/sky.shader");

	for (int i = 0; i < nShaders; ++i) {
		if (m_shaderManager->exists(m_bspShaders[i].name)) {
			m_shaders[i] = m_shaderManager->getShader(m_bspShaders[i].name);
			ILogger::log("--->  %s\n", m_bspShaders[i].name);
		}
		else {
			Q3ShaderDefault shaderDefault(textureManager->getTexture(m_bspShaders[i].name));
			m_shaders[i] = shaderDefault;
			ILogger::log("--->  %s (No shader file, default loaded)\n", m_bspShaders[i].name);
		}
	}
	

	return true;
}


bool Q3Bsp::_loadLightMaps(FILE * file) {
	int nLightMaps = 0;

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, m_lightMaps[i].map);
	}

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

	ILogger::log("--> vis data loaded.\n");

	return true;
}


bool Q3Bsp::_loadEntities(FILE * file) {
	if (!file) {
		return false;
	}


	char * entities = new char[m_header.entries[LUMP_ENTITIES].length];
	fseek(file, m_header.entries[LUMP_ENTITIES].offset, SEEK_SET);
	fread(entities, 1, m_header.entries[LUMP_ENTITIES].length, file);

	m_entities = entities;

	if (entities)
		delete[] entities;


//	std::cout << m_entities << std::endl;

	ILogger::log("--> entities loaded.\n");

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

		// PVS test
		if (!checkClusterVisibility(m_cameraCluster, m_leafs[i].cluster)) {
			return;
		}

		// Clipping test
		unsigned int and_clip = ~0;
		
		for (int j = 0; j < 8; ++j)
		{
			Vector4f v, cv;
			unsigned int flags = 0;

			v.x = (GLfloat) m_leafs[i].bbox[bbox_index[j][0]];
			v.y = (GLfloat) m_leafs[i].bbox[bbox_index[j][1]];
			v.z = (GLfloat) m_leafs[i].bbox[bbox_index[j][2]];
			v.w = 1.0f;

			cv = m_clipMatrix * v;

			if (cv.x < -cv.w)
				flags |= CLIP_X_LEFT;

			if (cv.x > cv.w)
				flags |= CLIP_X_RIGHT;

			if (cv.y > cv.w)
				flags |= CLIP_Y_LEFT;

			if (cv.y > cv.w)
				flags |= CLIP_Y_RIGHT;

			if (cv.z > cv.w)
				flags |= CLIP_Z_LEFT;

			if (cv.z > cv.w)
				flags |= CLIP_Z_RIGHT;

			and_clip &= flags;
		}

		if (and_clip)
			return;

		for (int j = 0; j < m_leafs[i].n_leaffaces; ++j) {
			const int f = j + m_leafs[i].leafface;
			m_facesToRender.insert(m_leafFaces[f].face);
		}

	}
	 else { // Node
		 const Q3BspNode&  node = m_nodes[index];
		 const Q3BspPlane& plane = m_planes[node.plane];
		 const Vector3f planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);

		 // Clipping test
		 unsigned int and_clip = ~0;

		 for (int j = 0; j < 8; ++j)
		 {
			 Vector4f v, cv;
			 unsigned int flags = 0;

			 v.x = (GLfloat) node.bbox[bbox_index[j][0]];
			 v.y = (GLfloat) node.bbox[bbox_index[j][1]];
			 v.z = (GLfloat) node.bbox[bbox_index[j][2]];
			 v.w = 1.0f;

			 cv = m_clipMatrix * v;

			 if (cv.x < -cv.w)
				 flags |= CLIP_X_LEFT;

			 if (cv.x > cv.w)
				 flags |= CLIP_X_RIGHT;

			 if (cv.y > cv.w)
				 flags |= CLIP_Y_LEFT;

			 if (cv.y > cv.w)
				 flags |= CLIP_Y_RIGHT;

			 if (cv.z > cv.w)
				 flags |= CLIP_Z_LEFT;

			 if (cv.z > cv.w)
				 flags |= CLIP_Z_RIGHT;

			 and_clip &= flags;
		 }

		 if (and_clip)
			 return;

		 _selectFaces(node.children[0]);
		 _selectFaces(node.children[1]);

	 }
}



void Q3Bsp::_beginShaderPass() {
	if (!m_currentShaderPass)
		return;

}


void Q3Bsp::_endShaderPass() {
	if (!m_currentShaderPass)
		return;


}

void Q3Bsp::render() {
	//	std::cout << m_facesToRender.size() << std::endl;

	// Step 2 : Render previously selected faces
	std::set<int>::iterator faceToRender = m_facesToRender.begin();
	std::set<int>::iterator faceToRenderEnd = m_facesToRender.end();

	while (faceToRender != faceToRenderEnd) {
		const Q3BspFace& face = m_faces[*faceToRender];
		Q3Shader& shader = m_shaders[face.shader];

		if (face.type == FACE_BAD || face.type == FACE_BILLBOARD) {
			
		}
		else if (face.type == FACE_PATCH) {
			m_patches[*faceToRender].render();
		}
		else {
			std::vector<Q3ShaderPass>& shaderPasses = shader.getShaderPasses();

			GLboolean ogl_env_cullface = glIsEnabled(GL_CULL_FACE);

			if (shader.getFlags() & SHADER_NOCULL) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glEnable(GL_CULL_FACE);
			}

			std::vector<Q3ShaderPass>::iterator shaderPasse = shaderPasses.begin();
			std::vector<Q3ShaderPass>::iterator shaderPassesEnd = shaderPasses.end();

			//		if (face.type == FACE_MESH || face.type == FACE_POLYGON) {

			glVertexPointer(3, GL_FLOAT, sizeof(Q3BspVertex), m_vertexes[face.vertex].position);

			while (shaderPasse != shaderPassesEnd) {

				GLboolean ogl_env_blend = glIsEnabled(GL_BLEND);
				GLboolean ogl_env_alpha = glIsEnabled(GL_ALPHA_TEST);
				GLboolean ogl_env_depthmask;
				glGetBooleanv(GL_DEPTH_WRITEMASK, &ogl_env_depthmask);
				GLboolean ogl_env_colorarray = glIsEnabled(GL_COLOR_ARRAY);


				if ((*shaderPasse).m_flags & SHADER_LIGHTMAP) {
					glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes[face.vertex].texcoord[1]));
					glBindTexture(GL_TEXTURE_2D, m_lmIds[face.lm_index]);
					//			std::cout << face.lm_index << std::endl;
					//				m_lmIds

				}
				else if ((*shaderPasse).m_flags & SHADER_ANIMMAP) {

					int frame = 0;

					(*shaderPasse).m_frame += m_Delta * (*shaderPasse).m_animSpeed;

					frame = ((int)(*shaderPasse).m_frame) % (*shaderPasse).m_animNumframes;

					//					std::cout << (*shaderPasse).m_animFrames[frame] << std::endl;
					glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes[face.vertex].texcoord[0]));
					glBindTexture(GL_TEXTURE_2D, (*shaderPasse).m_animFrames[frame]);
				}
				else {
					glTexCoordPointer(2, GL_FLOAT, sizeof(Q3BspVertex), &(m_vertexes[face.vertex].texcoord[0]));
					glBindTexture(GL_TEXTURE_2D, (*shaderPasse).m_texId);
				}

				if ((*shaderPasse).m_flags & SHADER_BLENDFUNC) {
					glEnable(GL_BLEND);
					glBlendFunc((*shaderPasse).m_blendSrc, (*shaderPasse).m_blendDst);
				}
				else {
					glDisable(GL_BLEND);
				}

				if ((*shaderPasse).m_flags & SHADER_ALPHAFUNC) {
					glEnable(GL_ALPHA_TEST);
					glAlphaFunc((*shaderPasse).m_alphaFunc, (*shaderPasse).m_alphaFuncRef);
				}
				else {
					glDisable(GL_ALPHA_TEST);
				}


				glDepthFunc((*shaderPasse).m_depthFunc);
				if ((*shaderPasse).m_flags & SHADER_DEPTHWRITE) {
					glDepthMask(GL_TRUE);
					//std::cout << shader.getName() << std::endl;
					//return;
					//std::cout << "aze" << std::endl;
				}
				else {
					//std::cout << "cvvc " << shader.getName() << std::endl;
					glDepthMask(GL_FALSE);
				}

				glDrawElements(GL_TRIANGLES, face.n_meshverts, GL_UNSIGNED_INT, &(m_meshVerts.get()[face.meshvert]));

				/*
				if (ogl_env_blend) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				}
				else if ((*shaderPasse).m_flags & SHADER_BLENDFUNC) {
				glDisable(GL_BLEND);
				}

				if (ogl_env_alpha) {
				glEnable(GL_ALPHA_TEST);
				}
				else if ((*shaderPasse).m_flags & SHADER_ALPHAFUNC) {
				glDisable(GL_ALPHA_TEST);
				}
				*/
				//				glDepthMask(GL_TRUE);
				/*
				if (ogl_env_depthmask) {
				glDepthMask(GL_TRUE);
				}
				else {
				glDepthMask(GL_FALSE);
				}
				*/
				/*
				if (ogl_env_colorarray) {
				glEnableClientState(GL_COLOR_ARRAY);
				}
				else {
				glDisableClientState(GL_COLOR_ARRAY);
				}
				*/

				++shaderPasse;
			}
			//}

			if (ogl_env_cullface) {
				glEnable(GL_CULL_FACE);
			}
			else {
				glDisable(GL_CULL_FACE);
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

	m_cameraCluster = m_leafs[getLeafIndex(m_attachedCamera->getPosition())].cluster;
	m_Delta = delta;

//	std::cout << delta << std::endl;

	// Get clip matrix
	float m[16], p[16], r[16];
	glGetFloatv(GL_PROJECTION_MATRIX, p);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	glPushMatrix();
	glLoadMatrixf(p);
	glMultMatrixf(m);
	glGetFloatv(GL_MODELVIEW_MATRIX, r);
	glPopMatrix();

	m_clipMatrix = r;

//	if (glfwGetKey(Video::getInstance()->getWindow(), GLFW_KEY_A) == GLFW_PRESS) {
		m_facesToRender.clear();
		_selectFaces(0);
//	}

	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}


bool Q3BezierPatch::tesselate() {
	float px = 0.f, py = 0.f;
	Vector3f temp[3];
	Vector3f aux;

	if (!m_lod) {
		m_lod = 8;
	}

	m_vertices.reset(new Q3BspVertex[(m_lod + 1)*(m_lod + 1)]);

	for (int a = 0; a <= m_lod; ++a)
	{
		px = (float)a / m_lod;

		aux = m_anchors[0] * ((1.f - px)*(1.f - px)) +
					m_anchors[3] * ((1.f - px)*px * 2) +
					m_anchors[6] * (px*px);

		m_vertices[a].position[0] = aux.x;
		m_vertices[a].position[1] = aux.y;
		m_vertices[a].position[2] = aux.z;
	}


	for (int a = 1; a <= m_lod; ++a)
	{
		py = (float)a / m_lod;

		temp[0] =	m_anchors[0] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[1] * ((1.0f - py)*py * 2) +
					m_anchors[2] * (py*py);

		temp[1] =	m_anchors[3] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[4] * ((1.0f - py)*py * 2) +
					m_anchors[5] * (py*py);

		temp[2] =	m_anchors[6] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[7] * ((1.0f - py)*py * 2) +
					m_anchors[8] * (py*py);

		for (int b = 0; b <= m_lod; ++b)
		{
			px = (float)b / m_lod;

			aux = temp[0] * ((1.0f - px)*(1.0f - px)) +
				temp[1] * ((1.0f - px)*px * 2) +
				temp[2] * (px*px);

			m_vertices[a*(m_lod + 1) + b].position[0] = aux.x;
			m_vertices[a*(m_lod + 1) + b].position[1] = aux.y;
			m_vertices[a*(m_lod + 1) + b].position[2] = aux.z;
		}
	}


	m_indices.reset(new GLuint[m_lod*(m_lod + 1) * 2]);

	for (int a = 0; a<m_lod; ++a)
	{
		for (int b = 0; b <= m_lod; ++b)
		{
			m_indices[(a*(m_lod + 1) + b) * 2 + 1] = a*(m_lod + 1) + b;
			m_indices[(a*(m_lod + 1) + b) * 2] = (a + 1)*(m_lod + 1) + b;
		}
	}
	

	return true;
}

void Q3BezierPatch::render() {

	// FIXME : Use Vertex Array
	glBegin(GL_TRIANGLE_STRIP);
	for (int a = 0; a < m_lod; ++a)
	{
		for (int b = 0; b < 2 * (m_lod + 1); ++b)
		{
			glVertex3f(m_vertices[m_indices[a * 2 * (m_lod + 1) + b]].position[0], m_vertices[m_indices[a * 2 * (m_lod + 1) + b]].position[1], m_vertices[m_indices[a * 2 * (m_lod + 1) + b]].position[2]);
		}
	}
	glEnd();

	// glVertexPointer(3, GL_FLOAT, sizeof(Q3BspVertex), m_vertices[0].position);
	//for (int a = 0; a < m_lod; ++a) {
		//glDrawElements(GL_TRIANGLE_STRIP, 2 * (m_lod + 1), GL_UNSIGNED_INT, &m_indices[a * 2 * (m_lod + 1)]);
	//}
}


void Q3Patch::render() {
	for (int i = 0; i < m_numPatches; ++i) {
		m_bezierPatches[i].render();
	}
}