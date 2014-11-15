//
//  Bsp.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <cstdio>

#include "unique.h"
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
	: m_cameraCluster(0), m_Delta(0.) {
}


Q3Bsp::~Q3Bsp() {
    /*
	if (m_visData && m_visData->bits) {
		delete[] m_visData->bits;
	}*/
}


bool Q3Bsp::load(const char* filename) {
	FILE * file = NULL;

	ILogger::log("Bsp:: Loading %s ...\n", filename);

	file = fopen(filename, "rb");
	if (!file) {
		ILogger::log("Unable to open the bsp file %s.\n", filename);
		return false;
	}
    
    BspHeader bspHeader;

	fread(&bspHeader, sizeof(BspHeader), 1, file);

	if (bspHeader.magic[0] != 'I' || bspHeader.magic[1] != 'B' ||
		bspHeader.magic[2] != 'S' || bspHeader.magic[3] != 'P' ||
		bspHeader.version != Q3BSP_VERSION)
	{
		ILogger::log("%s is not a quake 3 map.", filename);
		fclose(file);
		return false;
	}

	ILogger::log("-> Magic : %s\n-> Version : %d\n", bspHeader.magic, bspHeader.version);


	if (!this->_loadVertices(file,
                             bspHeader.entries[LUMP_VERTICES],
                             bspHeader.entries[LUMP_INDEXES])
        
        ||
        
        !this->_loadFaces(file,
                          bspHeader.entries[LUMP_FACES],
                          bspHeader.entries[LUMP_SHADERS],
                          bspHeader.entries[LUMP_LIGHTMAPS])
        
        ||
        
        !this->_loadBspTree(file,
                            bspHeader.entries[LUMP_NODES],
                            bspHeader.entries[LUMP_LEAFS],
                            bspHeader.entries[LUMP_PLANES],
                            bspHeader.entries[LUMP_LEAFFACES],
                            bspHeader.entries[LUMP_LEAFBRUSHES],
                            bspHeader.entries[LUMP_VISDATA])
		 //|| !this->_loadEntities(file, m_header.entries[LUMP_VERTICES].offset, m_header.entries[LUMP_VERTICES].length)
		)
	{
		ILogger::log("-> Error while loading data from bsp file %s.\n", filename);
		fclose(file);
		return false;
	}


	fclose(file);
	ILogger::log("done\n");
	return true;
}

bool Q3Bsp::_loadVertices(FILE * file, const BspLumpEntry& verticesLump, const BspLumpEntry& indexesLump) {
    int n = 0;
    
	if (!file) {
		return false;
	}
    
    struct BspVertex {
        float position[3];
        float texcoord[2][2];
        float normal[3];
        unsigned char color[4];
    };

	n = verticesLump.length / sizeof(BspVertex);
    std::unique_ptr<BspVertex[]> bspVertices = std::make_unique<BspVertex[]>(n);
    
	fseek(file, verticesLump.offset, SEEK_SET);
	fread(bspVertices.get(), verticesLump.length, 1, file);
    
    m_verticesPool.vertices.reserve(n);
    m_verticesPool.vertices.resize(n);
    
    for (int i = 0; i < n; ++i) {
        m_verticesPool.vertices[i].position.x = bspVertices[i].position[0];
        m_verticesPool.vertices[i].position.y = bspVertices[i].position[1];
        m_verticesPool.vertices[i].position.z = bspVertices[i].position[2];
        
        m_verticesPool.vertices[i].texcoord.s = bspVertices[i].texcoord[0][0];
        m_verticesPool.vertices[i].texcoord.t = bspVertices[i].texcoord[0][1];
        
        m_verticesPool.vertices[i].lmcoord.s = bspVertices[i].texcoord[1][0];
        m_verticesPool.vertices[i].lmcoord.t = bspVertices[i].texcoord[1][1];
        
        m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];

        m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];
        
        m_verticesPool.vertices[i].color.r = bspVertices[i].color[0];
        m_verticesPool.vertices[i].color.g = bspVertices[i].color[1];
        m_verticesPool.vertices[i].color.b = bspVertices[i].color[2];
        m_verticesPool.vertices[i].color.a = bspVertices[i].color[3];
    }
    

    ILogger::log("--> %d vertexes loaded.\n", n);
    
    
    n = indexesLump.length / sizeof(int);
    std::unique_ptr<int[]> bspIndexes = std::make_unique<int[]>(n);
    fseek(file, indexesLump.offset, SEEK_SET);
    fread(bspIndexes.get(), indexesLump.length, 1, file);
    
    m_verticesPool.indexes.reserve(n);
    m_verticesPool.indexes.resize(n);
    
    for (int i = 0; i < n; ++i) {
        m_verticesPool.indexes[i] = bspIndexes[i];
    }
    
    ILogger::log("--> %d indexes loaded.\n", n);
    
    
	return true;
}



