#pragma once

#include "stdafx.h"
#include "ConvTypes.h"
#include "File.h"

class Convert {
public:
	static bool Run(const wstring& _inputFile, const wstring& _outputFile, const wstring& _dictionary,
		ConvType _type, bool _switchLeftRightSidesOfDictionary,
		int _errorReportingLevel, wchar_t _errorCharacter,
		File::FILE_ENCODING _outputFileType = File::FILE_ENCODING::UTF8
	);

	static bool Dump(const wstring& _inputFile, const wstring& _outputFile, bool _hex = false, File::FILE_ENCODING _outputFileType = File::FILE_ENCODING::UTF8);

private:
	Convert() = delete;
	~Convert() = delete;
};
