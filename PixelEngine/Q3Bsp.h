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
	LUMP_VERTICES,
	LUMP_INDEXES,
	LUMP_EFFECTS,
	LUMP_FACES,
	LUMP_LIGHTMAPS,
	LUMP_LIGHTVOLS,
	LUMP_VISDATA,
	LUMP_TOTAL
};


enum {
	FACE_BAD = 0,
	FACE_PLANAR,
	FACE_PATCH,
	FACE_TRIANGLE_SOUP,
	FACE_FLARE
};

/*
typedef	struct {
	int offset;
	int length;
} Q3BspLumpEntry;


typedef	struct {
	char magic[4];
	int version;
	Q3BspLumpEntry entries[LUMP_TOTAL];
} Q3BspHeader;
*/
typedef	struct {
	char * ents;
} Q3BspEntity;
/*
typedef	struct {
	char name[64];
	int flags;
	int contents;
} Q3BspShader;
*/
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
/*
typedef	struct {
	float position[3];
	float texcoord[2][2];
	float normal[3];
	unsigned char color[4];
} Q3BspVertex;
*/
/*
typedef	struct {
	int offset;
} Q3BspMeshVert;
*/
typedef	struct {
	char * name[64];
	int brush;
	int unknown;
} Q3BspEffect;

/*
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
*/

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


class Q3Vertex {
public:
    Vector3f position;
    Vector2f texcoord;
    Vector2f lmcoord;
    Vector3f normal;
    Vector4<unsigned char> color;
    
    Q3Vertex operator + (const Q3Vertex& patchVertex) {
        Q3Vertex r;
        
        r.position += patchVertex.position;
        r.texcoord += patchVertex.texcoord;
        r.lmcoord += patchVertex.lmcoord;
        
        return r;
    }
    
    Q3Vertex operator * (float rhs) {
        Q3Vertex r;
        
        r.position *= rhs;
        r.texcoord *= rhs;
        r.lmcoord *= rhs;
        
        return r;
    }
};

typedef std::vector<Q3Vertex> Q3VerticesList;
typedef std::vector<int> Q3IndexesList;


class Q3Face {
    Q3Shader& m_shader;
    const Q3VerticesList& m_worldVertices;
    const Q3IndexesList& m_worldIndexes;
    
//    int m_effectIndex;
//    int m_lightmapIndex;
    
    int m_firstVertex;
    int n_numVertices;
    
    int m_firstIndex;
    int m_numIndexes;
    
    
//    int lightmapX, lightmapY;
//    int lightmapWidth, lightmapHeight;
//    Vector3f lightmapOrigin;
//    Vector3f lightmapVecs[3];
    
public:
    Q3Face(const Q3VerticesList& worldVertices, const Q3IndexesList& worldIndexes, Q3Shader& shader)
    :m_shader(shader), m_worldVertices(worldVertices), m_worldIndexes(worldIndexes){}
    virtual ~Q3Face() {}
    
    
    virtual void render() = 0;
};


class Q3FacesList : public std::vector<Q3Face> {
    
//    std::vector<Q3Face> m_faces;
    
public:
/*
    typedef std::vector<Q3Face>::iterator iterator;
    typedef std::vector<Q3Face>::const_iterator const_iterator;
    
    iterator begin() { return m_faces.begin(); }
    const_iterator begin() const { return m_faces.begin(); }
    
    iterator end() { return m_faces.end(); }
    const_iterator end() const { return m_faces.end(); }
*/

};


class Q3FacePlanar : public Q3Face {
    
public:
    
    
    
    void render() {
        
    }

};


class Q3FacePatch : public Q3Face {

    class Q3BiquadraticBezier {
    public:
        GLint m_lod;
        Q3Vertex m_anchors[9];
        std::unique_ptr<Q3Vertex[]> m_vertices;
        std::unique_ptr<GLuint[]> m_indices;
        
        Q3BiquadraticBezier()
        :m_lod(0) {}
        
        bool tesselate();
        void render();
    };
    

    int patchWidth, patchHeight;
    std::unique_ptr<Q3BiquadraticBezier[]> m_bezierPatches;
    
public:

    bool init() {
        return true;
    }
    
    void render() {};
};


class Q3FaceTriangleSoup : public Q3Face {
    
public:
    void render() {
        
    }
    
};

class Q3FaceFlare : public Q3Face {
    
public:
    void render() {
        
    }
};


class Q3Bsp : public SceneNode {

//	Q3BspHeader m_header;
//	std::shared_ptr<Q3ShaderManager> m_shaderManager;
	std::unique_ptr<GLuint[]> m_lmIds;

	GLdouble m_Delta;
	int m_cameraCluster;
	std::set<int> m_facesToRender;
	Matrix4f m_clipMatrix;
//	std::shared_ptr<Camera> m_attachedCamera;
//	Q3ShaderPass* m_currentShaderPass;
	std::map<int, Q3FacePatch> m_patches;

	// Level data
    Q3VerticesList   m_worldVertices;
    Q3IndexesList    m_worldIndexes;
    Q3FacesList      m_faces;
//	std::vector<Q3Shader>   m_shaders;
    
    /*
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
	std::string m_entities;
*/
	static int bbox_index[8][3];
    

    // Private methods
    
	bool _loadVertices(FILE * file, int offset, int length);
	bool _loadIndexes(FILE * file, int offset, int length);
	bool _loadShaders(FILE * file, int offset, int length);
	bool _loadFaces(FILE * file, int offset, int length);
	bool _loadLightMaps(FILE * file, int offset, int length);
	/*bool _loadBspTree(FILE * file, int offset, int length);
	bool _loadVisData(FILE * file, int offset, int length);
	bool _loadEntities(FILE * file, int offset, int length);

	void _selectFaces(int index);

	void _setCurrentShaderPass(Q3ShaderPass* currentShaderPass) { m_currentShaderPass = currentShaderPass; };
	void _beginShaderPass();
	void _endShaderPass();
     */
	
public:
	Q3Bsp();
	virtual ~Q3Bsp();
/*
	int getLeafIndex(const Vector3d& v) const;
	bool checkClusterVisibility(int from, int to) const;


	void render();
	void update(GLdouble delta);
*/
	bool load(const char* filename);
 
};




#endif