//
//  Singleton.h
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Singleton_h
#define PixelEngine_Singleton_h

#include <memory>

template <class T>
class Singleton {

public:

	static std::shared_ptr<T> getInstance() {
		if (!instance) {
            instance.reset(new T);
			instance->init();
		}
		return instance;
	}

protected:

	Singleton() {};
	~Singleton() {};

	virtual bool init() { return true; };
//	virtual bool deinit() { return true; };

private:

	static std::shared_ptr<T> instance;
	Singleton(Singleton const&);
	void operator =(Singleton const&);

};


template <class T> std::shared_ptr<T> Singleton<T>::instance = NULL;

#endif
