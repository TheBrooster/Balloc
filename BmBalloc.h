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
	template<const unsigned int BytesPerBlock, const unsigned int BlockCount>
	class Balloc
	{
	private:
		union Block
		{
			uint8_t bytes[BytesPerBlock];
			Block* pNext;
		};

		static_assert(BytesPerBlock >= sizeof(Block*), "BytesPerBlock too small");
		static_assert(BlockCount > 0, "BlockCount too small");

		std::atomic<Block*> mFreeHead;
		std::atomic<bool> mLock;
		std::atomic<unsigned int> mFreeCount;
		unsigned int mLowTideMark;
		std::array<Block, BlockCount> mBlocks;

		void* mBlocksFirst;
		void* mBlocksLast;

		Balloc(const Balloc& other) {} // non construction-copyable
		Balloc& operator=(const Balloc&) {} // non copyable

	public:
		Balloc(void)
		: mLock(false)
		, mFreeCount(BlockCount)
		, mLowTideMark(BlockCount)
		{
			mFreeHead = mBlocks.data();

			// Store a linked list of free blocks in the block storage.
			auto currentBlock = mFreeHead.load();
			auto lastBlock = mFreeHead.load() + (BlockCount - 1);
			do
			{
				currentBlock->pNext = currentBlock + 1;
			}
			while (++currentBlock < lastBlock);
			lastBlock->pNext = nullptr;

			// Store pointers to the first and last blocks for use in the Contains method.
			mBlocksFirst = mFreeHead.load();
			mBlocksLast = lastBlock;
		}

		~Balloc(void)
		{
			if (mFreeCount != BlockCount)
			{ 
				std::fprintf(stderr, "[BM::Balloc] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);
				assert(false);
			}
		}

		inline void* Allocate()
		{
			void* returnPtr = nullptr;

			// Entering critical section...
			bool expected = false;
			while(mLock.compare_exchange_strong(expected, true) && !expected);

			if (mFreeHead.load() != nullptr)
				returnPtr = mFreeHead.exchange(mFreeHead.load()->pNext);

			mLock = false;
			// ...left critical section.

			if (returnPtr != nullptr)
			{
				--mFreeCount;
				if (mFreeCount < mLowTideMark)
					mLowTideMark = mFreeCount;
			}
			else
				std::fprintf(stderr, "[BM::Balloc] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);

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
				blockPtr->pNext = mFreeHead.exchange(blockPtr);
				++mFreeCount;
			}
			return containsPtr;
		}
	};
}
