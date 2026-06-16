#pragma once

#include "stdafx.h"

class Clipboard
{
public:
	struct ClipEntry {
		UINT format;
		string formatName;
		vector<uint8_t> data;
	};

	// Helpers
	static vector<ClipEntry> ReadClipboardAllFormats();
	static bool ReadWstringFromClipboard(wstring& _buffer);
	static bool ReadBytesFromClipboard(vector<unsigned char>& _outBytes);
	
	// UTF-16 (wstring)
	static bool SetUnicode(const wstring& _text);
	static bool GetUnicode(wstring& _out);

	// UTF-8 (string)
	static bool SetUTF8(const string& _text);
	static bool GetUTF8(string& _out);

	// ANSI (string)
	static bool SetANSI(const string& _text);
	static bool GetANSI(string& _out);

	// Raw bytes (custom format)
	static bool SetBytes(const wstring& _formatName, const vector<uint8_t>& _data);
	static bool GetBytes(const wstring& _formatName, vector<uint8_t>& _out);

private:
	static HGLOBAL AllocAndCopy(const void* _data, SIZE_T _size);
};
