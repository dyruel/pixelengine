//
//  Q3Map.h
//  pixelengine
//
//  Created by Morgan on 15/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __pixelengine__Q3Map__
#define __pixelengine__Q3Map__

#include <vector>
#include <set>

#include "Types.h"
#include "Opengl.h"
#include "Vector.h"
#include "Q3Shader.h"
#include "Scene.h"
#include "Memory.h"
#include "unique.h"
#include "File.h"


#if 1
// Debug variables
//extern GLdouble			g_pushingTime;
#endif


/*
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
typedef std::vector<GLuint>      Q3IndexesList;

typedef struct {
    Q3VerticesList vertices;
    Q3IndexesList indexes;
} Q3VerticesPool;



class Q3Face {
    friend class Q3Map;
    
    
protected:
    Q3Shader	m_shader;
	GLuint		m_vboVerticesId;
	GLuint		m_vboIndexesId;
    int			type;
//	std::vector<Q3Vertex> m_vertices;
	//    const Q3VerticesPool& m_verticesPool;    
    //    int m_effectIndex;
    //    int m_lightmapIndex;
    
//    int m_firstVertex;
//    int m_numVertices;
    
//    int m_firstIndex;
    int m_numElements;
    
    
    //    int lightmapX, lightmapY;
    //    int lightmapWidth, lightmapHeight;
    //    Vector3f lightmapOrigin;
    //    Vector3f lightmapVecs[3];
    
public:
  

 

	Q3Face() {}
    virtual ~Q3Face() {
		if (glIsBuffer(m_vboVerticesId) == GL_TRUE){
			glDeleteBuffers(1, &m_vboVerticesId);
		}
		if (glIsBuffer(m_vboIndexesId) == GL_TRUE){
			glDeleteBuffers(1, &m_vboIndexesId);
		}
	}
    
    virtual void render() = 0;
    
    Q3Shader& getShader() { return m_shader; }
    
    int getType() const { return type; }
};

typedef std::vector<std::shared_ptr<Q3Face>> Q3FacesList;



class Q3FacePlanar : public Q3Face {
    
public:
    Q3FacePlanar(){}
    virtual ~Q3FacePlanar() {}
    
    void render();
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
    
    Q3FacePatch(){}
    virtual ~Q3FacePatch() {}
    
    void render() {};
};


class Q3FaceTriangleSoup : public Q3Face {
    
public:
    Q3FaceTriangleSoup() {}
    virtual ~Q3FaceTriangleSoup() {}
    
    void render();
    
};

class Q3FaceFlare : public Q3Face {
    
public:
    
    Q3FaceFlare(){}
    virtual ~Q3FaceFlare() {}
    
    void render() {
        
    }
};
*/

class CQ3Map : public ISceneNode {
    
public:

	CQ3Map() {}
	~CQ3Map() {}
    
    bool load(const char* filename);
    
    void render();
    
	void update(const f64& delta);

private:

	// Data related to the BSP
	enum {
		LUMP_ENTITIES = 0,
		LUMP_SHADERS,
		LUMP_PLANES,
		LUMP_NODES,
		LUMP_LEAVES,
		LUMP_LEAFFACES,
		LUMP_LEAFBRUSHES,
		LUMP_MODELS,
		LUMP_BRUSHES,
		LUMP_BRUSHSIDES,
		LUMP_VERTICES,
		LUMP_MESHINDICES,
		LUMP_FOGS,
		LUMP_FACES,
		LUMP_LIGHTMAPS,
		LUMP_LIGHTVOLS,
		LUMP_VISDATA,
		LUMP_TOTAL,

//		ELUMP_FACESTODRAW,
//		ELUMP_TOTAL
	};

	enum {
		FACE_BAD = 0,
		FACE_PLANAR,
		FACE_PATCH,
		FACE_TRIANGLE_SOUP,
		FACE_FLARE
	};

	enum {
		CLIP_X_LEFT = 1 << 0,
		CLIP_X_RIGHT = 1 << 1,
		CLIP_Y_LEFT = 1 << 2,
		CLIP_Y_RIGHT = 1 << 3,
		CLIP_Z_LEFT = 1 << 4,
		CLIP_Z_RIGHT = 1 << 5,
	};

	class CBspLumpEntry {
	public:
		s32 offset;
		s32 length;
	};

	CMemoryChunk*	m_memoryChunk;
//	CBspLumpEntry	m_extraDataEntries[ELUMP_TOTAL];



	const int Q3BSP_VERSION = 46; // quake 3 maps

	class CBspHeader {
	public:
		s32 magic;
		s32 version;
		CBspLumpEntry entries[LUMP_TOTAL];
	};

	class CBspEntity {
	public:
		u8 * ents;
	};

	class CBspShader {
	public:
		c8 name[64];
		s32 flags;
		s32 contents;
	};

	class CBspPlane {
	public:
		Vector3<f32> normal;
		f32 dist;
	};

	class CBspNode {
	public:
		s32 planeIdx;
		s32 children[2];
		s32 bbox[6];
	};

