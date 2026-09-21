#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

namespace Log {

static Level sLevel = Level::All;

void SetLevel(Level _level)
{
	sLevel = _level;
}

Level GetLevel()
{
	return sLevel;
}

static void Write(Level _min, const wchar_t* _format, va_list _args)
{
	if (sLevel < _min)
		return;
	vwprintf(_format, _args);
}

void Error(const wchar_t* _format, ...)
{
	va_list args;
	va_start(args, _format);
	Write(Level::Errors, _format, args);
	va_end(args);
}

void Warn(const wchar_t* _format, ...)
{
	va_list args;
	va_start(args, _format);
	Write(Level::Important, _format, args);
	va_end(args);
}

void Info(const wchar_t* _format, ...)
{
	va_list args;
	va_start(args, _format);
	Write(Level::All, _format, args);
	va_end(args);
}

}
