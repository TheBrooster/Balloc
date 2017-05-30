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
#include <mutex>

namespace BM
{
	template<unsigned int BytesPerBlock, unsigned int NumBlocks>
	class Balloc
	{
		union Block
		{
			uint8_t bytes[BytesPerBlock];
			Block* pNext;
		};

		static_assert(BytesPerBlock >= sizeof(Block*), "BytesPerBlock too small");
		static_assert(NumBlocks > 0, "NumBlocks too small");

	public:
		Balloc(void)
		: mFreeCount(NumBlocks)
		{
			mFreeHead = mBlocks.data();

			// Weave free list into block storage array.
			auto currentBlock = mFreeHead;
			auto lastBlock = mFreeHead + (NumBlocks - 1);
			do
			{
				currentBlock->pNext = currentBlock + 1;
			}
			while (++currentBlock < lastBlock);

			lastBlock->pNext = nullptr;
		}

		~Balloc(void)
		{
			std::lock_guard<std::mutex> l(mMutex);
			if (mFreeCount != NumBlocks)
			{
				std::fprintf(stderr, "[BmBalloc] Not all blocks freed, dangling pointer danger: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, NumBlocks);
				assert(false);
			}
		}

		inline void* Allocate()
		{
			void* returnPtr = nullptr;

			std::lock_guard<std::mutex> l(mMutex);
			if (mFreeCount > 0)
			{
				returnPtr = mFreeHead;
				mFreeHead = mFreeHead->pNext;
				--mFreeCount;
			}
			else
			{
				std::fprintf(stderr, "[BmBalloc] Out of blocks: BytesPerBlock: %d NumBlocks: %d\n", BytesPerBlock, NumBlocks);
			}

			return returnPtr;
		}

		inline bool ContainsBlock(const void* const ptr) const
		{
			auto blockPtr = static_cast<const Block* const>(ptr);
			auto index = blockPtr - mBlocks.data();
			return (index >= 0 && index < NumBlocks);
		}

		inline bool Free(void* ptr)
		{
			bool returnValue = false;
			if (ptr != nullptr)
			{
				if (ContainsBlock(ptr))
				{
					std::lock_guard<std::mutex> l(mMutex);

					auto blockPtr = static_cast<Block*>(ptr);
					blockPtr->pNext = mFreeHead;
					mFreeHead = blockPtr;

					++mFreeCount;
					returnValue = true;
				}
			}
			return returnValue;
		}

	private:

		Block* mFreeHead;
		unsigned int mFreeCount;
		std::mutex mMutex;
		std::array<Block, NumBlocks> mBlocks;
	};
}