	class CBspLeaf {
	public:
		s32 cluster;
		s32 area;
		s32 bbox[6];

		s32 firstFaceIdx;
		s32 numFaces;

		s32 firstBrushIdx;
		s32 numBrushes;
	};

	class CBspModel {
	public:
		f32 bbox[6];
		s32 firstFace;
		s32 numFaces;
		s32 firstBrush;
		s32 numBrushes;
	};

	class CBspBrush {
	public:
		s32 firstBrushSide;
		s32 numBrushSides;
		s32 shader;
	};

	class CBspBrushSide {
	public:
		s32 planeIdx;
		s32 shaderIdx;
	};

	class CBspVertex {
	public:
		Vector3f	position;
		Vector2f	texcoord;
		Vector2f	lmcoord;
		Vector3f	normal;
		Vector4<u8>		color;
	};

	class CBspFog {
	public:
		c8 name[64];
		s32 brushIdx;
		s32 visibleSide;
	};


	class CBspFace {
	public:
		s32 shaderIdx;
		s32 effectIdx;
		s32 type;

		s32 firstVertexIdx;
		s32 numVertices;

		s32 firstElementIdx;
		s32 numElements;

		s32 lmIdx;
		s32 lmStart[2];
		s32 lmSize[2];
		f32 lmOrigin[3];
		f32 lmVecs[2][3];

		f32 normal[3];

		s32 size[2];
	};


	class CBspLightMap {
	public:
		u8  map[128][128][3];
	};


	class CBspLightVol {
	public:
		u8  ambient[3];
		u8  directional[3];
		u8  dir[2];
	};


	class CBspVisData {
	public:
		s32 numClusters;
		s32 sizeClusters;
		u8 * bits;
	};

	CBspHeader m_header;

	CBspShader* m_shaders;
	s32 m_numShaders;

	CBspLightMap* m_lightMaps;
	s32 m_numLightMaps;

	CBspVertex* m_vertices;
	s32 m_numVertices;

	CBspFace* m_faces;
	s32		  m_numFaces;

	CBspPlane* m_planes;
	s32 m_numPlanes;

	CBspNode* m_nodes;
	s32 m_numNodes;

	CBspLeaf* m_leaves;
	s32 m_numLeaves;

	s32* m_leafFaceIndices;
	s32 m_numLeafFaceIndices;

	s32* m_leafBrushIndices;
	s32 m_numLeafBrushIndices;

	CBspModel* m_models;
	s32 m_numModels;

	s32* m_meshIndices;
	s32 m_numMeshIndices;

	CBspBrush* m_brushes;
	s32 m_numBrushes;

	CBspBrushSide* m_brushSides;
	s32 m_numBrushSides;

	CBspVisData m_visData;


	bool loadShaders		(const CBspLumpEntry& l); // Load the textures
	bool loadLightmaps		(const CBspLumpEntry& l); // Load the lightmaps
	bool loadVerts			(const CBspLumpEntry& l); // Load the vertices
	bool loadFaces			(const CBspLumpEntry& l); // Load the faces
	bool loadPlanes			(const CBspLumpEntry& l); // Load the Planes of the BSP
	bool loadNodes			(const CBspLumpEntry& l); // load the Nodes of the BSP
	bool loadLeaves			(const CBspLumpEntry& l); // load the Leafs of the BSP
	bool loadLeafFaces		(const CBspLumpEntry& l); // load the Faces of the Leafs of the BSP
	bool loadVisData		(const CBspLumpEntry& l); // load the visibility data of the clusters
	bool loadEntities		(const CBspLumpEntry& l); // load the entities
	bool loadModels			(const CBspLumpEntry& l); // load the models
	bool loadMeshIndices	(const CBspLumpEntry& l); // load the mesh vertices
	bool loadBrushes		(const CBspLumpEntry& l); // load the brushes of the BSP
	bool loadBrushSides		(const CBspLumpEntry& l); // load the brushsides of the BSP
	bool loadLeafBrushes	(const CBspLumpEntry& l); // load the brushes of the leaf
	bool loadFogs			(const CBspLumpEntry& l); // load the shaders
	bool loadExtras			();

	// Additional content
	class CShadedFace {
	public:
		CBspFace * face;
		Q3Shader shader;
		bool toBeDrawn;
	};

	CShadedFace* m_shadedFaces;
	s32 m_numShadedFaces;


	GLuint		vboIds[2];
	s32			m_cameraCluster;
	f32			m_clipMatrix[16];

//	s32*		m_pushedFaces;
//	s32*		m_faceToDrawIndices;
//	s32			m_numFacesToDraw;
//	Q3Shader **  m_faceQ3shaders;

	const CBspLeaf& getLeafFromPosition(const Vector3f& v) const {
		int index = 0;

		while (index >= 0) {
			const CBspNode&		node =	m_nodes[index];
			const CBspPlane&	plane = m_planes[node.planeIdx];
			const double		distance = m_planes[node.planeIdx].normal.dotProduct(v) - plane.dist;

			if (distance >= 0) {
				index = node.children[0];
			}
			else {
				index = node.children[1];
			}
		}

		return m_leaves[-(index + 1)];
	}


