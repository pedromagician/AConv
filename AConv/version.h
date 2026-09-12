#include "../git/revision.h"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               2
#define VERSION_MINOR               1
#define VERSION_PATCH               0
#define VERSION_REVISION            __BUILD_REVISION__

#ifdef _WIN64
#	define BITS_NUM				64
#	define BITS_STR				"(64-bit)"
#else
#	define BITS_NUM				32
#	define BITS_STR				"(32-bit)"
#endif

#ifdef _DEBUG
#	define DEBUG_STR         " DEBUG"
#else
#	define DEBUG_STR         ""
#endif

#define VER_COMPANYNAME_STR         "Wolf (original), Pedro (maintainer)"
#define VER_FILE_DESCRIPTION_STR    "Amiga/Atari/Windows/ZX conversion utility"

#ifdef XP
#	define VER_PRODUCTNAME_STR			"AConvXP"
#	define VER_INTERNAL_NAME_STR		"AConvXP"
#	define XP_STR						" XP"
#	define VER_ORIGINAL_FILENAME_STR	"AConvXP.exe"
#else
#	define VER_PRODUCTNAME_STR			"AConv"
#	define VER_INTERNAL_NAME_STR		"AConv"
#	define XP_STR						""
#	define VER_ORIGINAL_FILENAME_STR	"AConv.exe"
#endif

#define VER_COPYRIGHT_STR           "Copyright (C) 2026 Pedro - original concept by Wolf"

#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR) "." STRINGIZE(VERSION_MINOR) "." STRINGIZE(VERSION_PATCH) "." STRINGIZE(VERSION_REVISION) " " BITS_STR XP_STR DEBUG_STR 
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_REVISION
#define VER_PRODUCT_VERSION         VER_FILE_VERSION

#ifdef _DEBUG
#	ifdef NDEBUG
#		error Incorrectly configured build: mix of debug and release versions
#	endif
#endif
