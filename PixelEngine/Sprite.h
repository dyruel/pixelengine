//
//  Sprite.h
//  pixelengine
//
//  Created by Morgan on 08/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __pixelengine__Sprite__
#define __pixelengine__Sprite__

#include <vector>

#include "Singleton.h"
#include "Scene.h"

class Sprite : public SceneNode {
    
};

typedef std::vector<Sprite> SpriteList;


class SpriteManager : public Singleton<SpriteManager> {
    friend class Singleton<SpriteManager>;
    
private:
    
    SpriteList m_sprites;
    
    bool init();
    bool deinit();
    
    SpriteManager() {};
    SpriteManager(SpriteManager&);
    void operator =(SpriteManager&);
    
public:
    
    virtual ~SpriteManager() {};
    
    bool addSprite(Sprite& sprite) { m_sprites.push_back(sprite); return true;};

};



#endif /* defined(__pixelengine__Sprite__) */
