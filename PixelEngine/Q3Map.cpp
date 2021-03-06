//
//  Q3Map.cpp
//  pixelengine
//
//  Created by Morgan on 15/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//



#include "Q3Map.h"

bool CQ3Map::load(const char* filename) {

	FileSystem * fileSystem = FileSystem::getInstance();
	CMemoryManager * memoryManager = CMemoryManager::getInstance();
	GLuint * lmIds = nullptr;

	ILogger::log("Loading Q3 Map %s\n", filename);

	if (!fileSystem->open(filename)) {
		return false;
	}
	
	// Read the file header and check version
	fileSystem->seek(0);
	fileSystem->read(&m_header, sizeof(CBspHeader));

	if (m_header.magic != 0x50534249 ||
		m_header.version != Q3BSP_VERSION)
	{
		ILogger::log("Not a quake 3 map (magic :%d, version %d).", m_header.magic, m_header.version);
		fileSystem->close();
		return false;
	}

	ILogger::log("-> Magic : %d\n-> Version : %d\n", m_header.magic, m_header.version);

	// Calculate the total amount of memory needed to store the BSP data...
	u32 totalMapSize = 0;
	u32 additionalSize = 0;
//	CBspLumpEntry extraEntries[2];

	for (int i = LUMP_ENTITIES; i < LUMP_TOTAL; ++i) {
		totalMapSize += m_header.entries[i].length;
	}

	// ...  and additional data
	additionalSize += (m_header.entries[LUMP_FACES].length / sizeof(CBspFace)) * sizeof(CShadedFace);
	additionalSize += (m_header.entries[LUMP_LIGHTMAPS].length / sizeof(CBspLightMap)) * sizeof(GLuint);

	// Reserve a chunk
	m_memoryChunk = memoryManager->getMemory(totalMapSize + additionalSize, "Q3 Map");

	// Load each lump into the memory chunk
	if (
		!this->loadShaders			(m_header.entries[LUMP_SHADERS])		||
		!this->loadLightmaps		(m_header.entries[LUMP_LIGHTMAPS])		||
		!this->loadVerts			(m_header.entries[LUMP_VERTICES])		||
		!this->loadFaces			(m_header.entries[LUMP_FACES])			||
		!this->loadPlanes			(m_header.entries[LUMP_PLANES])			||
		!this->loadNodes			(m_header.entries[LUMP_NODES])			||
		!this->loadLeaves			(m_header.entries[LUMP_LEAVES])			||
		!this->loadLeafFaces		(m_header.entries[LUMP_LEAFFACES])		||
		!this->loadVisData			(m_header.entries[LUMP_VISDATA])		||
		!this->loadEntities			(m_header.entries[LUMP_ENTITIES])		||
		!this->loadModels			(m_header.entries[LUMP_MODELS])			||
		!this->loadMeshIndices		(m_header.entries[LUMP_MESHINDICES])	||
		!this->loadBrushes			(m_header.entries[LUMP_BRUSHES])		||
		!this->loadBrushSides		(m_header.entries[LUMP_BRUSHSIDES])		||
		!this->loadLeafBrushes		(m_header.entries[LUMP_LEAFBRUSHES])	||
		!this->loadFogs				(m_header.entries[LUMP_FOGS])
//		!this->loadExtras			()
		)
	{
		ILogger::log("-> Error while loading data from bsp file.\n");
		fileSystem->close();
		return false;
	}


//	m_faceToDrawIndices = (s32*)m_memoryChunk->reserve(m_numFaces * sizeof(s32));
//	m_pushedFaces =		  (s32*)m_memoryChunk->reserve(m_numFaces * sizeof(s32));
//	m_faceQ3shaders = (Q3Shader**)m_memoryChunk->reserve(m_numFaces * sizeof(Q3Shader*));
//	m_numFacesToDraw	= 0;

	//(s32*)m_memoryChunk->reserve((m_header.entries[LUMP_FACES].length / sizeof(CBspFace)) * sizeof(s32));

	m_shadedFaces = (CShadedFace*) m_memoryChunk->reserve(m_numFaces * sizeof(CShadedFace));
	lmIds = (GLuint*)m_memoryChunk->reserve(m_numLightMaps * sizeof(GLuint));
	
	glGenTextures(m_numLightMaps, lmIds);

	for (int i = 0; i < m_numLightMaps; ++i) {
		glBindTexture(GL_TEXTURE_2D, lmIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, m_lightMaps[i].map);
	}
	
	memoryManager->print();

	for (int i = 0; i < m_numFaces; ++i){
		const CBspFace& face = m_faces[i];

		m_shadedFaces[i].face = &m_faces[i];
		m_shadedFaces[i].toBeDrawn = false;

		for (int b = face.firstElementIdx; b < face.firstElementIdx + face.numElements; ++b) {
			m_meshIndices[b] += face.firstVertexIdx;
		}

		switch (face.type) {

			case FACE_TRIANGLE_SOUP:
			case FACE_PLANAR:
			{
				if (Q3ShaderManager::getInstance()->exists(m_shaders[face.shaderIdx].name)) {

					const Q3Shader& shader = Q3ShaderManager::getInstance()->at(m_shaders[face.shaderIdx].name);
					
					/*
					std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPass = shader->begin();
					std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = shader->end();

					while (shaderPass != shaderPassesEnd) {
						Texture texture = (*shaderPass)->getTexture();

						if ((*shaderPass)->getFlags() & SHADER_LIGHTMAP) {
							//texture.m_texId = lmIds[bspFaces[i].lm_index];
							//texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].lmcoord.x;
						}
						else {
							//texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].texcoord.x;
						}

						//texture.m_Stride = sizeof(Q3Vertex);
						(*shaderPass)->setTexture(texture);
						(*shaderPass)->init();

						++shaderPass;
					}
					*/
					m_shadedFaces[i].shader = shader;
					m_shadedFaces[i].shader.init();
//					m_faceQ3shaders[i] = shader;

				}
				else {

					Texture tex[2];
					Q3Shader shaderDefault;
					Q3ShaderPass shaderPass[2];

					tex[0].m_name = m_shaders[face.shaderIdx].name;
					//tex[0].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].texcoord.x;
					//tex[0].m_Stride = sizeof(Q3Vertex);

					tex[1].m_name = "lightmap";
					tex[1].m_texId = lmIds[face.lmIdx];
//					std::cout << lmIds[face.lmIdx] << std::endl;
					//tex[1].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].lmcoord.x;
					//tex[1].m_Stride = sizeof(Q3Vertex);

					shaderDefault.clear();
					shaderDefault.setName("default");
					shaderDefault.setFlags(0);
					
					shaderPass[0].clear();
					shaderPass[0].setFlags(SHADER_LIGHTMAP | SHADER_DEPTHWRITE);
					shaderPass[0].addFrame(tex[1]);
					shaderPass[0].setDepthFunc(GL_LEQUAL);
					shaderDefault.addShaderPass(shaderPass[0]);

					shaderPass[1].clear();
					shaderPass[1].setFlags(SHADER_BLENDFUNC | SHADER_DEPTHWRITE);
					shaderPass[1].addFrame(tex[0]);
					shaderPass[1].setDepthFunc(GL_LEQUAL);
					shaderPass[1].setBlending(GL_DST_COLOR, GL_ZERO);
					//shaderDefault.addShaderPass(shaderPass[1]);

					m_shadedFaces[i].shader = shaderDefault;
					m_shadedFaces[i].shader.init();

//					std::string name(m_shaders[face.shaderIdx].name);

//					m_faceQ3shaders[i] = shaderDefault.get();
					//(*Q3ShaderManager::getInstance())[name] = shaderDefault;
					//face->m_shader = shaderDefault;
					//infoString = "(No shader script found, default loaded)";
				}

			}
			break;

			case FACE_PATCH:
			{

			}
			break;

			case FACE_FLARE:
			{

			}
			break;

			case FACE_BAD:
			default:
			{

				ILogger::log("---> Face %d, type bad\n", i);
			}
			break;
		}


	}
	


	glGenBuffers(2, vboIds);

	glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, m_numVertices * sizeof(CBspVertex), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numVertices * sizeof(CBspVertex), m_vertices);
	glVertexPointer(3, GL_FLOAT, sizeof(CBspVertex), 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(CBspVertex), (GLvoid*) sizeof(Vector3f));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numMeshIndices * sizeof(s32), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_numMeshIndices * sizeof(s32), m_meshIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



	ILogger::log("done\n", filename);
	fileSystem->close();
	return true;
}

