//
//  Bsp.cpp
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include <cstdio>

#include "Q3Level.h"
#include "unique.h"
#include "Video.h"
#include "Texture.h"
#include "Logger.h"



bool Q3Level::load(const char* filename) {
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

bool Q3Level::_loadVertices(FILE * file, const BspLumpEntry& verticesLump, const BspLumpEntry& indexesLump) {
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
    
    m_map.m_verticesPool.vertices.reserve(n);
    m_map.m_verticesPool.vertices.resize(n);
    
    for (int i = 0; i < n; ++i) {
        m_map.m_verticesPool.vertices[i].position.x = bspVertices[i].position[0];
        m_map.m_verticesPool.vertices[i].position.y = bspVertices[i].position[1];
        m_map.m_verticesPool.vertices[i].position.z = bspVertices[i].position[2];
        
        m_map.m_verticesPool.vertices[i].texcoord.s = bspVertices[i].texcoord[0][0];
        m_map.m_verticesPool.vertices[i].texcoord.t = bspVertices[i].texcoord[0][1];
        
        m_map.m_verticesPool.vertices[i].lmcoord.s = bspVertices[i].texcoord[1][0];
        m_map.m_verticesPool.vertices[i].lmcoord.t = bspVertices[i].texcoord[1][1];
        
        m_map.m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_map.m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_map.m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];

        m_map.m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_map.m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_map.m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];
        
        m_map.m_verticesPool.vertices[i].color.r = bspVertices[i].color[0];
        m_map.m_verticesPool.vertices[i].color.g = bspVertices[i].color[1];
        m_map.m_verticesPool.vertices[i].color.b = bspVertices[i].color[2];
        m_map.m_verticesPool.vertices[i].color.a = bspVertices[i].color[3];
    }
    

    ILogger::log("--> %d vertexes loaded.\n", n);
    
    
    n = indexesLump.length / sizeof(int);
    std::unique_ptr<int[]> bspIndexes = std::make_unique<int[]>(n);
    fseek(file, indexesLump.offset, SEEK_SET);
    fread(bspIndexes.get(), indexesLump.length, 1, file);
    
    m_map.m_verticesPool.indexes.reserve(n);
    m_map.m_verticesPool.indexes.resize(n);
    
    for (int i = 0; i < n; ++i) {
        m_map.m_verticesPool.indexes[i] = bspIndexes[i];
    }
    
    ILogger::log("--> %d indexes loaded.\n", n);
    
    
	return true;
}



