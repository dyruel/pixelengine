//
//  Q3Level.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


// Doc: http://www.mralligator.com/q3/

#ifndef PixelEngine_Q3Level_h
#define PixelEngine_Q3Level_h


#include "Q3Map.h"
#include "Scene.h"






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
*/

/*
class Q3Level : public SceneNode {
    


    Q3Map m_map;

    bool _loadMap(const char* filename);
    
public:
    Q3Level() {};
    virtual ~Q3Level() {};

	bool load(const char* filename);
    
    void render();
    void update(double delta);
 
};
*/

#endif