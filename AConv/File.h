#pragma once

#include "stdafx.h"

class File
{
public:
	enum class FILE_ENCODING : int {
		UNKNOWN				= 0,	// Unknown or binary content
		ANSI				= 1,	// System default codepage (0-255)
		ASCII				= 2,	// Strict 7-bit ASCII (0-127)
		UTF8_NOBOM			= 11,   // UTF-8 without BOM
		UTF8_BOM			= 12,	// UTF-8 with Byte Order Mark (EF BB BF)
		UTF16_LE_NOBOM		= 21,   // UTF-16 Little Endian without BOM
		UTF16_LE_BOM		= 22,   // UTF-16 Little Endian with BOM (FF FE)
		UTF16_BE_NOBOM		= 31,	// UTF-16 Big Endian without BOM
		UTF16_BE_BOM		= 32,   // UTF-16 Big Endian with BOM (FE FF)

		UTF8				= UTF8_NOBOM
	};

	enum class END_OF_LINE : int {
		R			= 0x0D, // CR = 13 = \r
		N			= 0x0A, // LF = 10 = \n
		Windows		= -1,	// \r\n
		Unix		= N,
		ClassicMac	= R,
		Amiga		= N,
		Atari8bit	= 0x9B, // 155 = string(1,char(155))
		ZX			= 0x0D, // 13 = \r
	};

	enum class CONVERT_END_OF_LINE {
		DoNotTouch	= false,
		Convert		= true,
	};

	enum class ERRORS {
		Hide = false,
		Show = true,
	};

	static void Windows2Unix(const wstring& _windows, wstring& _unix);
	static void Unix2Windows(const wstring& _unix, wstring& _windows);

	static vector<wstring> LoadFileNames(const wstring& _folder, const wstring& _extension);
	static void SplitMask(wstring _mask, wstring& _folder, wstring& _extensionNoDot, const wstring& _defaultFolder, const wstring& _defaultExtensionNoDot);
	static wstring String(FILE_ENCODING _fileType);
	static bool CreateDirectoryRecursively(wstring _directory);

private:
	vector <unsigned char> mData;
	FILE_ENCODING mEncoding;
	bool mFileOpened;
	ERRORS mErrors;

	void DetectEncoding();
	bool IsValidUtf8NoBom() const;
	bool IsLikelyUtf16LeNoBom() const;
	bool IsLikelyUtf16BeNoBom() const;

	static vector<unsigned char> ConvertToBytes(const wstring& _text, File::FILE_ENCODING _encoding);

public:

	File();
	~File();
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	File(File&&) noexcept = default;
	File& operator=(File&&) noexcept = default;

	bool Open(const wstring& _filename, ERRORS _err);
	void Close();

	vector<unsigned char>& GetDataVector();
	const vector<unsigned char>& GetDataVector() const;
	size_t GetLength() const;
	FILE_ENCODING GetEncoding() const;

	wstring GetContentAsWString(CONVERT_END_OF_LINE _eol) const;
	wstringstream GetContentAsStream(CONVERT_END_OF_LINE _eol) const;

	static bool ReadTextFile(const wstring& _filename, wstringstream& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err);
	static bool ReadTextFile(const wstring& _filename, wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err);
	static bool ReadFile(const wstring& _filename, vector<unsigned char>& _buffer, ERRORS _err);
	static bool ReadFile(const wstring& _filename, BYTE*& _buffer, unsigned int& _length, ERRORS _err);
	static bool ReadFile(const wstring& _filename, unique_ptr<BYTE[]>& _buffer, unsigned int& _length, ERRORS _err);

	static bool WriteFile(const wstring& _filename, const wstring& _buffer, const File::FILE_ENCODING& _type, CONVERT_END_OF_LINE _eol, ERRORS _err);
	static bool WriteUTF8File(const wstring& _filename, const wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err);
	static bool WriteANSIFile(const wstring& _filename, const wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err);

	static bool WriteFile(const wstring& _filename, const BYTE* _buffer, unsigned int _length, ERRORS _err);
	static bool WriteFile(const wstring& _filename, const vector<unsigned char>& _buffer, ERRORS _err);
};