bool Q3Bsp::_loadFaces(FILE * file, const BspLumpEntry& facesLump, const BspLumpEntry& shadersLump, const BspLumpEntry& lightmapsLump) {
	if (!file) {
		return false;
	}
    
    std::shared_ptr<Q3ShaderManager> shaderManager = Q3ShaderManager::getInstance();
    std::shared_ptr<TextureManager> textureManager = TextureManager::getInstance();
    std::vector<Q3Shader> shaders;
    std::string infoString("");
    int n = 0, nPlanar = 0, nPatch = 0, nTriSoup = 0, nBad = 0;

    struct BspFace {
        int shader;
        int effect;
        int type;
        
        int vertex;
        int n_vertexes;
        
        int meshvert;
        int n_meshverts;
        
        int lm_index;
        int lm_start[2];
        int lm_size[2];
        float lm_origin[3];
        float lm_vecs[2][3];
        
        float normal[3];
        
        int size[2];
    };
    
    struct BspShader {
        char name[64];
        int flags;
        int contents;
    };
    
    struct Q3BspLightMap {
        unsigned char  map[128][128][3];
    };
    
    
    // Load and create lightmaps
    n = lightmapsLump.length / sizeof(Q3BspLightMap);
    std::unique_ptr<Q3BspLightMap[]> bspLightmap = std::make_unique<Q3BspLightMap[]>(n);
    fseek(file, lightmapsLump.offset, SEEK_SET);
    fread(bspLightmap.get(), lightmapsLump.length, 1, file);
    
    std::unique_ptr<GLuint[]> lmIds = std::make_unique<GLuint[]>(n);
    
    glGenTextures(n, lmIds.get());
    
    for (int i = 0; i < n; ++i) {
        glBindTexture(GL_TEXTURE_2D, lmIds[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, bspLightmap[i].map);
    }
    
    ILogger::log("--> %d LightMaps\n", n);
    
    // Load shaders and faces from the file
    n = shadersLump.length / sizeof(BspShader);
    std::unique_ptr<BspShader[]> bspShaders = std::make_unique<BspShader[]>(n);
    fseek(file, shadersLump.offset, SEEK_SET);
    fread(bspShaders.get(), shadersLump.length, 1, file);
    
    n = facesLump.length / sizeof(BspFace);
    std::unique_ptr<BspFace[]> bspFaces = std::make_unique<BspFace[]>(n);
    fseek(file, facesLump.offset, SEEK_SET);
    fread(bspFaces.get(), facesLump.length, 1, file);
    
    // Fill faces informations
    for (int i = 0; i < n; ++i) {
        
        switch (bspFaces[i].type) {
                
            case FACE_PLANAR:
            {
                std::shared_ptr<Q3Face> face = std::make_shared<Q3FacePlanar>(m_verticesPool);
                face->type = FACE_PLANAR;
                
                if (shaderManager->exists(bspShaders[bspFaces[i].shader].name)) {
                    face->m_shader = shaderManager->getShader(bspShaders[bspFaces[i].shader].name);
                }
                else {
                    //std::cout << bspShader[i].name << std::endl;
                    Q3ShaderDefault shaderDefault(textureManager->getTexture(bspShaders[bspFaces[i].shader].name), lmIds[bspFaces[i].lm_index]);
                    face->m_shader = shaderDefault;
                    infoString = "(No shader script found, default loaded)";
                }
                
                m_faces.push_back(face);
                ++nPlanar;
                
                ILogger::log("---> Face %d, type planar, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case FACE_PATCH:
            {
                /*
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
                 m_patches[i].m_bezierPatches[y*maxPatchesX + x].m_anchors[a * 3 + b] = m_vertexes[m_faces[i].vertex + (2 * y*m_faces[i].size[0] + 2 * x) + (a * m_faces[i].size[0] + b)];
                 }
                 }
                 
                 m_patches[i].m_bezierPatches[y*maxPatchesX + x].tesselate();
                 }
                 }
                 
                 }
                 */
                ++nPatch;
                ILogger::log("---> Face %d, type patch, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case FACE_TRIANGLE_SOUP:
            {
                ++nTriSoup;
                ILogger::log("---> Face %d, type triangle soup, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case FACE_BAD:
            default:
            {
                ++nBad;
                ILogger::log("---> Face %d, type bad\n", i);
            }
            break;
        }

    }
    
    ILogger::log("--> %d faces loaded (%d Planars, %d Patches, %d Triangle Soup, %d Bad).\n",
                 nPlanar+nPatch+nTriSoup+nBad, nPlanar, nPatch, nTriSoup, nBad);

	return true;
}




bool Q3Bsp::_loadBspTree(FILE * file,
                         const BspLumpEntry& nodesLump,
                         const BspLumpEntry& leafLump,
                         const BspLumpEntry& planesLump,
                         const BspLumpEntry& leafFaceLump,
                         const BspLumpEntry& leafBrushLump,
                         const BspLumpEntry& visDataLump)
{
	if (!file) {
		return false;
	}
    
    struct BspPlane {
        float normal[3];
        float dist;
    } ;
    
    struct BspNode {
        int plane;
        int children[2];
        int bbox[6];
    } ;
    
    struct BspLeaf {
        int cluster;
        int area;
        int bbox[6];
        int leafface;
        int n_leaffaces;
        int leafbrush;
        int n_leafbrushes;
    } ;

    int n = 0;

	n = nodesLump.length / sizeof(BspNode);
    std::unique_ptr<BspNode[]> bspNodes = std::make_unique<BspNode[]>(n);
	fseek(file, nodesLump.offset, SEEK_SET);
	fread(bspNodes.get(), nodesLump.length, 1, file);

	ILogger::log("--> %d nodes loaded.\n", n);
    
    n = planesLump.length / sizeof(BspPlane);
    std::unique_ptr<BspPlane[]> bspPlanes = std::make_unique<BspPlane[]>(n);
    fseek(file, planesLump.offset, SEEK_SET);
    fread(bspPlanes.get(), planesLump.length, 1, file);

	ILogger::log("--> %d planes loaded.\n", n);

    n = leafLump.length / sizeof(BspLeaf);
    std::unique_ptr<BspLeaf[]> bspLeaf = std::make_unique<BspLeaf[]>(n);
    fseek(file, leafLump.offset, SEEK_SET);
    fread(bspLeaf.get(), leafLump.length, 1, file);

	ILogger::log("--> %d leafs loaded.\n", n);

    n = leafFaceLump.length / sizeof(int);
    std::unique_ptr<int[]> bspLeafFace = std::make_unique<int[]>(n);
    fseek(file, leafFaceLump.offset, SEEK_SET);
    fread(bspLeafFace.get(), leafFaceLump.length, 1, file);

	ILogger::log("--> %d leaf faces loaded.\n", n);

    n = leafBrushLump.length / sizeof(int);
    std::unique_ptr<int[]> bspBrushFace = std::make_unique<int[]>(n);
    fseek(file, leafBrushLump.offset, SEEK_SET);
    fread(bspBrushFace.get(), leafBrushLump.length, 1, file);

	ILogger::log("--> %d leaf brushes loaded.\n", n);
    

    int visData[2];
    fseek(file, visDataLump.offset, SEEK_SET);
    fread(visData, 2, sizeof(int), file);
    
    int size = visData[0] * visData[1];
    std::unique_ptr<unsigned char[]> bits = std::make_unique<unsigned char[]>(size);
    fread(bits.get(), 1, size, file);
    
    ILogger::log("--> vis data loaded.\n");

	return true;
}



/*
bool Q3Bsp::_loadEntities(FILE * file, int offset, int length) {
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
*/

#if 0
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

	std::string noshader("noshader");

	while (faceToRender != faceToRenderEnd) {
		const Q3BspFace& face = m_faces[*faceToRender];
		Q3Shader& shader = m_shaders[face.shader];

//		if (!noshader.compare(m_bspShaders[face.shader].name))
//			std::cout << "dd" << std::endl;
        
		if (face.type == FACE_BAD || face.type == FACE_BILLBOARD) {
			
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
//				GLboolean ogl_env_colorarray = glIsEnabled(GL_COLOR_ARRAY);

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

                if (face.type == FACE_PATCH) {
                    m_patches[*faceToRender].render();
                }
                else
                    glDrawElements(GL_TRIANGLES, face.n_meshverts, GL_UNSIGNED_INT, &(m_meshVerts.get()[face.meshvert]));

				
				if (ogl_env_blend) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				}
				else {
                    glDisable(GL_BLEND);
				}

				if (ogl_env_alpha) {
                    glEnable(GL_ALPHA_TEST);
				}
				else {
                    glDisable(GL_ALPHA_TEST);
				}
				
				//				glDepthMask(GL_TRUE);
				
				if (ogl_env_depthmask) {
                    glDepthMask(GL_TRUE);
				}
				else {
                    glDepthMask(GL_FALSE);
				}
				
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
    
	m_cameraCluster = m_leafs[getLeafIndex(SceneManager::getInstance()->getCamera()->getPosition())].cluster;
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
	float temp[3][3];
	float tex[3][2];
	//Vector3f aux;

	if (!m_lod) {
		m_lod = 8;
	}

	m_vertices.reset(new Q3BspVertex[(m_lod + 1)*(m_lod + 1)]);

	for (int a = 0; a <= m_lod; ++a)
	{
		px = (float)a / m_lod;

		m_vertices[a].position[0] = m_anchors[0].position[0] * ((1.f - px)*(1.f - px)) +
									m_anchors[3].position[0] * ((1.f - px)*px * 2) +
									m_anchors[6].position[0] * (px*px);
		m_vertices[a].position[1] = m_anchors[0].position[1] * ((1.f - px)*(1.f - px)) +
									m_anchors[3].position[1] * ((1.f - px)*px * 2) +
									m_anchors[6].position[1] * (px*px);
		m_vertices[a].position[2] = m_anchors[0].position[2] * ((1.f - px)*(1.f - px)) +
									m_anchors[3].position[2] * ((1.f - px)*px * 2) +
									m_anchors[6].position[2] * (px*px);
    
		m_vertices[a].texcoord[0][0] = m_anchors[0].texcoord[0][0] * ((1.f - px)*(1.f - px)) +
										m_anchors[3].texcoord[0][0] * ((1.f - px)*px * 2) +
										m_anchors[6].texcoord[0][0] * (px*px);
		m_vertices[a].texcoord[0][1] = m_anchors[0].texcoord[0][1] * ((1.f - px)*(1.f - px)) +
										m_anchors[3].texcoord[0][1] * ((1.f - px)*px * 2) +
										m_anchors[6].texcoord[0][1] * (px*px);
	}


	for (int a = 1; a <= m_lod; ++a)
	{
		py = (float)a / m_lod;

		temp[0][0] = m_anchors[0].position[0] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[1].position[0] * ((1.0f - py)*py * 2) +
					 m_anchors[2].position[0] * (py*py);
		temp[0][1] = m_anchors[0].position[1] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[1].position[1] * ((1.0f - py)*py * 2) +
				     m_anchors[2].position[1] * (py*py);
		temp[0][2] = m_anchors[0].position[2] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[1].position[2] * ((1.0f - py)*py * 2) +
					 m_anchors[2].position[2] * (py*py);

		tex[0][0] = m_anchors[0].texcoord[0][0] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[1].texcoord[0][0] * ((1.0f - py)*py * 2) +
					m_anchors[2].texcoord[0][0] * (py*py);
		tex[0][1] = m_anchors[0].texcoord[0][1] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[1].texcoord[0][1] * ((1.0f - py)*py * 2) +
					m_anchors[2].texcoord[0][1] * (py*py);


		temp[1][0] = m_anchors[3].position[0] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[4].position[0] * ((1.0f - py)*py * 2) +
					 m_anchors[5].position[0] * (py*py);
		temp[1][1] = m_anchors[3].position[1] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[4].position[1] * ((1.0f - py)*py * 2) +
				     m_anchors[5].position[1] * (py*py);
		temp[1][2] = m_anchors[3].position[2] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[4].position[2] * ((1.0f - py)*py * 2) +
					 m_anchors[5].position[2] * (py*py);

		tex[1][0] = m_anchors[3].texcoord[0][0] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[4].texcoord[0][0] * ((1.0f - py)*py * 2) +
					m_anchors[5].texcoord[0][0] * (py*py);
		tex[1][1] = m_anchors[3].texcoord[0][1] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[4].texcoord[0][1] * ((1.0f - py)*py * 2) +
					m_anchors[5].texcoord[0][1] * (py*py);

		
		temp[2][0] = m_anchors[6].position[0] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[7].position[0] * ((1.0f - py)*py * 2) +
					 m_anchors[8].position[0] * (py*py);
		temp[2][1] = m_anchors[6].position[1] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[7].position[1] * ((1.0f - py)*py * 2) +
				     m_anchors[8].position[1] * (py*py);
		temp[2][2] = m_anchors[6].position[2] * ((1.0f - py)*(1.0f - py)) +
					 m_anchors[7].position[2] * ((1.0f - py)*py * 2) +
					 m_anchors[8].position[2] * (py*py);

		tex[2][0] = m_anchors[6].texcoord[0][0] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[7].texcoord[0][0] * ((1.0f - py)*py * 2) +
					m_anchors[8].texcoord[0][0] * (py*py);
		tex[2][1] = m_anchors[6].texcoord[0][1] * ((1.0f - py)*(1.0f - py)) +
					m_anchors[7].texcoord[0][1] * ((1.0f - py)*py * 2) +
					m_anchors[8].texcoord[0][1] * (py*py);

		for (int b = 0; b <= m_lod; ++b)
		{
			px = (float)b / m_lod;

			m_vertices[a*(m_lod + 1) + b].position[0] = temp[0][0] * ((1.0f - px)*(1.0f - px)) +
														temp[1][0] * ((1.0f - px)*px * 2) +
														temp[2][0] *(px*px);
			m_vertices[a*(m_lod + 1) + b].position[1] = temp[0][1] * ((1.0f - px)*(1.0f - px)) +
														temp[1][1] * ((1.0f - px)*px * 2) +
														temp[2][1] *(px*px);
			m_vertices[a*(m_lod + 1) + b].position[2] = temp[0][2] * ((1.0f - px)*(1.0f - px)) +
														temp[1][2] * ((1.0f - px)*px * 2) +
														temp[2][2] *(px*px);

			m_vertices[a*(m_lod + 1) + b].texcoord[0][0] = tex[0][0] * ((1.0f - px)*(1.0f - px)) +
														   tex[1][0] * ((1.0f - px)*px * 2) +
														   tex[2][0] * (px*px);
			m_vertices[a*(m_lod + 1) + b].texcoord[0][1] = tex[0][1] * ((1.0f - px)*(1.0f - px)) +
														   tex[1][1] * ((1.0f - px)*px * 2) +
														   tex[2][1] * (px*px);

            //m_vertices[a*(m_lod + 1) + b].texcoord[0][0] = 1.f - py;
            //m_vertices[a*(m_lod + 1) + b].texcoord[0][1] = 1.f - px;
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
            glTexCoord2f(m_vertices[m_indices[a * 2 * (m_lod + 1) + b]].texcoord[0][0], m_vertices[m_indices[a * 2 * (m_lod + 1) + b]].texcoord[0][1]);
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

#endif