void CQ3Map::render() {

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);

	for (int i = 0; i < m_numFaces; ++i){
		if (!m_shadedFaces[i].toBeDrawn)
			continue;
		//const CBspFace& face = m_faces[i];
		CShadedFace& shadedFace = m_shadedFaces[i];

		const CBspFace* face = m_shadedFaces[i].face;
		Q3Shader& shader = shadedFace.shader;

//	for (int i = 0; i < m_numFacesToDraw; ++i){
//		const CBspFace& face = m_faces[m_faceToDrawIndices[i]];
		//const CBspShader& shader = m_shaders[face.shaderIdx];

		if (shadedFace.face->type == FACE_TRIANGLE_SOUP || shadedFace.face->type == FACE_PLANAR) {
			//Q3Shader * shader = Q3ShaderManager::getInstance()->at(m_shaders[face.shaderIdx].name).get();
			shader.start();

			for (int j = 0; j < shader.getNumShaderPasses(); ++j) {
				Q3ShaderPass& shaderPass = shader.getShaderPass(j);

				shaderPass.start();

				glDrawElements(GL_TRIANGLES, face->numElements, GL_UNSIGNED_INT, (GLvoid*)(sizeof(s32) * face->firstElementIdx));

//				glClientActiveTexture(GL_TEXTURE1);
//				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//				glTexCoordPointer(2, GL_FLOAT, sizeof(CBspVertex), (GLvoid*) sizeof(Vector3f));

				shaderPass.stop();

			}

			
			

			shader.stop();
			
		}


		
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}


