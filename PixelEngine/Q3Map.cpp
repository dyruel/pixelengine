//
//  Q3Map.cpp
//  pixelengine
//
//  Created by Morgan on 15/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#include "Q3Map.h"


int Q3Map::bbox_index[8][3] =
{
    { 0, 1, 2 }, { 3, 1, 2 }, { 3, 4, 2 }, { 0, 4, 2 },
    { 0, 1, 5 }, { 3, 1, 5 }, { 3, 4, 5 }, { 0, 4, 5 }
};


inline void Q3FacePlanar::render() {
    m_shader.begin();
    
    std::vector<Q3ShaderPass>& shaderPasses = m_shader.getShaderPasses();
    std::vector<Q3ShaderPass>::iterator shaderPasse = shaderPasses.begin();
    std::vector<Q3ShaderPass>::iterator shaderPassesEnd = shaderPasses.end();
    
    
    
    
    glVertexPointer(3, GL_FLOAT, sizeof(Q3Vertex), m_verticesPool.vertices.data() + m_firstVertex);
    
    while (shaderPasse != shaderPassesEnd) {
        
        //shaderPasse->begin();
        
        //m_verticesPool.indexes[m_firstIndex]
        
        glDrawElements(GL_TRIANGLES, m_numIndexes, GL_UNSIGNED_INT, m_verticesPool.indexes.data() + m_firstIndex);
        
        //shaderPasse->end();
        
        ++shaderPasse;
    }
    
    m_shader.end();
}




/**
 *
 * Return true if the node can be safely clipped
 */
inline bool Q3Map::_clipTest(const Node& node) const {
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


inline void Q3Map::_pushFaces(int index) {
    
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


inline void Q3Map::update(double delta) {
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


inline void Q3Map::render() {
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
