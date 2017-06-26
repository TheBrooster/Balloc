#ifndef _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_
#define _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include "bmAtomicSpinLock.h"
#include "bmDebugBreak.h"

#include <cstdint>
#include <cstdio>
#include <array>

#ifdef DEBUG
# define TRACK_USAGE
#endif

namespace bm
{
	template<const unsigned int BlockSize, const unsigned int BlockCount>
	class BlockAllocator
	{
	private:
		BlockAllocator(const BlockAllocator& other) {} // non construction-copyable
		BlockAllocator& operator=(const BlockAllocator&) { return *this; } // non copyable

		union Block
		{
			uint8_t bytes[BlockSize];
			Block* pNext;
		};

		static_assert(BlockSize >= sizeof(Block*), "BlockSize too small. Needs to be at least the size of a pointer.");
		static_assert(BlockCount > 0, "BlockCount cannot be 0.  Suggest 1024 minimum.");

		std::array<Block, BlockCount> mBlocks;
		std::array<bool, BlockCount> mAlreadyFreed;
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
		BlockAllocator(void)
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

		~BlockAllocator(void)
		{
#ifdef TRACK_USAGE
			if (mFreeCount != BlockCount)
			{
				DebugBreak(true, "[bm::BlockAllocator] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BlockSize, BlockCount);
			}
#endif
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
                
				auto index = static_cast<Block*>(returnPtr) - mBlocks.data();
				mAlreadyFreed[index] = false;
                
#ifdef TRACK_USAGE
				--mFreeCount;
				++mTotalAllocationCount;
				if (mFreeCount < mLowTideMark)
					mLowTideMark = mFreeCount;
#endif
			}

			mLock.unlock();

			if (returnPtr == nullptr && mReportedOutOfBlocks == false)
			{
				std::fprintf(stderr, "[bm::BlockAllocator] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BlockSize, BlockCount);
				mReportedOutOfBlocks = true;
			}

			return returnPtr;
		}

		inline bool free(void* ptr)
		{
			bool containsPtr = contains(ptr);
			if (containsPtr)
			{
				mLock.lock();

				auto blockPtr = static_cast<Block*>(ptr);
				auto index = blockPtr - mBlocks.data();
				if (mAlreadyFreed[index] == false)
				{
					mAlreadyFreed[index] = true;

					blockPtr->pNext = mFreeHead;
					mFreeHead = blockPtr;
#ifdef TRACK_USAGE
					++mFreeCount;
#endif
				}
                
				mLock.unlock();
			}
			return containsPtr;
		}

		inline void reportUsage() const
		{
#ifdef TRACK_USAGE
			std::fprintf(stdout, "[bm::BlockAllocator] BlockSize: %d Free: %d/%d Total Allocation Count: %ld\n", BlockSize, mFreeCount, BlockCount, mTotalAllocationCount);
#endif
		}
	};
}

#endif //ifndef _BM_BLOCKALLOCATOR_H_BA69E10F_97F3_4AA7_9751_BE68CD182529_