/*
glBegin(GL_TRIANGLES);
for (int b = face.firstElementIdx; b < face.firstElementIdx + face.numElements; ++b)
{
glVertex3f(m_vertices[m_meshIndices[b] + face.firstVertexIdx].position.x,
m_vertices[m_meshIndices[b] + face.firstVertexIdx].position.y,
m_vertices[m_meshIndices[b] + face.firstVertexIdx].position.z);
}
glEnd();
*/
/*
glBegin(GL_TRIANGLES);
for (int b = face.firstElementIdx; b < face.firstElementIdx + face.numElements; ++b)
{
glVertex3f(m_vertices[m_meshIndices[b] ].position.x,
m_vertices[m_meshIndices[b] ].position.y,
m_vertices[m_meshIndices[b] ].position.z);
}
glEnd();
*/

void CQ3Map::update(const f64& delta) {

	// Get clip matrix
	float m[16], p[16], r[16];
	glGetFloatv(GL_PROJECTION_MATRIX, p);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	glPushMatrix();
	glLoadMatrixf(p);
	glMultMatrixf(m);
	glGetFloatv(GL_MODELVIEW_MATRIX, m_clipMatrix);
	glPopMatrix();

//	m_clipMatrix = r;
//	m_numFacesToDraw = 0;
	m_cameraCluster = this->getLeafFromPosition(CSceneManager::getInstance()->getCamera()->getPosition()).cluster;
//	memset(m_pushedFaces, 0, m_numFaces);
	for (int i = 0; i < m_numFaces; ++i){
		m_shadedFaces[i].toBeDrawn = false;
	}

	this->updateFacesToDrawIndices(0);
}



