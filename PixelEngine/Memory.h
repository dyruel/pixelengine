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

	void* reserve(const u64& size) {
		assert(m_nextFree + size < m_size);
//		std::cout << m_nextFree + size << " " << m_size << std::endl;
		void* p = (char*)m_ptr + m_nextFree;
		m_nextFree += size;
		return p;
	}

private:
	CMemoryChunk()
		: m_ptr(nullptr), m_size(0), m_nextFree(0) {};
	CMemoryChunk(CMemoryChunk&);
	void operator =(CMemoryChunk&);

	void* m_ptr;
	u64	  m_size;
	u64   m_nextFree;
};


class CMemoryManager : public Singleton<CMemoryManager> {
	friend class Singleton<CMemoryManager>;

public:
	~CMemoryManager() {};

	CMemoryChunk* getMemory(const u64& size, c8 * label);

	void freeChunk(CMemoryChunk *ptr);

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
	SMemoryBlock* getBlockFromPointer(CMemoryChunk *ptr);
};

#endif