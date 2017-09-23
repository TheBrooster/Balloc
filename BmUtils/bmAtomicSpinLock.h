#ifndef _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_
#define _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <atomic>

namespace bm
{
	class AtomicSpinLock
	{
	public:
		AtomicSpinLock() = default;
		AtomicSpinLock(const AtomicSpinLock& other) = delete;
		AtomicSpinLock& operator=(const AtomicSpinLock&) = delete;

		inline void lock()
		{
			while (mLocked.exchange(true));
		}

		inline void unlock()
		{
			mLocked = false;
		}

	private:
		std::atomic<bool> mLocked{ false };
	};
}

#endif //ifndef _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_