bool CQ3Map::loadShaders(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_shaders = nullptr;
	m_numShaders = 0;

	if (l.length > 0) {
		//m_shaders = (CBspShader*)(*m_memoryChunk)[l.offset];
		m_shaders = (CBspShader*)m_memoryChunk->reserve(l.length);
		m_numShaders = l.length / sizeof(CBspShader);
		fileSystem->seek(l.offset);
		fileSystem->read(m_shaders, l.length);
		ILogger::log("-> %d shaders\n", m_numShaders);
	}
	else {
		ILogger::log("-> No shaders\n");
	}


	return true;
}


bool CQ3Map::loadLightmaps(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_lightMaps		= nullptr;
	m_numLightMaps	= 0;

	if (l.length > 0) {
		//m_lightMaps = (CBspLightMap*)(*m_memoryChunk)[l.offset];
		m_lightMaps = (CBspLightMap*)m_memoryChunk->reserve(l.length);
		m_numLightMaps = l.length / sizeof(CBspLightMap);
		fileSystem->seek(l.offset);
		fileSystem->read(m_lightMaps, l.length);
		ILogger::log("-> %d lightmaps\n", m_numLightMaps);
	}
	else {
		ILogger::log("-> No lightmaps\n");
	}

	return true;
}


bool CQ3Map::loadVerts(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_vertices = nullptr;
	m_numVertices = 0;

	if (l.length > 0) {
		//m_vertices = (CBspVertex*)(*m_memoryChunk)[l.offset];
		m_vertices = (CBspVertex*)m_memoryChunk->reserve(l.length);
		m_numVertices = l.length / sizeof(CBspVertex);
		fileSystem->seek(l.offset);
		fileSystem->read(m_vertices, l.length);
		ILogger::log("-> %d vertices (%d Ko)\n", m_numVertices, l.length >> 10);
	}
	else {
		ILogger::log("-> No vertices\n");
	}

	return true;
}


bool CQ3Map::loadFaces(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_faces = nullptr;
	m_numFaces = 0;

	if (l.length > 0) {
		//m_faces = (CBspFace*)(*m_memoryChunk)[l.offset];
		m_faces = (CBspFace*)m_memoryChunk->reserve(l.length);
		m_numFaces = l.length / sizeof(CBspFace);
		fileSystem->seek(l.offset);
		fileSystem->read(m_faces, l.length);
		ILogger::log("-> %d faces\n", m_numFaces);
	}
	else {
		ILogger::log("-> No faces\n");
	}

	return true;
}


bool CQ3Map::loadPlanes(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_planes = nullptr;
	m_numPlanes = 0;

	if (l.length > 0) {
		//m_planes = (CBspPlane*)(*m_memoryChunk)[l.offset];
		m_planes = (CBspPlane*)m_memoryChunk->reserve(l.length);
		m_numPlanes = l.length / sizeof(CBspPlane);
		fileSystem->seek(l.offset);
		fileSystem->read(m_planes, l.length);
		ILogger::log("-> %d planes\n", m_numPlanes);
	}
	else {
		ILogger::log("-> No planes\n");
	}

	return true;
}


bool CQ3Map::loadNodes(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_nodes = nullptr;
	m_numNodes = 0;

	if (l.length > 0) {
		//m_nodes = (CBspNode*)(*m_memoryChunk)[l.offset];
		m_nodes = (CBspNode*)m_memoryChunk->reserve(l.length);
		m_numNodes = l.length / sizeof(CBspNode);
		fileSystem->seek(l.offset);
		fileSystem->read(m_nodes, l.length);
		ILogger::log("-> %d nodes\n", m_numNodes);
	}
	else {
		ILogger::log("-> No nodes\n");
	}

	return true;
}


bool CQ3Map::loadLeaves(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_leaves = nullptr;
	m_numLeaves = 0;

	if (l.length > 0) {
		//m_leaves = (CBspLeaf*)(*m_memoryChunk)[l.offset];
		m_leaves = (CBspLeaf*)m_memoryChunk->reserve(l.length);
		m_numLeaves = l.length / sizeof(CBspLeaf);
		fileSystem->seek(l.offset);
		fileSystem->read(m_leaves, l.length);
		ILogger::log("-> %d leaves\n", m_numLeaves);
	}
	else {
		ILogger::log("-> No leaves\n");
	}

	return true;
}