	const s32& getCluster(const Vector3f& v) const {
		return this->getLeafFromPosition(v).cluster;
	}


	bool isVisible(const s32& clusterFrom, const s32& clusterTo) const {
		unsigned char bits = 0;

		if (!m_visData.bits || clusterFrom < 0) {
			return true;
		}

		bits = m_visData.bits[(clusterFrom * m_visData.sizeClusters) + (clusterTo >> 3)];

		return (bits & (1 << (clusterTo & 7))) != 0;
	}

	bool clipTest(const s32 bbox[6]) const {
		u32 and_clip = ~0;
		const s32 bbox_index[8][3] =
		{
			{ 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
			{ 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
		};

		for (s32 j = 0; j < 8; ++j)
		{
//			Vector4f v, cv;
			f32 v[4], cv[4];
			u32 flags = 0;

			v[0] = (f32)bbox[bbox_index[j][0]];
			v[1] = (f32)bbox[bbox_index[j][1]];
			v[2] = (f32)bbox[bbox_index[j][2]];
			v[3] = 1.0f;

//			cv[0] = m_clipMatrix[0] * v[0] + m_clipMatrix[4] * v[1] + m_clipMatrix[8] * v[2] + m_clipMatrix[12] * v[3];
//			cv[1] = m_clipMatrix[1] * v[0] + m_clipMatrix[5] * v[1] + m_clipMatrix[9] * v[2] + m_clipMatrix[13] * v[3];
//			cv[2] = m_clipMatrix[2] * v[0] + m_clipMatrix[6] * v[1] + m_clipMatrix[10] * v[2] + m_clipMatrix[14] * v[3];
//			cv[3] = m_clipMatrix[3] * v[0] + m_clipMatrix[7] * v[1] + m_clipMatrix[11] * v[2] + m_clipMatrix[15] * v[3];

			__m128 x = _mm_loadu_ps((const float*)&v);
			__m128 A0 = _mm_loadu_ps((const float*)(m_clipMatrix + 0));
			__m128 A1 = _mm_loadu_ps((const float*)(m_clipMatrix + 4));
			__m128 A2 = _mm_loadu_ps((const float*)(m_clipMatrix + 8));
			__m128 A3 = _mm_loadu_ps((const float*)(m_clipMatrix + 12));

			// Multiply each matrix row with the vector x
			__m128 m0 = _mm_mul_ps(A0, x);
			__m128 m1 = _mm_mul_ps(A1, x);
			__m128 m2 = _mm_mul_ps(A2, x);
			__m128 m3 = _mm_mul_ps(A3, x);

			// Using HADD, we add four floats at a time
			__m128 sum_01 = _mm_hadd_ps(m0, m1);
			__m128 sum_23 = _mm_hadd_ps(m2, m3);
			__m128 result = _mm_hadd_ps(sum_01, sum_23);

			// Finally, store the result
			_mm_storeu_ps((float*)&cv, result);

			//cv = m_clipMatrix * v;

			if (cv[0] < -cv[3])
				flags |= CLIP_X_LEFT;

			if (cv[0] > cv[3])
				flags |= CLIP_X_RIGHT;
			
			if (cv[1] < -cv[3])
				flags |= CLIP_Y_LEFT;

			if (cv[1] > cv[3])
				flags |= CLIP_Y_RIGHT;

			if (cv[2] < -cv[3])
				flags |= CLIP_Z_LEFT;

			if (cv[2] > cv[3])
				flags |= CLIP_Z_RIGHT;

			and_clip &= flags;
		}

		if (and_clip)
			return true;

		return false;
	}


	void updateFacesToDrawIndices(const s32& index) {
		if (index < 0) { // Leaf
			const s32 i = -(index + 1);
			const CBspLeaf& leaf = m_leaves[i];

			// PVS test
			//if (!this->isVisible(m_cameraCluster, leaf.cluster)) {
			//	return;
			//}
			
			// Frustrum culling
			if (this->clipTest(leaf.bbox)) {
				return;
			}
			
			const s32 end = leaf.firstFaceIdx + leaf.numFaces;

			for (s32 j = leaf.firstFaceIdx; j < end; ++j) {
				if (!m_shadedFaces[m_leafFaceIndices[j]].toBeDrawn) {
					m_shadedFaces[m_leafFaceIndices[j]].toBeDrawn = true;
				}
			}
			
		}
		else { // Node 
			const CBspNode& node = m_nodes[index];
			//		 const Q3BspPlane& plane = m_planes[node.plane];
			//		 const Vector3f planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);

			// Frustrum culling
			//if (this->clipTest(node.bbox)) {
			//	return;
			//}		

			this->updateFacesToDrawIndices(node.children[0]);
			this->updateFacesToDrawIndices(node.children[1]);
		}
	}

};


#endif /* defined(__pixelengine__Q3Map__) */
