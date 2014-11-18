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


#include "Vector.h"
#include "Q3Shader.h"
#include "Scene.h"



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
typedef std::vector<int>      Q3IndexesList;

typedef struct {
    Q3VerticesList vertices;
    Q3IndexesList indexes;
} Q3VerticesPool;



class Q3Face {
    friend class Q3Map;
    
    
protected:
    Q3Shader m_shader;
    const Q3VerticesPool& m_verticesPool;
    int type;
    
    //    int m_effectIndex;
    //    int m_lightmapIndex;
    
    int m_firstVertex;
    int m_numVertices;
    
    int m_firstIndex;
    int m_numIndexes;
    
    
    //    int lightmapX, lightmapY;
    //    int lightmapWidth, lightmapHeight;
    //    Vector3f lightmapOrigin;
    //    Vector3f lightmapVecs[3];
    
public:
    
    
    enum {
        FACE_BAD = 0,
        FACE_PLANAR,
        FACE_PATCH,
        FACE_TRIANGLE_SOUP,
        FACE_FLARE
    };
    
    
    Q3Face(const Q3VerticesPool& verticesPool)
    :m_verticesPool(verticesPool){}
    virtual ~Q3Face() {}
    
    virtual void render() = 0;
    
    Q3Shader& getShader() { return m_shader; }
    
    int getType() const { return type; }
};

typedef std::vector<std::shared_ptr<Q3Face>> Q3FacesList;



class Q3FacePlanar : public Q3Face {
    
public:
    Q3FacePlanar(const Q3VerticesPool& verticesPool)
    :Q3Face(verticesPool){}
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
    
    Q3FacePatch(const Q3VerticesPool& verticesPool)
    :Q3Face(verticesPool){}
    virtual ~Q3FacePatch() {}
    
    void render() {};
};


class Q3FaceTriangleSoup : public Q3Face {
    
public:
    Q3FaceTriangleSoup(const Q3VerticesPool& verticesPool)
    :Q3Face(verticesPool){}
    virtual ~Q3FaceTriangleSoup() {}
    
    void render();
    
};

class Q3FaceFlare : public Q3Face {
    
public:
    
    Q3FaceFlare(const Q3VerticesPool& verticesPool)
    :Q3Face(verticesPool){}
    virtual ~Q3FaceFlare() {}
    
    void render() {
        
    }
};


class Q3Map : public SceneNode {
    
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
        CLIP_X_LEFT		= 1 << 0,
        CLIP_X_RIGHT	= 1 << 1,
        CLIP_Y_LEFT		= 1 << 2,
        CLIP_Y_RIGHT	= 1 << 3,
        CLIP_Z_LEFT		= 1 << 4,
        CLIP_Z_RIGHT	= 1 << 5,
    };
    
    struct BspLumpEntry {
        int offset;
        int length;
    };
    
    struct BspHeader  {
        char magic[4];
        int version;
        BspLumpEntry entries[LUMP_TOTAL];
    };
    
    struct VisData {
        int numClusters;
        int sizeCluster;
        std::unique_ptr<unsigned char[]> bits;
    };
    
    struct Node {
        int bbox[6];
    };
    
    struct Plane {
        Vector3f normal;
        float dist;
    };
    
    struct InternalNode : public Node {
        Plane plane;
        int children[2];
    };
    
    struct LeafNode : public Node {
        int cluster;
        int area;
        
        int firstLeafFace;
        int numLeafFaces;
        
        int firstLeafBrush;
        int numLeafBrushes;
    };
    
    typedef std::vector<std::shared_ptr<InternalNode>>      InternalNodesList;
    typedef std::vector<std::shared_ptr<LeafNode>>          LeafNodesList;
    typedef int                                             ClusterIndex;
    
    static int bbox_index[8][3];
    
    Q3VerticesPool      m_verticesPool;
    Q3FacesList         m_faces;
    InternalNodesList   m_internalNodes;
    LeafNodesList       m_leafNodes;
    Q3IndexesList       m_leafFaceIndexes;
    Q3IndexesList       m_leafBrushIndexes;
    VisData             m_visData;
    
    
    
    std::set<int>       m_facesToRender;
    void                _pushFaces(int index);
    
    Matrix4f            m_clipMatrix;
    bool                _clipTest(const Node& node) const;
    
    
    bool _loadVertices(FILE * file,
                       const BspLumpEntry& verticesLump,
                       const BspLumpEntry& indexesLump);
    
    bool _loadFaces(FILE * file,
                    const BspLumpEntry& facesLump,
                    const BspLumpEntry& shadersLump,
                    const BspLumpEntry& lightmapsLump);
    
    bool _loadBspTree(FILE * file,
                      const BspLumpEntry& nodesLump,
                      const BspLumpEntry& leafLump,
                      const BspLumpEntry& planesLump,
                      const BspLumpEntry& leafFaceLump,
                      const BspLumpEntry& leafBrushLump,
                      const BspLumpEntry& visDataLump);
    
public:
    
    const LeafNode& getLeafFromPosition(const Vector3f& v) const {
        int index = 0;
        
        while (index >= 0) {
            std::shared_ptr<InternalNode> node = m_internalNodes[index];
            const double distance = node->plane.normal.dotProduct(v) - node->plane.dist;
            
            if (distance >= 0) {
                index = node->children[0];
            }
            else {
                index = node->children[1];
            }
        }
        
        return *m_leafNodes[-(index + 1)];
    }
    
    ClusterIndex getClusterIndex(const Vector3f& v) const {
        return this->getLeafFromPosition(v).cluster;
    }
    
    
    bool isVisible(ClusterIndex clusterFrom, ClusterIndex clusterTo) const {
        unsigned char bits = 0;
        
        if (!m_visData.bits || clusterFrom < 0) {
            return true;
        }
        
        bits = m_visData.bits[(clusterFrom * m_visData.sizeCluster) + (clusterTo >> 3)];
        
        return (bits & (1 << (clusterTo & 7))) != 0;
    }
    
    
    bool load(const char* filename);
    bool load(FILE * file);
    
    void render();
    
    void update(double delta);
    
};


#endif /* defined(__pixelengine__Q3Map__) */
