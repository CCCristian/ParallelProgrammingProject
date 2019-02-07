#pragma once

#include <iostream>
#include <sstream>
#include <winbase.h>

template<typename Last> void _log_(const Last& last)
{
	std::cout << last << '\n';
}

template<typename First, typename ... Rest> void _log_(const First& first, const Rest& ... args)
{
	std::cout << first;
	_log_(args...);
}

template<typename Last> void _log_tostring(std::stringstream& output, const Last& last)
{
	output << last << '\n';
}

template<typename First, typename ... Rest> void _log_tostring(std::stringstream& output, const First& first, const Rest& ... args)
{
	output << first;
	_log_tostring(output, args...);
}

template<typename ... Args> void _log(bool condition, const char* file, const char* function, int line, Args ... args)
{
	if (!condition)
	{
		std::stringstream outputStream;
		_log_tostring(outputStream, args...);
		std::string output = outputStream.str();
		OutputDebugStringA(output.c_str());

		std::cerr << function << '\n';
		std::cerr << "\tMesajul  : " << output;
		std::cerr << "\n\tFisierul : " << file;
		std::cerr << "\n\tLinia    : " << line << '\n';
	}
}

#ifdef _DEBUG
#define assert(condition, ...)\
{\
	auto _cond_ = condition; \
	_log(_cond_, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
	_ASSERT_EXPR(_cond_, L#condition);\
}
#else
#define assert(condition, ...)
#endif

#define exitWithError(...)\
{\
	_log(false, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
	std::cin.get();\
	exit(1);\
}
