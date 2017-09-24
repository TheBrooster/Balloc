#ifndef _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_
#define _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include "bmAtomicSpinLock.h"
#include "bmAssert.h"

#include <cstdint>
#include <cstdio>
#include <array>
#include <bitset>
#include <mutex>
#include <algorithm>

#ifdef DEBUG
# define TRACK_USAGE
#endif

namespace bm
{
	template<const unsigned int BlockSize, const unsigned int BlockCount>
	class PoolAllocator
	{
	private:
		PoolAllocator(const PoolAllocator& other) = delete;
		PoolAllocator& operator=(const PoolAllocator&) = delete;

		union Block
		{
			uint8_t bytes[BlockSize];
			Block* pNext;
		};

		static_assert(BlockSize >= sizeof(Block*), "BlockSize too small. Needs to be at least the size of a pointer.");
		static_assert(BlockCount > 0, "BlockCount cannot be 0.  Suggest 1024 minimum.");

		std::array<Block, BlockCount> mBlocks;
		std::bitset<BlockCount> mInUse;
		AtomicSpinLock mLock;

		Block* mFreeHead;
		void* mBlocksFirst;
		void* mBlocksLast;

		bool mReportedOutOfBlocks;

#ifdef TRACK_USAGE
		unsigned int mFreeCount;
		unsigned int mLowTideMark;
		uint64_t mTotalAllocationCount;
#endif

	public:
		PoolAllocator(void)
			: mReportedOutOfBlocks(false)
#ifdef TRACK_USAGE
			, mFreeCount(BlockCount)
			, mLowTideMark(BlockCount)
			, mTotalAllocationCount(0)
#endif
		{
			mFreeHead = mBlocks.data();

			// Ensure O(1) allocation by using free blocks to store a linked list.
			for (auto& block : mBlocks)
				block.pNext = &block + 1;

			mBlocks[BlockCount - 1].pNext = nullptr;

			// Cache first and last void pointers for use in contains member function.
			mBlocksFirst = mFreeHead;
			mBlocksLast = mFreeHead + (BlockCount - 1);
		}

		~PoolAllocator(void)
		{
#ifdef TRACK_USAGE
			bm::Assert(mFreeCount == BlockCount, "[bm::PoolAllocator] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BlockSize, BlockCount);
#endif
		}

		inline bool contains(const void* const ptr) const
		{
			return (ptr >= mBlocksFirst) && (ptr <= mBlocksLast);
		}

		inline void* allocate_lock_free()
		{
			return nullptr;
		}

		inline void release_lock_free(void* ptr)
		{
		}

		inline void* allocate()
		{
			void* returnPtr = nullptr;

			std::lock_guard<bm::AtomicSpinLock> l(mLock);
			if (mFreeHead != nullptr)
			{
				returnPtr = mFreeHead;
				mFreeHead = mFreeHead->pNext;

				auto index = static_cast<Block*>(returnPtr) - mBlocks.data();
				mInUse[index] = true;

#ifdef TRACK_USAGE
				--mFreeCount;
				++mTotalAllocationCount;
				mLowTideMark = std::min(mFreeCount, mLowTideMark);
#endif
			}

			if (returnPtr == nullptr && mReportedOutOfBlocks == false)
			{
				std::fprintf(stderr, "[bm::PoolAllocator] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BlockSize, BlockCount);
				mReportedOutOfBlocks = true;
			}

			return returnPtr;
		}

		inline bool free(void* ptr)
		{
			bool containsPtr = contains(ptr);
			if (containsPtr)
			{
				Block* blockPtr = static_cast<Block*>(ptr);
				size_t index = blockPtr - mBlocks.data();

				std::lock_guard<bm::AtomicSpinLock> l(mLock);
				if (mInUse[index])
				{
					mInUse[index] = false;

					blockPtr->pNext = mFreeHead;
					mFreeHead = blockPtr;
#ifdef TRACK_USAGE
					++mFreeCount;
#endif
				}
				else
				{
					std::fprintf(stderr, "[bm::PoolAllocator] Double free attempt detected. BlockSize: %d\n", BlockSize);
				}
			}
			return containsPtr;
		}

		inline void reportUsage() const
		{
#ifdef TRACK_USAGE
			std::fprintf(stdout, "[bm::PoolAllocator] BlockSize: %d Free: %d/%d Total Allocation Count: %ld\n", BlockSize, mFreeCount, BlockCount, mTotalAllocationCount);
#endif
		}
	};
}

#endif //ifndef _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_