bool Q3Level::_loadFaces(FILE * file, const BspLumpEntry& facesLump, const BspLumpEntry& shadersLump, const BspLumpEntry& lightmapsLump) {
	if (!file) {
		return false;
	}
    
    std::shared_ptr<Q3ShaderManager> shaderManager = Q3ShaderManager::getInstance();
    std::shared_ptr<TextureManager> textureManager = TextureManager::getInstance();
    std::vector<Q3Shader> shaders;
    std::string infoString("");
    int n = 0, nPlanar = 0, nPatch = 0, nTriSoup = 0, nFlare = 0,nBad = 0;

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
        
//        if(i == 2054) {
//            std::cout << "qsd " << bspFaces[i].type <<  std::endl;
//        }
        
        switch (bspFaces[i].type) {
                
            case Q3Face::FACE_PLANAR:
            {
                
                std::shared_ptr<Q3Face> face = std::make_shared<Q3FacePlanar>(m_map.m_verticesPool);
                face->type = Q3Face::FACE_PLANAR;
                
                if (shaderManager->exists(bspShaders[bspFaces[i].shader].name)) {
                    face->m_shader = shaderManager->getShader(bspShaders[bspFaces[i].shader].name);
                    
                    std::vector<Q3ShaderPass>::iterator shaderPass = face->m_shader.m_shaderPasses.begin();
                    std::vector<Q3ShaderPass>::iterator shaderPassesEnd = face->m_shader.m_shaderPasses.end();
                    
                    while (shaderPass != shaderPassesEnd) {
                        TextureManager::getInstance()->getTexture(shaderPass->m_Texture);
                        shaderPass->_setTexCoordPointer(&m_map.m_verticesPool.vertices[bspFaces[i].vertex].texcoord.s, sizeof(Q3Vertex));
                        ++shaderPass;
                    }
                }
                else {
                    Texture tex[2];
                    
                    float * texCoordPointer[2] = {
                        &m_map.m_verticesPool.vertices[bspFaces[i].vertex].texcoord.s,
                        &m_map.m_verticesPool.vertices[bspFaces[i].vertex].lmcoord.s
                    };
                    
                    GLsizei stride[2] = {
                        sizeof(Q3Vertex),
                        sizeof(Q3Vertex)
                    };
                    
                    tex[0].m_name   = bspShaders[bspFaces[i].shader].name;
                    tex[1].m_name   = "lightmap";
                    tex[1].m_texId  = lmIds[bspFaces[i].lm_index];
                    
                    TextureManager::getInstance()->getTexture(tex[0]);
                    
                    Q3ShaderDefault shaderDefault(tex, texCoordPointer, stride);
                    
                    face->m_shader = shaderDefault;
                    infoString = "(No shader script found, default loaded)";
                }

                m_map.m_faces.push_back(face);
                ++nPlanar;
                
                ILogger::log("---> Face %d, type planar, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case Q3Face::FACE_PATCH:
            {
                std::shared_ptr<Q3Face> face = std::make_shared<Q3FacePatch>(m_map.m_verticesPool);
                face->type = Q3Face::FACE_PATCH;
                
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
                m_map.m_faces.push_back(face);
                ++nPatch;
                ILogger::log("---> Face %d, type patch, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case Q3Face::FACE_TRIANGLE_SOUP:
            {
                std::shared_ptr<Q3Face> face = std::make_shared<Q3FacePatch>(m_map.m_verticesPool);
                face->type = Q3Face::FACE_PATCH;
                
                m_map.m_faces.push_back(face);
                ++nTriSoup;
                ILogger::log("---> Face %d, type triangle soup, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
                
            case Q3Face::FACE_FLARE:
            {
                std::shared_ptr<Q3Face> face = std::make_shared<Q3FaceFlare>(m_map.m_verticesPool);
                face->type = Q3Face::FACE_FLARE;
                
                m_map.m_faces.push_back(face);
                ++nFlare;
                ILogger::log("---> Face %d, type flare, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
                
            case Q3Face::FACE_BAD:
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




bool Q3Level::_loadBspTree(FILE * file,
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
        
        int firstLeafFace;
        int numLeafFaces;
        
        int firstLeafBrush;
        int numLeafBrushes;
    } ;

    int nInternalNodes = 0, nPlanes = 0, n = 0;

	nInternalNodes = nodesLump.length / sizeof(BspNode);
    std::unique_ptr<BspNode[]> bspNodes = std::make_unique<BspNode[]>(nInternalNodes);
	fseek(file, nodesLump.offset, SEEK_SET);
	fread(bspNodes.get(), nodesLump.length, 1, file);
    
	ILogger::log("--> %d nodes loaded.\n", nInternalNodes);
    
    nPlanes = planesLump.length / sizeof(BspPlane);
    std::unique_ptr<BspPlane[]> bspPlanes = std::make_unique<BspPlane[]>(nPlanes);
    fseek(file, planesLump.offset, SEEK_SET);
    fread(bspPlanes.get(), planesLump.length, 1, file);

	ILogger::log("--> %d planes loaded.\n", nPlanes);
    
    
    for (int i = 0; i < nInternalNodes; ++i) {
        std::shared_ptr<Q3Map::InternalNode> node = std::make_shared<Q3Map::InternalNode>();
        
        node->plane.normal.x = bspPlanes[bspNodes[i].plane].normal[0];
        node->plane.normal.y = bspPlanes[bspNodes[i].plane].normal[1];
        node->plane.normal.z = bspPlanes[bspNodes[i].plane].normal[2];
        
        node->plane.dist = bspPlanes[bspNodes[i].plane].dist;
        
        for (int j = 0; j < 6; ++j) {
            node->bbox[j] = bspNodes[i].bbox[j];
        }
        
        for (int j = 0; j < 2; ++j) {
            node->children[j] = bspNodes[i].children[j];
        }
        
        m_map.m_internalNodes.push_back(node);
    }

    n = leafLump.length / sizeof(BspLeaf);
    std::unique_ptr<BspLeaf[]> bspLeaf = std::make_unique<BspLeaf[]>(n);
    fseek(file, leafLump.offset, SEEK_SET);
    fread(bspLeaf.get(), leafLump.length, 1, file);
    
    for (int i = 0; i < n; ++i) {
        std::shared_ptr<Q3Map::LeafNode> leaf = std::make_shared<Q3Map::LeafNode>();
        
        leaf->cluster = bspLeaf[i].cluster;
        leaf->area    = bspLeaf[i].area;
        
        for (int j = 0; j < 6; ++j) {
            leaf->bbox[j] = bspLeaf[i].bbox[j];
        }
        
        leaf->firstLeafFace = bspLeaf[i].firstLeafFace;
        leaf->numLeafFaces = bspLeaf[i].numLeafFaces;
        
        leaf->firstLeafBrush = bspLeaf[i].firstLeafBrush;
        leaf->numLeafBrushes = bspLeaf[i].numLeafBrushes;
        
        m_map.m_leafNodes.push_back(leaf);
    }

	ILogger::log("--> %d leafs loaded.\n", n);

    n = leafFaceLump.length / sizeof(int);
    std::unique_ptr<int[]> bspLeafFace = std::make_unique<int[]>(n);
    fseek(file, leafFaceLump.offset, SEEK_SET);
    fread(bspLeafFace.get(), leafFaceLump.length, 1, file);
    
    for (int i = 0; i < n; ++i) {
        m_map.m_leafFaceIndexes.push_back(bspLeafFace[i]);
    }

	ILogger::log("--> %d leaf face indexes loaded.\n", n);
    

    n = leafBrushLump.length / sizeof(int);
    std::unique_ptr<int[]> bspLeafBrush = std::make_unique<int[]>(n);
    fseek(file, leafBrushLump.offset, SEEK_SET);
    fread(bspLeafBrush.get(), leafBrushLump.length, 1, file);

    for (int i = 0; i < n; ++i) {
        m_map.m_leafBrushIndexes.push_back(bspLeafBrush[i]);
    }
    
	ILogger::log("--> %d leaf brush indexes loaded.\n", n);
    

    fseek(file, visDataLump.offset, SEEK_SET);
    fread(&m_map.m_visData, 2, sizeof(int), file);
    
    int size = m_map.m_visData.numClusters * m_map.m_visData.sizeCluster;
    m_map.m_visData.bits = std::make_unique<unsigned char[]>(size);
    fread(m_map.m_visData.bits.get(), 1, size, file);
    
    ILogger::log("--> vis data loaded (%d x %d).\n", m_map.m_visData.numClusters, m_map.m_visData.sizeCluster);
    

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





inline void Q3Level::render() {
//    std::cout << "qze" << std::endl;
    m_map.render();

}


inline void Q3Level::update(GLdouble delta) {
	SceneNodeList::iterator i = m_children.begin();
	SceneNodeList::iterator end = m_children.end();
    
    m_map.update(delta);
    
	while (i != end) {
		(*i)->update(delta);
		++i;
	}
}


#if 0
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