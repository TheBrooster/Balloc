/**
* Very Simple Block Allocator
*
* Copyright (c) Bruce McNeish
* All rights reserved.
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the "Software"), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
* to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <array>
#include <atomic>

namespace BM
{
	class AtomicLock
	{
	private:
		AtomicLock(const AtomicLock& other)	{} // non construction-copyable
		AtomicLock& operator=(const AtomicLock&) { return *this; } // non copyable

		std::atomic<bool>* mLock;

	public:
		AtomicLock(std::atomic<bool>* pLock)
		: mLock(pLock)
		{
			bool expected = false;
			while (mLock->compare_exchange_weak(expected, true) && !expected);
		}
		~AtomicLock()
		{
			mLock->store(false);
		}
	};

	template<const unsigned int BytesPerBlock, const unsigned int BlockCount>
	class BlockAllocator
	{
	private:
		BlockAllocator(const BlockAllocator& other) {} // non construction-copyable
		BlockAllocator& operator=(const BlockAllocator&) { return *this; } // non copyable

		union Block
		{
			uint8_t bytes[BytesPerBlock];
			Block* pNext;
		};

		static_assert(BytesPerBlock >= sizeof(Block*), "BytesPerBlock too small");
		static_assert(BlockCount > 0, "BlockCount too small");

		Block* mFreeHead;
		std::atomic<bool> mLock;
		unsigned int mFreeCount;
		unsigned int mLowTideMark;
		std::array<Block, BlockCount> mBlocks;

		void* mBlocksFirst;
		void* mBlocksLast;

	public:
		BlockAllocator(void)
			: mLock(false)
			, mFreeCount(BlockCount)
			, mLowTideMark(BlockCount)
		{
			mFreeHead = mBlocks.data();

			// Store a linked list of free blocks in the block storage.
			auto currentBlock = mFreeHead;
			auto lastBlock = mFreeHead + (BlockCount - 1);
			do
			{
				currentBlock->pNext = currentBlock + 1;
			}
			while (++currentBlock < lastBlock);
			lastBlock->pNext = nullptr;

			// Store pointers to the first and last blocks for use in the Contains method.
			mBlocksFirst = mFreeHead;
			mBlocksLast = lastBlock;
		}

		~BlockAllocator(void)
		{
			if (mFreeCount != BlockCount)
			{
				std::fprintf(stderr, "[BM::BlockAllocator] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);
				assert(false);
			}
		}

		inline void* Allocate()
		{
			void* returnPtr = nullptr;

			// Entering critical section...
			//
			{
				AtomicLock l(&mLock);
				if (mFreeHead != nullptr)
				{
					returnPtr = mFreeHead;
					mFreeHead = mFreeHead->pNext;
					--mFreeCount;
				}
			}
			//
			// ...left critical section.

			if (returnPtr != nullptr)
			{
				if (mFreeCount < mLowTideMark)
					mLowTideMark = mFreeCount;
			}
			else
				std::fprintf(stderr, "[BM::BlockAllocator] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);

			return returnPtr;
		}

		inline bool Contains(const void* const ptr) const
		{
			return (ptr >= mBlocksFirst) && (ptr <= mBlocksLast);
		}

		inline bool Free(void* ptr)
		{
			bool containsPtr = Contains(ptr);
			if (containsPtr)
			{
				auto blockPtr = static_cast<Block*>(ptr);
				// Entering critical section...
				//
				{
					AtomicLock l(&mLock);
					blockPtr->pNext = mFreeHead;
					mFreeHead = blockPtr;
					++mFreeCount;
				}
				//
				// ...left critical section.
			}
			return containsPtr;
		}
	};
}
