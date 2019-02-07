#pragma once

#include <iostream>

namespace Communication
{
	template<typename Last> void _log(const Last& last)
	{
		std::cout << last;
	}

	template<typename First, typename ... Rest> void _log(const First& first, const Rest& ... args)
	{
		std::cout << first;
		_log(args...);
	}

	template<typename ... Args> void _assert(bool condition, const char* file, const char* function, int line, Args ... args)
	{
		if (!condition)
		{
			std::cerr << function << '\n';
			std::cerr << "\tMesajul  : ";
			_log(args...);
			std::cerr << "\n\tFisierul : " << file << '\n';
			std::cerr << "\tLinia    : " << line << '\n';
			//abort();
		}
	}

#ifdef _DEBUG
#define assert(condition, ...)\
{\
	_assert(condition, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
	_ASSERTE(condition);\
}
#else
#define assert(condition, ...)
#endif
}
