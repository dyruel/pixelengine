//
//  Singleton.h
//  PixelEngine
//
//  Created by morgan on 22/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Singleton_h
#define PixelEngine_Singleton_h

template <class T>
class Singleton {
public:

	static T* getInstance() {
		if (!instance) {
			instance = new T;
			instance->init();
		}
		return instance;
	}


	static void drop() {
		if (instance) {
			instance->deinit();
			delete instance;
			instance = NULL;
		}
	}

protected:

	Singleton() {};
	~Singleton() {};

	virtual bool init() { return true; };
	virtual bool deinit() { return true; };

private:

	static T* instance; 

	Singleton(Singleton&);
	void operator =(Singleton&);

};


template <class T> T* Singleton<T>::instance = NULL;

#endif