bool CQ3Map::loadLeafFaces(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_leafFaceIndices = nullptr;
	m_numLeafFaceIndices = 0;

	if (l.length > 0) {
		//m_leafFaceIndices = (s32*)(*m_memoryChunk)[l.offset];
		m_leafFaceIndices = (s32*)m_memoryChunk->reserve(l.length);
		m_numLeafFaceIndices = l.length / sizeof(s32);
		fileSystem->seek(l.offset);
		fileSystem->read(m_leafFaceIndices, l.length);
		ILogger::log("-> %d leaf face indices\n", m_numLeafFaceIndices);
	}
	else {
		ILogger::log("-> No leaf face indices\n");
	}

	return true;
}


bool CQ3Map::loadVisData(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_visData.numClusters = m_visData.sizeClusters = 0;
	m_visData.bits = nullptr;

	if (l.length > 0) {
		fileSystem->seek(l.offset);
		fileSystem->read(&m_visData, sizeof(s32)*2);

		s32 size = m_visData.numClusters * m_visData.sizeClusters;
		//m_visData.bits = (u8*)(*m_memoryChunk)[l.offset];
		m_visData.bits = (u8*)m_memoryChunk->reserve(size);

		fileSystem->read(m_visData.bits, size);
		ILogger::log("-> Vis data size %d\n", size);
	}
	else {
		ILogger::log("-> No vis data\n");
	}

	return true;
}


bool CQ3Map::loadEntities(const CBspLumpEntry& l) {


	return true;
}


bool CQ3Map::loadModels(const CBspLumpEntry& l) {

	return true;
}


bool CQ3Map::loadMeshIndices(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_meshIndices = nullptr;
	m_numMeshIndices = 0;

	if (l.length > 0) {
		//m_meshIndices = (s32*)(*m_memoryChunk)[l.offset];
		m_meshIndices = (s32*)m_memoryChunk->reserve(l.length);
		m_numMeshIndices = l.length / sizeof(s32);
		fileSystem->seek(l.offset);
		fileSystem->read(m_meshIndices, l.length);
		ILogger::log("-> %d mesh indices (%d Ko)\n", m_numMeshIndices, l.length >> 10);
	}
	else {
		ILogger::log("-> No mesh indices\n");
	}

	return true;
}


bool CQ3Map::loadBrushes(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_brushes = nullptr;
	m_numBrushes = 0;

	if (l.length > 0) {
		//m_brushes = (CBspBrush*)(*m_memoryChunk)[l.offset];
		m_brushes = (CBspBrush*)m_memoryChunk->reserve(l.length);
		m_numBrushes = l.length / sizeof(CBspBrush);
		fileSystem->seek(l.offset);
		fileSystem->read(m_brushes, l.length);
		ILogger::log("-> %d brushes\n", m_numBrushes);
	}
	else {
		ILogger::log("-> No brushes\n");
	}

	return true;
}

bool CQ3Map::loadBrushSides(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_brushSides = nullptr;
	m_numBrushSides = 0;

	if (l.length > 0) {
		//m_brushSides = (CBspBrushSide*)(*m_memoryChunk)[l.offset];
		m_brushSides = (CBspBrushSide*)m_memoryChunk->reserve(l.length);
		m_numBrushSides = l.length / sizeof(CBspBrushSide);
		fileSystem->seek(l.offset);
		fileSystem->read(m_brushSides, l.length);
		ILogger::log("-> %d brush sides\n", m_numBrushSides);
	}
	else {
		ILogger::log("-> No brush sides\n");
	}

	return true;
}

