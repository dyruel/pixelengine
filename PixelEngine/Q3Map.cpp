//
//  Q3Map.cpp
//  pixelengine
//
//  Created by Morgan on 15/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Q3Map.h"
#include "unique.h"

int Q3Map::bbox_index[8][3] =
{
    { 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
    { 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
};


void Q3FacePlanar::render() {
    m_shader.start();
    
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPasse = m_shader.begin();
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = m_shader.end();
    
    
    glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), m_verticesPool.vertices.data() + m_firstVertex);
 
/*
    glBegin(GL_TRIANGLES);
    for (int b = m_firstIndex; b < m_firstIndex + m_numIndexes; ++b)
    {

        glVertex3f(m_verticesPool.vertices[m_verticesPool.indexes[b] + m_firstVertex].position.x,
                   m_verticesPool.vertices[m_verticesPool.indexes[b] + m_firstVertex].position.y,
                   m_verticesPool.vertices[m_verticesPool.indexes[b] + m_firstVertex].position.z);
        
        std::cout << m_verticesPool.vertices[m_verticesPool.indexes[b]].position.toString() << std::endl;
    }
    glEnd();
*/
    while (shaderPasse != shaderPassesEnd) {
        
        (*shaderPasse)->start();
        
        glDrawElements(GL_TRIANGLES, m_numIndexes, GL_UNSIGNED_INT, m_verticesPool.indexes.data() + m_firstIndex);
        
        (*shaderPasse)->stop();
        
        ++shaderPasse;
    }
    
    m_shader.stop();
}


void Q3FaceTriangleSoup::render() {
    m_shader.start();
    
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPasse = m_shader.begin();
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = m_shader.end();
    
    glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), m_verticesPool.vertices.data() + m_firstVertex);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Q3Vertex), &m_verticesPool.vertices[m_firstVertex].texcoord.x);
    
    while (shaderPasse != shaderPassesEnd) {
        
        (*shaderPasse)->start();
        
        glDrawElements(GL_TRIANGLES, m_numIndexes, GL_UNSIGNED_INT, m_verticesPool.indexes.data() + m_firstIndex);
        
        (*shaderPasse)->stop();
        
        ++shaderPasse;
    }
    
    m_shader.stop();
}




/**
 *
 * Return true if the node can be safely clipped
 */
