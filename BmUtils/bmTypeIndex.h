#ifndef _BM_TYPEINDEX_H_662CF6C0_DD52_4A8B_8A11_1DE065F7692F_
#define _BM_TYPEINDEX_H_662CF6C0_DD52_4A8B_8A11_1DE065F7692F_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <atomic>

namespace bm
{
	class TypeIndex
	{
	public:
		template<typename T>
		static unsigned int Get()
		{
			static unsigned int index = NextIndex();
			return index;
		}

	private:
		static int NextIndex()
		{
			static std::atomic<unsigned int> mIndex;
			return mIndex++;
		}
	};
}

#endif // ifndef _BM_TYPEINDEX_H_662CF6C0_DD52_4A8B_8A11_1DE065F7692F_
