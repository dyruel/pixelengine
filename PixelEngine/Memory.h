//
//  Memory.h
//  PixelEngine
//
//  Created by morgan on 23/10/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//

#ifndef PixelEngine_Memory_h
#define PixelEngine_Memory_h

#include <cassert>
#include "Singleton.h"
#include "Types.h"
#include "Logger.h"


class CMemoryChunk {
	friend class CMemoryManager;

public:
	~CMemoryChunk() {};
	
	void* operator [] (const u64& i) {
		assert(i >= 0 && i < m_size);
		return (char*)m_ptr + i;
	}
	
private:
	CMemoryChunk()
		: m_ptr(nullptr) {};
	CMemoryChunk(CMemoryChunk&);
	void operator =(CMemoryChunk&);

	void* m_ptr;
	u64	  m_size;
};


class CMemoryManager : public Singleton<CMemoryManager> {
	friend class Singleton<CMemoryManager>;

public:
	~CMemoryManager() {};

	CMemoryChunk* getMemory(const u64& size, c8 * label);

	void freeBlock(void *ptr);

	void print(void);

	void clearMemory(void);

private:
	CMemoryManager()
		:m_headBlock(nullptr), m_numBlocks(0), m_totalMemorySize(0) {};
	CMemoryManager(CMemoryManager&);
	void operator =(CMemoryManager&);

	typedef struct _SMemoryBlock
	{
		u64				id;
		CMemoryChunk	memChunk;
		u64				size;
		c8*				label;
		struct _SMemoryBlock *prev, *next;
	} SMemoryBlock;

	SMemoryBlock* m_headBlock;
	s32 m_numBlocks;

	s32 m_totalMemorySize;

	void pushMemoryBlock(SMemoryBlock* block);
	void popMemoryBlock(SMemoryBlock* block);
	SMemoryBlock* getBlockFromPointer(void *ptr);
};

#endif