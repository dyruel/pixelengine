//
//  Bsp.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


// Doc: http://www.mralligator.com/q3/

#ifndef PixelEngine_Q3Bsp_h
#define PixelEngine_Q3Bsp_h

#include <set>

#include "Q3Shader.h"
#include "Scene.h"

const int Q3BSP_VERSION = 46; // quake 3 maps

enum {
	LUMP_ENTITIES = 0,
	LUMP_SHADERS,
	LUMP_PLANES,
	LUMP_NODES,
	LUMP_LEAFS,
	LUMP_LEAFFACES,
	LUMP_LEAFBRUSHES,
	LUMP_MODELS,
	LUMP_BRUSHES,
	LUMP_BRUSHSIDES,
	LUMP_VERTEXES,
	LUMP_MESHVERTS,
	LUMP_EFFECTS,
	LUMP_FACES,
	LUMP_LIGHTMAPS,
	LUMP_LIGHTVOLS,
	LUMP_VISDATA,
	LUMP_TOTAL
};


enum {
	FACE_BAD = 0,
	FACE_POLYGON,
	FACE_PATCH,
	FACE_MESH,
	FACE_BILLBOARD
};

typedef	struct {
	int offset;
	int length;
} Q3BspLumpEntry;

typedef	struct {
	char magic[4];
	int version;
	Q3BspLumpEntry entries[LUMP_TOTAL];
} Q3BspHeader;

typedef	struct {
	char * ents;
} Q3BspEntity;

typedef	struct {
	char name[64];
	int flags;
	int contents;
} Q3BspShader;

typedef	struct {
	float normal[3];
	float dist;
} Q3BspPlane;

typedef	struct {
	int plane;
	int children[2];
	int bbox[6];
} Q3BspNode;

typedef	struct {
	int cluster;
	int area;
	int bbox[6];
	int leafface;
	int n_leaffaces;
	int leafbrush;
	int n_leafbrushes;
} Q3BspLeaf;

typedef	struct {
	int face;
} Q3BspLeafFace;

typedef	struct {
	int brush;
} Q3BspLeafBrush;

typedef	struct {
	int bbox[6];
	int face;
	int n_faces;
	int brush;
	int n_brushes;
} Q3BspModel;

typedef	struct {
	int brushside;
	int n_brushsides;
	int shader;
} Q3BspBrush;

typedef	struct {
	int plane;
	int shader;
} Q3BspBrushSide;

typedef	struct {
	float position[3];
	float texcoord[2][2];
	float normal[3];
	unsigned char color[4];
} Q3BspVertex;

typedef	struct {
	int offset;
} Q3BspMeshVert;

typedef	struct {
	char * name[64];
	int brush;
	int unknown;
} Q3BspEffect;

typedef	struct {
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
} Q3BspFace;


typedef	struct {
	unsigned char  map[128][128][3];
} Q3BspLightMap;


typedef	struct {
	unsigned char  ambient[3];
	unsigned char  directional[3];
	unsigned char  dir[2];
} Q3BspLightVol;


typedef	struct {
	int n_clusters;
	int sz_clusters;
	unsigned char * bits;
} Q3BspVisData;


/*
class BspNode : public SceneNode {
public:

	BspNode();
	virtual ~BspNode();

	void render();
	void update(GLdouble delta);
};

class BspLeaf : public SceneNode {

};
*/


class Q3Bsp : public SceneNode {

	Q3BspHeader m_header;
	std::shared_ptr<Q3ShaderManager> m_shaderManager;
	std::unique_ptr<Q3Shader[]> m_shaders;
	std::unique_ptr<GLuint[]> m_textureIds;
	std::unique_ptr<GLuint[]> m_lmIds;

	GLuint m_blankTexId;
	int m_cameraCluster;
	std::set<int> m_facesToRender;
	Matrix4f m_clipMatrix;

	std::shared_ptr<Camera> m_attachedCamera;

	// BSP data
	std::unique_ptr<Q3BspVertex[]> m_vertexes;
	std::unique_ptr<Q3BspMeshVert[]> m_meshVerts;
	std::unique_ptr<Q3BspShader[]> m_bspShaders;
	std::unique_ptr<Q3BspLightMap[]> m_lightMaps;
	int m_nFaces;
	std::unique_ptr<Q3BspFace[]> m_faces;

	std::unique_ptr<Q3BspNode[]> m_nodes;
	std::unique_ptr<Q3BspPlane[]> m_planes;
	int m_nLeafs;
	std::unique_ptr<Q3BspLeaf[]> m_leafs;
	std::unique_ptr<Q3BspLeafFace[]> m_leafFaces;
	std::unique_ptr<Q3BspLeafBrush[]> m_leafBrushes;

	std::unique_ptr<Q3BspVisData> m_visData;

	static int bbox_index[8][3];

	bool _loadVertexes(FILE * file);
	bool _loadMeshVerts(FILE * file);
	bool _loadShaders(FILE * file);
	bool _loadFaces(FILE * file);
	bool _loadLightMaps(FILE * file);
	bool _loadBspTree(FILE * file);
	bool _loadVisData(FILE * file);

	void _selectFaces(int index);
	
public:
	Q3Bsp();
	virtual ~Q3Bsp();

	int getLeafIndex(const Vector3d& v) const;
	bool checkClusterVisibility(int from, int to) const;

	void attachCamera(std::shared_ptr<Camera> camera) { m_attachedCamera = camera; };


	void render();
	void update(GLdouble delta);

	bool load(const char* filename);
};




#endif