#ifndef _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_
#define _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_

// Copyright (c) 2017 Bruce McNeish 
// Distributed under the MIT License, see accompanying file LICENSE.txt

#include <cstdarg>
#include <cstdio>

namespace bm
{
	static void Assert(bool statement, const char* fmt, ...)
	{
		if (!statement)
		{
			va_list ap;

			va_start(ap, fmt);
			std::fprintf(stderr, fmt, ap);
			va_end(ap);

#if defined(_DURANGO) || defined(__ORBIS__) || defined(_MSC_VER)
			__debugbreak();
#elif defined(__APPLE__)
			__asm__("int $3");
#else
# error UNDEFINED PLATFORM
#endif
		}
	}
}

#endif //ifndef _BM_DEBUGBREAK_H_F74CF32F_EF5F_451E_8EE3_6A400AC4044B_
