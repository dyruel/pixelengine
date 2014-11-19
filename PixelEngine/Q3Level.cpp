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


bool Q3Level::_loadMap(const char* filename) {
    return m_map.load(filename);
}


bool Q3Level::load(const char* filename) {
    bool r = true;
    
    r = this->_loadMap(filename);
    
    return r;
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





void Q3Level::render() {
//    std::cout << "qze" << std::endl;
    m_map.render();

}


void Q3Level::update(GLdouble delta) {
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