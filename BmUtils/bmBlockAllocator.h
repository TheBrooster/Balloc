#pragma once

// Copyright Bruce McNeish 2017
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include "bmAtomicSpinLock.h"

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <array>

namespace BM
{
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

		static_assert(BytesPerBlock >= sizeof(Block*), "BytesPerBlock too small.  Needs to be at least the size of a pointer.");
		static_assert(BlockCount > 0, "BlockCount cannot be 0.  Suggest 1024 minimum.");

		Block* mFreeHead;
		AtomicSpinLock mLock;
		unsigned int mFreeCount;
		unsigned int mLowTideMark;
		std::array<Block, BlockCount> mBlocks;

		void* mBlocksFirst;
		void* mBlocksLast;

	public:
		BlockAllocator(void)
			: mFreeCount(BlockCount)
			, mLowTideMark(BlockCount)
		{
			mFreeHead = mBlocks.data();

			// Store a linked list of free blocks in the block storage.
			for (auto& block : mBlocks)
			{
				block.pNext = &block + 1;
			}
			mBlocks[BlockCount - 1].pNext = nullptr;

			// Cache first and last void pointers for use in contains member function.
			mBlocksFirst = mFreeHead;
			mBlocksLast = mFreeHead + (BlockCount - 1);
		}

		~BlockAllocator(void)
		{
			if (mFreeCount != BlockCount)
			{
				std::fprintf(stderr, "[BM::BlockAllocator] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);
				assert(false);
			}
		}

		inline bool contains(const void* const ptr) const
		{
			return (ptr >= mBlocksFirst) && (ptr <= mBlocksLast);
		}

		inline void* allocate()
		{
			void* returnPtr = nullptr;

			mLock.lock();

			if (mFreeHead != nullptr)
			{
				returnPtr = mFreeHead;
				mFreeHead = mFreeHead->pNext;
				--mFreeCount;

				if (mFreeCount < mLowTideMark)
					mLowTideMark = mFreeCount;
			}

			mLock.unlock();

			if (returnPtr == nullptr)
				std::fprintf(stderr, "[BM::BlockAllocator] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, BlockCount);

			return returnPtr;
		}

		inline bool free(void* ptr)
		{
			bool containsPtr = contains(ptr);
			if (containsPtr)
			{
				auto blockPtr = static_cast<Block*>(ptr);

				mLock.lock();

				blockPtr->pNext = mFreeHead;
				mFreeHead = blockPtr;
				++mFreeCount;

				mLock.unlock();
			}
			return containsPtr;
		}
	};
}
