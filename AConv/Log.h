#pragma once

// Console logging levels (cumulative):
//   Silent    - nothing is printed
//   Errors    - only errors
//   Important - errors + warnings and key progress messages
//   All       - everything (default)
namespace Log {

enum class Level {
	Silent = 0,
	Errors = 1,
	Important = 2,
	All = 3,
};

void SetLevel(Level _level);
Level GetLevel();

void Error(const wchar_t* _format, ...);	// shown at Errors and above
void Warn(const wchar_t* _format, ...);		// shown at Important and above
void Info(const wchar_t* _format, ...);		// shown at All only

}
