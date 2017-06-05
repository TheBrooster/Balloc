#pragma once

// Copyright Bruce McNeish 2017
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <atomic>

namespace BM
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
