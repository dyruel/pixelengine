//
//  Physics.h
//  PixelEngine
//
//  Created by morgan on 21/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef __PixelEngine__Video__
#define __PixelEngine__Video__

#include "Scene.h"
#include "Singleton.h"

class IPhysical {

};


class PhysicsManager : public Singleton < PhysicsManager > {
	friend class Singleton < PhysicsManager >;

private:

	PhysicsManager();
	PhysicsManager(PhysicsManager&);
	void operator =(PhysicsManager&);

public:

	virtual ~PhysicsManager();


};



#endif /* defined(__PixelEngine__Video__) */