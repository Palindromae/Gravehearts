#pragma once

#include <cstdarg>
#include <stdio.h>
#include <windows.h>

namespace trace {
	enum level : int32_t {
		error,
		warning,
		info,
		debug,
		data
	};

	static void log(trace::level level, const char* function, int32_t line, const char* fmt, ...) {
		std::va_list args;

		char header[256] = { 0 };
		switch (level) {
		case error:
			_snprintf_s(header, _countof(header), _TRUNCATE, "[error  ][%s]:[%4d] ", function, line); break;
		case warning:
			_snprintf_s(header, _countof(header), _TRUNCATE, "[warning][%s]:[%4d] ", function, line); break;
		case info:
			_snprintf_s(header, _countof(header), _TRUNCATE, "[info   ][%s]:[%4d] ", function, line); break;
		case data:
			_snprintf_s(header, _countof(header), _TRUNCATE, "[data   ][%s]:[%4d] ", function, line); break;
		case debug:
			__fallthrough;
		default:
			_snprintf_s(header, _countof(header), _TRUNCATE, "[debug  ][%s]:[%4d] ", function, line);
		}

		char log[1024 - sizeof(header)] = { 0 };
		va_start(args, fmt);
		vsnprintf_s(log, _countof(log), _TRUNCATE, fmt, args);
		va_end(args);
		if (level != data) {
			if (log[strlen(log) - 1] != '\n') {
				log[_countof(log) - 2] = 0;
				strncat_s(log, _countof(log), "\n", _TRUNCATE);
			}
		}

		OutputDebugStringA(header);
		OutputDebugStringA(log);
	}
}
#if defined(_DEBUG) || defined(RELEASE_INSTRUMENTED)
#define trace_error(fmt, ...)	trace::log(trace::level::error, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define trace_warning(fmt, ...) trace::log(trace::level::warning, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define trace_info(fmt, ...)	trace::log(trace::level::info, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define trace_debug(fmt, ...)	trace::log(trace::level::debug, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define trace_data(fmt, ...)	trace::log(trace::level::data, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#else
#define trace_error
#define trace_warning
#define trace_info
#define trace_debug
#define trace_data
#endif