bool Q3Map::_clipTest(const Node& node) const {
    unsigned int and_clip = ~0;
    
    for (int j = 0; j < 8; ++j)
    {
        Vector4f v, cv;
        unsigned int flags = 0;
        
        v.x = (float) node.bbox[bbox_index[j][0]];
        v.y = (float) node.bbox[bbox_index[j][1]];
        v.z = (float) node.bbox[bbox_index[j][2]];
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
        return true;
    
    return false;
}


void Q3Map::_pushFaces(int index) {
    
    if (index < 0) { // Leaf
        int i = -(index + 1);
        std::shared_ptr<LeafNode> leaf = m_leafNodes[i];
        
        // PVS test
        const Vector3f& cameraPosition = SceneManager::getInstance()->getCamera()->getPosition();
        int cameraCluster = this->getClusterIndex(cameraPosition);
        if (!this->isVisible(cameraCluster, m_leafNodes[i]->cluster)) {
            return;
        }
        
        // Frustrum culling
        if (this->_clipTest(*leaf)) {
            return;
        }
        
        for (int j = 0; j < m_leafNodes[i]->numLeafFaces; ++j) {
            const int f = j + m_leafNodes[i]->firstLeafFace;
            m_facesToRender.insert(m_leafFaceIndexes[f]);
        }
        
    } else { // Node

        
        std::shared_ptr<InternalNode> node = m_internalNodes[index];
        
        //		 const Q3BspPlane& plane = m_planes[node.plane];
        //		 const Vector3f planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);
        
        // Frustrum culling
        if (this->_clipTest(*node)) {
            return;
        }
        
        this->_pushFaces(node->children[0]);
        this->_pushFaces(node->children[1]);
        
    }
}

bool Q3Map::load(FILE * file) {
    if (!file) {
        return false;
    }
    
    BspHeader bspHeader;
    
    fread(&bspHeader, sizeof(BspHeader), 1, file);
    
    if (bspHeader.magic[0] != 'I' || bspHeader.magic[1] != 'B' ||
        bspHeader.magic[2] != 'S' || bspHeader.magic[3] != 'P' ||
        bspHeader.version != Q3BSP_VERSION)
    {
        ILogger::log("Not a quake 3 map.");
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
        ILogger::log("-> Error while loading data from bsp file.\n");
        fclose(file);
        return false;
    }
    
    return true;
}

bool Q3Map::load(const char *filename) {
    FILE * file = NULL;
    
    ILogger::log("Bsp:: Loading %s ...\n", filename);
    
    file = fopen(filename, "rb");
    if (!file) {
        ILogger::log("Unable to open the bsp file %s.\n", filename);
        return false;
    }
    
    if(!this->load(file))
        return false;
    
    fclose(file);
    ILogger::log("done\n");
    return true;

}


bool Q3Map::_loadVertices(FILE * file, const BspLumpEntry& verticesLump, const BspLumpEntry& indexesLump) {
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
        
        m_verticesPool.vertices[i].texcoord.x = bspVertices[i].texcoord[0][0];
        m_verticesPool.vertices[i].texcoord.y = bspVertices[i].texcoord[0][1];
        
        m_verticesPool.vertices[i].lmcoord.x = bspVertices[i].texcoord[1][0];
        m_verticesPool.vertices[i].lmcoord.y = bspVertices[i].texcoord[1][1];
        
        m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];
        
        m_verticesPool.vertices[i].normal.x = bspVertices[i].normal[0];
        m_verticesPool.vertices[i].normal.y = bspVertices[i].normal[1];
        m_verticesPool.vertices[i].normal.z = bspVertices[i].normal[2];
        
        m_verticesPool.vertices[i].color.x = bspVertices[i].color[0];
        m_verticesPool.vertices[i].color.y = bspVertices[i].color[1];
        m_verticesPool.vertices[i].color.z = bspVertices[i].color[2];
        m_verticesPool.vertices[i].color.w = bspVertices[i].color[3];
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



bool Q3Map::_loadFaces(FILE * file, const BspLumpEntry& facesLump, const BspLumpEntry& shadersLump, const BspLumpEntry& lightmapsLump) {
    if (!file) {
        return false;
    }
    
    std::shared_ptr<Q3ShaderManager> shaderManager = Q3ShaderManager::getInstance();
    std::shared_ptr<TextureManager> textureManager = TextureManager::getInstance();
    std::vector<Q3Shader> shaders;
    std::string infoString("");
    int n = 0, nPlanar = 0, nPatch = 0, nTriSoup = 0, nFlare = 0,nBad = 0;
    std::shared_ptr<Q3Face> face = nullptr;
    
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
    
    for (int i = 0; i < n; ++i) {
        
        switch (bspFaces[i].type) {
            
            case Q3Face::FACE_TRIANGLE_SOUP:
            case Q3Face::FACE_PLANAR:
            {
                face = std::make_shared<Q3FacePlanar>(m_verticesPool);
                face->type = bspFaces[i].type;
                
                if (shaderManager->exists(bspShaders[bspFaces[i].shader].name)) {
                    
                    face->m_shader = *(shaderManager->at(bspShaders[bspFaces[i].shader].name));
                    
                    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPass = face->m_shader.begin();
                    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = face->m_shader.end();
                    
                    while (shaderPass != shaderPassesEnd) {
                        Texture texture = (*shaderPass)->getTexture();
                        
                        if((*shaderPass)->getFlags() & SHADER_LIGHTMAP) {
                            texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].vertex].lmcoord.x;
                        }
                        else {
                            texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].vertex].texcoord.x;
                        }
                        
                        texture.m_Stride = sizeof(Q3Vertex);
                        (*shaderPass)->setTexture(texture);
                        (*shaderPass)->init();
                        
                        ++shaderPass;
                    }

                }
                else {
                    
                    Texture tex[2];
                    Q3Shader shaderDefault;
                    std::shared_ptr<Q3ShaderPass> shaderPass = std::make_shared<Q3ShaderPass>();
                    
                    tex[0].m_name   = bspShaders[bspFaces[i].shader].name;
                    tex[0].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].vertex].texcoord.x;
                    tex[0].m_Stride = sizeof(Q3Vertex);
                    
                    tex[1].m_name   = "lightmap";
                    tex[1].m_texId  = lmIds[bspFaces[i].lm_index];
                    tex[1].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].vertex].lmcoord.x;
                    tex[1].m_Stride = sizeof(Q3Vertex);
                    
                    shaderDefault.setName("default");
                    shaderDefault.setFlags(0);
                    
                    shaderPass->setFlags(SHADER_LIGHTMAP | SHADER_DEPTHWRITE);
                    shaderPass->setTexture(tex[1]);
                    shaderPass->setDepthFunc(GL_LEQUAL);
                    shaderDefault.push_back(shaderPass);
                    
                    shaderPass = std::make_shared<Q3ShaderPass>();
                    shaderPass->setFlags(SHADER_BLENDFUNC | SHADER_DEPTHWRITE);
                    shaderPass->setTexture(tex[0]);
                    shaderPass->setDepthFunc(GL_LEQUAL);
                    shaderPass->setBlending(GL_DST_COLOR, GL_ZERO);
                    shaderPass->init();
                    shaderDefault.push_back(shaderPass);
                    
                    face->m_shader = shaderDefault;
                    infoString = "(No shader script found, default loaded)";
                }
                
                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                
                
                m_faces.push_back(face);
               // face = nullptr;

                ++nPlanar;
                
                ILogger::log("---> Face %d, type planar, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
                break;
                
            case Q3Face::FACE_PATCH:
            {
                face = std::make_shared<Q3FacePatch>(m_verticesPool);
                face->type = bspFaces[i].type;

                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                
                
                m_faces.push_back(face);

                ++nPatch;
                ILogger::log("---> Face %d, type patch, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
            break;
/*
            case Q3Face::FACE_TRIANGLE_SOUP:
            {
                face = std::make_shared<Q3FaceTriangleSoup>(m_verticesPool);
                face->type = bspFaces[i].type;


                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                
                
                m_faces.push_back(face);
                ++nTriSoup;
                ILogger::log("---> Face %d, type triangle soup, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
                break;
*/
                
            case Q3Face::FACE_FLARE:
            {
                face = std::make_shared<Q3FaceFlare>(m_verticesPool);
                face->type = bspFaces[i].type;
                
                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                
                
                m_faces.push_back(face);
                
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




bool Q3Map::_loadBspTree(FILE * file,
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
        
        m_internalNodes.push_back(node);
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
        
        m_leafNodes.push_back(leaf);
    }
    
    ILogger::log("--> %d leafs loaded.\n", n);
    
    n = leafFaceLump.length / sizeof(int);
    std::unique_ptr<int[]> bspLeafFace = std::make_unique<int[]>(n);
    fseek(file, leafFaceLump.offset, SEEK_SET);
    fread(bspLeafFace.get(), leafFaceLump.length, 1, file);
    
    for (int i = 0; i < n; ++i) {
        m_leafFaceIndexes.push_back(bspLeafFace[i]);
    }
    
    ILogger::log("--> %d leaf face indexes loaded.\n", n);
    
    
    n = leafBrushLump.length / sizeof(int);
    std::unique_ptr<int[]> bspLeafBrush = std::make_unique<int[]>(n);
    fseek(file, leafBrushLump.offset, SEEK_SET);
    fread(bspLeafBrush.get(), leafBrushLump.length, 1, file);
    
    for (int i = 0; i < n; ++i) {
        m_leafBrushIndexes.push_back(bspLeafBrush[i]);
    }
    
    ILogger::log("--> %d leaf brush indexes loaded.\n", n);
    
    
    fseek(file, visDataLump.offset, SEEK_SET);
    fread(&m_visData, 2, sizeof(int), file);
    
    int size = m_visData.numClusters * m_visData.sizeCluster;
    m_visData.bits = std::make_unique<unsigned char[]>(size);
    fread(m_visData.bits.get(), 1, size, file);
    
    ILogger::log("--> vis data loaded (%d x %d).\n", m_visData.numClusters, m_visData.sizeCluster);
    
    
    return true;
}


void Q3Map::update(double delta) {
    SceneNodeList::iterator i = m_children.begin();
    SceneNodeList::iterator end = m_children.end();
    
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
    
    m_facesToRender.clear();
    this->_pushFaces(0);
    
    while (i != end) {
        (*i)->update(delta);
        ++i;
    }
}


void Q3Map::render() {
    std::set<int>::iterator faceToRender = m_facesToRender.begin();
    std::set<int>::iterator faceToRenderEnd = m_facesToRender.end();
  
   
    
    while (faceToRender != faceToRenderEnd) {
        
        std::shared_ptr<Q3Face> face = m_faces[*faceToRender];
        
        face->render();
        ++faceToRender;
    }
  
    SceneNodeList::iterator children = m_children.begin();
    SceneNodeList::iterator childrenEnd = m_children.end();
    
    while (children != childrenEnd) {
        glPushMatrix();
        (*children)->render();
        glPopMatrix();
        ++children;
    }
    
}
