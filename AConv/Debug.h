#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#	define DEBUG_NEW	new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#	define new DEBUG_NEW
#endif

class Debug {
public:
	Debug();
	~Debug();
};
