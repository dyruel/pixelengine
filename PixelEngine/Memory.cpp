//
//  MemoryManager.cpp
//  pixelengine
//
//  Created by Morgan on 15/11/14.
//  Copyright (c) 2014 Morgan Chopin. All rights reserved.
//


//#include <cstdlib>
#include "Memory.h"

#define MEM_ID	0x12345678l

CMemoryChunk* CMemoryManager::getMemory(const u64& size, c8 * label) {
	void* ptr;
	SMemoryBlock* block;

	ptr = std::malloc(size + sizeof(SMemoryBlock));
	if (ptr == nullptr) {
		ILogger::log("MemoryManager :: Unable to allocate the memory block\n");
		return nullptr;
	}
	
	block = (SMemoryBlock*)ptr;
	block->id = MEM_ID;
	block->memChunk.m_ptr = (u8*)ptr + sizeof(SMemoryBlock);
	block->size = size + sizeof(SMemoryBlock);
	block->memChunk.m_size = size;
	block->label = label;

	this->pushMemoryBlock(block);
	m_totalMemorySize += block->size;
	m_numBlocks++;

	return &block->memChunk;
}


void CMemoryManager::freeBlock(void *ptr) {
	SMemoryBlock *block = nullptr;

	block = this->getBlockFromPointer(ptr);

	if (!block) 
		return;

	this->popMemoryBlock(block);
	m_totalMemorySize -= block->size;
	m_numBlocks--;

	std::free(block);
} 


void CMemoryManager::print(void) {
	SMemoryBlock* block;
	int i;
	char buf[2048];

	ILogger::log("====== Memory Usage ======\n");
	ILogger::log("total memory size: %d KB\n", m_totalMemorySize >> 10);
	ILogger::log("total memory blocks: %d\n", m_numBlocks);

	i = 0;
	for (block = m_headBlock; block; block = block->next) {
		ILogger::log("%6d, %p, %8d : %s\n", i, block->memChunk.m_ptr, (int) block->size, block->label);
		i++;
	}
	ILogger::log("============\n");
}


void CMemoryManager::clearMemory(void) {
	SMemoryBlock* block;

	for (block = m_headBlock; block; block = m_headBlock) {
		freeBlock(block->memChunk.m_ptr);
	}

	m_totalMemorySize = 0;
}


void CMemoryManager::pushMemoryBlock(SMemoryBlock* block) {
	block->prev = nullptr;
	block->next = m_headBlock;
	if (m_headBlock) m_headBlock->prev = block;
	m_headBlock = block;
}


void CMemoryManager::popMemoryBlock(SMemoryBlock* block) {
	if (block->prev) block->prev->next = block->next;
	else m_headBlock = block->next;
	if (block->next) block->next->prev = block->prev;
}


CMemoryManager::SMemoryBlock* CMemoryManager::getBlockFromPointer(void *ptr) {
	SMemoryBlock *block = nullptr;

	if (!ptr) {
		return nullptr;
	}

	block = (SMemoryBlock*)((u8*)ptr - sizeof(SMemoryBlock));

	if (block->id != MEM_ID) {
		ILogger::log("MemoryManager :: invalid memory block\n");
	} 

	if (block->memChunk.m_ptr != ptr) {
		ILogger::log("MemoryManager :: memory block pointer invalid\n");
	}

	return block;
}


