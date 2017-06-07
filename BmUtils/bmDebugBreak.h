#ifndef _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_
#define _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <cstdarg>
#include <cstdio>

#ifdef _MSC_VER
# define BREAKPOINT __debugbreak()
#else
# define BREAKPOINT asm("int $3")
#endif

namespace bm
{
	class DebugBreak
	{
	private:
		DebugBreak(const DebugBreak& other) {} // non construction-copyable
		DebugBreak& operator=(const DebugBreak&) { return *this; } // non copyable

	public:
		DebugBreak(void)
		{
			BREAKPOINT;
		}

		DebugBreak(bool statement, const char* fmt, ...)
		{
			if (statement)
			{
				va_list ap;

				va_start(ap, fmt);
				std::fprintf(stderr, fmt, ap);
				va_end(ap);

				BREAKPOINT;
			}
		}
	};
}

#endif //ifndef _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_
