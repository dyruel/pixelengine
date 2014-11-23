//
//  Scene.h
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Scene_h
#define PixelEngine_Scene_h


#include <vector>
#include <memory>

#include "Opengl.h"
#include "Types.h"
#include "Physics.h"
#include "Singleton.h"
#include "Camera.h"

class ISceneNode {

public:
	virtual ~ISceneNode() {};

    virtual void render() = 0;
	virtual void update(const f64& delta) = 0;
};


class CInternalSceneNode : public std::vector<ISceneNode*>,  public ISceneNode {

public:
	CInternalSceneNode() {};
	virtual ~CInternalSceneNode() {};
    
    virtual void render();
	virtual void update(const f64& delta);

};


class CDummyNode : public ISceneNode {

public:
	CDummyNode() {};
	~CDummyNode() {};

	void render();
};


class CSceneManager : public Singleton<CSceneManager> {
	friend class Singleton<CSceneManager>;

public:

	virtual ~CSceneManager() {};

	void attachCamera(Camera * camera) { m_camera = camera; }

	void attachRoot(CInternalSceneNode * root) { m_root = root; }

    const Camera* getCamera() const { return m_camera; }

	const CInternalSceneNode* getRoot() const { return m_root; }
    
    void render() const;

	void update(const f64& delta);

private:

	CSceneManager()
		: m_camera(nullptr), m_root(nullptr) {};
	CSceneManager(CSceneManager&);
	void operator =(CSceneManager&);

	Camera* m_camera;

	CInternalSceneNode * m_root;

};

#endif
