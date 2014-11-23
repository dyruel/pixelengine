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

	static T* getInstance() {
		if (!instance) {
            instance = new T();
			if (instance && !instance->init()) {
				ILogger::log("Singleton creation error.");
				delete instance;
				instance = NULL;
			}
		}

		return instance;
	}

protected:

	Singleton() {};
	virtual ~Singleton() {
		if (instance != NULL) {
			delete instance;
		}
	};

	virtual bool init() { return true; };
//	virtual bool deinit() { return true; };

private:

	static T* instance;
	Singleton(Singleton const&);
	void operator =(Singleton const&);

};


template <class T> T* Singleton<T>::instance = NULL;

#endif