bool CQ3Map::loadLeafBrushes(const CBspLumpEntry& l) {
	FileSystem * fileSystem = FileSystem::getInstance();
	m_leafBrushIndices = nullptr;
	m_numLeafBrushIndices = 0;

	if (l.length > 0) {
		//m_leafBrushIndices = (s32*)(*m_memoryChunk)[l.offset];
		m_leafBrushIndices = (s32*)m_memoryChunk->reserve(l.length);
		m_numLeafBrushIndices = l.length / sizeof(s32);
		fileSystem->seek(l.offset);
		fileSystem->read(m_leafBrushIndices, l.length);
		ILogger::log("-> %d leaf brush indices\n", m_numLeafBrushIndices);
	}
	else {
		ILogger::log("-> No leaf brush indices\n");
	}

	return true;
}

bool CQ3Map::loadFogs(const CBspLumpEntry& l) {
	return true;
}

/*
bool CQ3Map::loadExtras() {

	m_faceToDrawIndices = (s32*)(*m_memoryChunk)[m_extraDataEntries[ELUMP_FACESTODRAW].offset];
	m_numFacesToDraw = 0;

	return true;
}
*/


#if 0
GLdouble			g_pushingTime = 0.0;

int Q3Map::bbox_index[8][3] =
{
    { 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
    { 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
};



void Q3FacePlanar::render() {
    m_shader.start();
	glColor3f(1.0f, 1.0f, 1.0f);
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPasse = m_shader.begin();
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = m_shader.end();


	glBindBuffer(GL_ARRAY_BUFFER, m_vboVerticesId);
//	glTexCoordPointer(3, GL_FLOAT, sizeof(Q3Vertex), 0);
	glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

//    glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), m_verticesPool.vertices.data() + m_firstVertex);
 
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

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndexesId);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    while (shaderPasse != shaderPassesEnd) {
        
        (*shaderPasse)->start();
		
		glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, 0);
		
        (*shaderPasse)->stop();
        
        ++shaderPasse;
    }
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    m_shader.stop();
}


