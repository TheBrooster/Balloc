#ifndef _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_
#define _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <atomic>

namespace bm
{
	class AtomicSpinLock
	{
	private:
		AtomicSpinLock(const AtomicSpinLock& other) {} // non construction-copyable
		AtomicSpinLock& operator=(const AtomicSpinLock&) { return *this; } // non copyable

		std::atomic<bool> mLocked;

	public:
		AtomicSpinLock()
			: mLocked(false)
		{}

		~AtomicSpinLock()
		{
			unlock();
		}

		inline void lock()
		{
			bool expected = false;
			while (!mLocked.compare_exchange_weak(expected, true))
			{
				expected = false;
			}
		}

		inline void unlock()
		{
			mLocked.store(false);
		}
	};
}

#endif //ifndef _BM_ATOMICSPINLOCK_H_8E88D833_8211_4E6B_AD64_A5E845819D9E_
