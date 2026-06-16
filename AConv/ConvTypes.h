#pragma once

enum class ConvType : UINT {
	UNKNOWN				= 0,
	BYTE_TO_BYTE		= 1,
	BYTE_TO_WINDOWS		= 2,
	WINDOWS_TO_BYTE		= 3,
	WINDOWS_TO_WINDOWS	= 4
};