void Q3FaceTriangleSoup::render() {
    m_shader.start();
    
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPasse = m_shader.begin();
    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = m_shader.end();
    
//    glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), m_verticesPool.vertices.data() + m_firstVertex);
//    glTexCoordPointer(2, GL_FLOAT, sizeof(Q3Vertex), &m_verticesPool.vertices[m_firstVertex].texcoord.x);
    
    while (shaderPasse != shaderPassesEnd) {
        
        (*shaderPasse)->start();
        
//        glDrawElements(GL_TRIANGLES, m_numIndexes, GL_UNSIGNED_INT, m_verticesPool.indexes.data() + m_firstIndex);
        
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
//		std::cout << "Leaf" << std::endl;
		
        int i = -(index + 1);
        std::shared_ptr<LeafNode> leaf = m_leafNodes[i];
        
        // PVS test
        if (!this->isVisible(m_cameraCluster, m_leafNodes[i]->cluster)) {
            return;
        }
        
        // Frustrum culling
        if (this->_clipTest(*leaf)) {
            return;
        }
		
		//std::copy(m_leafFaceIndexes, m_leafFaceIndexes, m_facesToRender);
		/*
        for (int j = 0; j < m_leafNodes[i]->numLeafFaces; ++j) {
            const int f = j + m_leafNodes[i]->firstLeafFace;
            m_facesToRender.push_back(m_leafFaceIndexes[f]);
        }
        */
    } else { // Node 
        std::shared_ptr<InternalNode> node = m_internalNodes[index];
//		std::cout << "Node" << std::endl;

        
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
	std::unique_ptr<GLuint[]> bspIndexes = std::make_unique<GLuint[]>(n);
    fseek(file, indexesLump.offset, SEEK_SET);
    fread(bspIndexes.get(), indexesLump.length, 1, file);

    m_verticesPool.indexes.reserve(n);
    m_verticesPool.indexes.resize(n);
    
    for (int i = 0; i < n; ++i) {
        m_verticesPool.indexes[i] = bspIndexes[i];
    }



//	std::cout << m_vboVerticesId << " " << m_vboIndexesId << std::endl;
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
        
        int firstVertex;
        int numVertices;
        
        int firstIndex;
        int numIndexes;
        
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


//	std::unique_ptr<GLuint[]> vboIds = std::make_unique<GLuint[]>(n);
//	glGenBuffers(n, vboIds.get());
    
    for (int i = 0; i < n; ++i) {
        
        switch (bspFaces[i].type) {
            
            case Q3Face::FACE_TRIANGLE_SOUP:
            case Q3Face::FACE_PLANAR:
            {
                face = std::make_shared<Q3FacePlanar>();
                face->type = bspFaces[i].type;
                
                if (shaderManager->exists(bspShaders[bspFaces[i].shader].name)) {
                    
                    face->m_shader = *(shaderManager->at(bspShaders[bspFaces[i].shader].name));
                    
                    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPass = face->m_shader.begin();
                    std::vector<std::shared_ptr<Q3ShaderPass>>::iterator shaderPassesEnd = face->m_shader.end();
                    
                    while (shaderPass != shaderPassesEnd) {
                        Texture texture = (*shaderPass)->getTexture();
                        
                        if((*shaderPass)->getFlags() & SHADER_LIGHTMAP) {
							texture.m_texId = lmIds[bspFaces[i].lm_index];
                            texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].lmcoord.x;
                        }
                        else {
							texture.m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].texcoord.x;
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
					tex[0].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].texcoord.x;
                    tex[0].m_Stride = sizeof(Q3Vertex);
                    
                    tex[1].m_name   = "lightmap";
                    tex[1].m_texId  = lmIds[bspFaces[i].lm_index];
					tex[1].m_texCoordPointer = &m_verticesPool.vertices[bspFaces[i].firstVertex].lmcoord.x;
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
				/*
				face->m_vboId = vboIds[i];

				for (int b = bspFaces[i].meshvert; b < bspFaces[i].meshvert + bspFaces[i].n_meshverts; ++b) {
					Q3Vertex v;

					v.position.x = m_verticesPool.vertices[m_verticesPool.indexes[b] + bspFaces[i].vertex].position.x;
					v.position.y = m_verticesPool.vertices[m_verticesPool.indexes[b] + bspFaces[i].vertex].position.y;
					v.position.z = m_verticesPool.vertices[m_verticesPool.indexes[b] + bspFaces[i].vertex].position.z;

					face->m_vertices.push_back(v);
				}

				glBindBuffer(GL_ARRAY_BUFFER, face->m_vboId);
				glBufferData(GL_ARRAY_BUFFER, face->m_vertices.size() * sizeof(Q3Vertex), face->m_vertices.data(), GL_STATIC_DRAW);
				*/

//				std::cout << face->m_vertices.size() << std::endl;

				//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Q3Vertex)*face->m_vertices.size(), face->m_vertices.data());

//				face->m_vboVerticesId = m_vboVerticesId;
//				face->m_vboIndexesId  = m_vboIndexesId;

//                face->m_firstVertex = bspFaces[i].vertex;
//                face->m_numVertices = bspFaces[i].n_vertexes;
                
//                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numElements = bspFaces[i].numIndexes;


				//glGenBuffers(1, &m_vboVerticesId);
				//glBindBuffer(GL_ARRAY_BUFFER, m_vboVerticesId);
				//glBufferData(GL_ARRAY_BUFFER, n * sizeof(Q3Vertex), m_verticesPool.vertices.data(), GL_STATIC_DRAW);
				//	glBufferData(GL_ARRAY_BUFFER, n * sizeof(Q3Vertex), NULL, GL_STATIC_DRAW);
				//	glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(Q3Vertex), m_verticesPool.vertices.data());

				Q3VerticesList faceVertices(m_verticesPool.vertices.begin() + bspFaces[i].firstVertex, m_verticesPool.vertices.begin() + bspFaces[i].firstVertex + bspFaces[i].numVertices);
				glGenBuffers(1, &face->m_vboVerticesId);
				glBindBuffer(GL_ARRAY_BUFFER, face->m_vboVerticesId);
				glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(Q3Vertex), 0, GL_STATIC_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, faceVertices.size() * sizeof(Q3Vertex), faceVertices.data());
				/*
				int o = 0;
				for (int b = face->m_firstVertex; b < face->m_firstVertex + face->m_numVertices; ++b) {

					std::cout << faceVertices[o].position.toString() << std::endl;
					std::cout << m_verticesPool.vertices[b].position.toString() << std::endl;
					++o;
					//std::cout << "V[" << b << "] = " << m_verticesPool.indexes[b] << std::endl;
				}
				*/
				Q3IndexesList faceIndexes(m_verticesPool.indexes.begin() + bspFaces[i].firstIndex, m_verticesPool.indexes.begin() + bspFaces[i].firstIndex + bspFaces[i].numIndexes);
				glGenBuffers(1, &face->m_vboIndexesId);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face->m_vboIndexesId);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndexes.size() * sizeof(GLuint), 0, GL_STATIC_DRAW);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faceIndexes.size() * sizeof(GLuint), faceIndexes.data());
				//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(GLuint), NULL, GL_STATIC_DRAW);
				//	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, n * sizeof(GLuint), m_verticesPool.indexes.data());

				/*
				std::cout << "START" << std::endl;
				std::cout << faceVertices.size() << std::endl;
				std::cout << face->m_firstVertex << " " << face->m_firstVertex + face->m_numVertices << std::endl;
				std::cout << face->m_firstIndex << " " << face->m_firstIndex + face->m_numIndexes << std::endl;

				for (int b = face->m_firstIndex; b < face->m_firstIndex + face->m_numIndexes; ++b) {
					std::cout << "V[" << b << "] = " << m_verticesPool.indexes[b] << std::endl;
				}
				
				std::cout << "END" << std::endl;
				*/
                
                
                m_faces.push_back(face);
               // face = nullptr;

                ++nPlanar;
                
                ILogger::log("---> Face %d, type planar, %s %s\n", i, bspShaders[bspFaces[i].shader].name, infoString.c_str());
            }
                break;
                
            case Q3Face::FACE_PATCH:
            {
                face = std::make_shared<Q3FacePatch>();
                face->type = bspFaces[i].type;
				/*
                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                */
                
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
                face = std::make_shared<Q3FaceFlare>();
                face->type = bspFaces[i].type;
				/*
                face->m_firstVertex = bspFaces[i].vertex;
                face->m_numVertices = bspFaces[i].n_vertexes;
                
                face->m_firstIndex = bspFaces[i].meshvert;
                face->m_numIndexes = bspFaces[i].n_meshverts;
                */
                
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
	const Vector3f& cameraPosition = SceneManager::getInstance()->getCamera()->getPosition();
    
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


	m_cameraCluster = this->getClusterIndex(cameraPosition);
    m_facesToRender.clear();
	GLdouble presentTime = (GLdouble)glfwGetTime() * 1000;
	this->_pushFaces(0);
	
	//std::cout << "a " << m_leafNodes.size() << std::endl;
	//for (int m = 0; m < m_leafNodes.size(); ++m) {
		//std::cout << "b " << m_leafNodes[m]->numLeafFaces << std::endl;
	//	for (int j = 0; j < m_leafNodes[m]->numLeafFaces; ++j) {
	//		const int f = j + m_leafNodes[m]->firstLeafFace;
	//		m_facesToRender.push_back(m_leafFaceIndexes[f]);
	//	}
	//}
	g_pushingTime = ((GLdouble)glfwGetTime() * 1000) - presentTime;
  
    while (i != end) {
        (*i)->update(delta);
        ++i;
    }
}


void Q3Map::render() {
	
    std::vector<int>::iterator faceToRender = m_facesToRender.begin();
	std::vector<int>::iterator faceToRenderEnd = m_facesToRender.end();
  
   

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
#endif