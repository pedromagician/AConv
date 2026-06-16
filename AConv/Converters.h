#pragma once

#include "stdafx.h"
#include "ConvTypes.h"
#include "DictionaryStructures.h"

class Converters {
public:
	static void ConvertFromBytes(const vector<unsigned char>& _input, const vector<MappingEntry>& _map, ConvType _type, int _errorReportingLevel, wchar_t _errorCharacter, vector<unsigned char>& _outBytes, wstring& _outW);
	static void ConvertFromWchars(const wstring& _input, const vector<MappingEntry>& _map, ConvType _type, int _errorReportingLevel, wchar_t _errorCharacter, vector<unsigned char>& _outBytes, wstring& _outW);

private:
	Converters() = delete;
	~Converters() = delete;
};
