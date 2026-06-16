#pragma once

#include "stdafx.h"

class Conversion {
public:
	static int ToInt(const wstring& _buff, int _base = 10);

	static wstring LeftTrimString(const wstring& _input, const wstring& _chars);
	static wstring RightTrimString(const wstring& _input, const wstring& _chars);
	static wstring TrimString(const wstring& _input, const wstring& _chars);
	static wstring TrimWhiteChar(const wstring& _val);

	static void StringReplaceAll(wstring& _mess, const wstring& _oldStr, const wstring& _newStr);

	static wstring ToLower(const wstring& _input);
	static wstring ToUpper(const wstring& _input);

	static bool StartsWith(const wstring& _str, const wstring& _val);
	static wstring ParseEscapeString(const wstring& _str);

	static wstring ConvertString2WString(const string& _inputString, UINT _codePage = CP_ACP);
	static string ConvertWString2String(const wstring& _inputWString, UINT _codePage = CP_ACP);
	static wstring ConvertToCodePage(const wstring& _text, UINT _codePage);

private:
	Conversion() = delete;
	~Conversion() = delete;
};
