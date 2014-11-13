//
//  Scene.h
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Scene_h
#define PixelEngine_Scene_h

#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

#include "Physics.h"
#include "Singleton.h"
#include "Camera.h"

class ISceneNode {
public:
	virtual ~ISceneNode() {};

    virtual void render() = 0;
    virtual void update(GLdouble delta) = 0;
};

typedef std::vector<std::shared_ptr<ISceneNode>> SceneNodeList;

// SceneNode
class SceneNode : public ISceneNode {

protected:

	SceneNodeList m_children;

public:
	SceneNode() {};
	virtual ~SceneNode() {};
    
    virtual void render();
    virtual void update(GLdouble delta);
    
    void addChild(std::shared_ptr<ISceneNode> child) {
        m_children.push_back(child);
    }
};


// SkyNode
class SkyNode : public SceneNode {
public:

	SkyNode() {};
	~SkyNode() {};
    
	void render();
};



// DummyNode
class DummyNode : public SceneNode {
public:
	DummyNode() {};
	~DummyNode() {};

	void render();
};


// SceneManager
class SceneManager : public Singleton<SceneManager> {
	friend class Singleton<SceneManager>;
    

private:
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<SceneNode> m_root;
    
	bool init();
	bool deinit();

	SceneManager() {};
	SceneManager(SceneManager&);
	void operator =(SceneManager&);

public:

	virtual ~SceneManager() {};

    void attachCamera(std::shared_ptr<Camera> camera) {
        m_camera = camera;
    }
    
    const std::shared_ptr<Camera>& getCamera() const {
        return m_camera;
    }
    
    void addSceneNode(std::shared_ptr<ISceneNode> node) {
        m_root->addChild(node);
    }
    
    void render() const;
	void update(GLdouble delta);
};

#endif
