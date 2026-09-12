#include "File.h"
#include "Conversion.h"

#ifdef _USING_V110_SDK71_

#ifndef XP
#	error Incorrectly configured build: XP
#endif

// XP toolset
vector<wstring> File::LoadFileNames(const wstring& _folder, const wstring& _filter)
{
	vector<wstring> files;

	wstring mask = _folder;
	if (!mask.empty() && mask.back() != L'\\')
		mask += L'\\';
	mask += L"*";
	mask += _filter;

	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW(mask.c_str(), &fd);

	if (h == INVALID_HANDLE_VALUE)
		return files;

	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			wstring fullPath = _folder;
			if (!fullPath.empty() && fullPath.back() != L'\\')
				fullPath += L'\\';
			fullPath += fd.cFileName;

			files.push_back(fullPath);
		}
	} while (FindNextFileW(h, &fd));

	FindClose(h);
	return files;
}
#else
//C++17

#ifdef XP
#	error Incorrectly configured build: XP
#endif

#include <filesystem>
vector<wstring> File::LoadFileNames(const wstring& _folder, const wstring& _filter)
{
	vector<wstring> dicts;

	try {
		filesystem::path folderPath(_folder);

		if (!filesystem::exists(folderPath) || !filesystem::is_directory(folderPath))
			return dicts;

		wstring ext;
		wstring trimmed = Conversion::TrimWhiteChar(_filter);
		if (trimmed.length() > 0) {
			if (trimmed[0] == L'.')
				ext = _filter;
			else
				ext = L"." + _filter;
		}
		
		for (auto& entry : filesystem::directory_iterator(folderPath)) {
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension() != ext)
				continue;

			dicts.push_back(entry.path().wstring());
		}
	}
	catch (...) {}

	return dicts;
}
#endif

void File::SplitMask(wstring _mask, wstring& _folder, wstring& _extensionNoDot, const wstring& _defaultFolder, const wstring& _defaultExtensionNoDot)
{
	size_t slashPos = _mask.find_last_of(L"\\/");
	wstring filePart;
	if (slashPos == wstring::npos) {
		_folder = L".";
		filePart = _mask;
	}
	else {
		_folder = _mask.substr(0, slashPos);
		filePart = _mask.substr(slashPos + 1);
	}

	size_t dotPos = filePart.find_last_of(L'.');
	if (dotPos != wstring::npos && dotPos + 1 < filePart.size()) {
		_extensionNoDot = filePart.substr(dotPos + 1);
	}
	else {
		_extensionNoDot = _defaultExtensionNoDot;
	}

	if (_folder.empty())
		_folder = _defaultFolder;
}

wstring File::String(FILE_ENCODING _fileType)
{
	switch (_fileType) {
	case File::FILE_ENCODING::ANSI:
		return L"ANSI";
	case File::FILE_ENCODING::ASCII:
		return L"ASCII";
	case File::FILE_ENCODING::UTF8_BOM:
		return L"UTF8_BOM";
	case File::FILE_ENCODING::UTF8_NOBOM:
		return L"UTF8_NOBOM";
	case File::FILE_ENCODING::UTF16_LE_BOM:
		return L"UTF16_LE_BOM";
	case File::FILE_ENCODING::UTF16_LE_NOBOM:
		return L"UTF16_LE_NOBOM";
	case File::FILE_ENCODING::UTF16_BE_BOM:
		return L"UTF16_BE_BOM";
	case File::FILE_ENCODING::UTF16_BE_NOBOM:
		return L"UTF16_BE_NOBOM";
	case File::FILE_ENCODING::UNKNOWN:
	default:
		return L"UNKNOWN";
	}
}

bool File::CreateDirectoryRecursively(wstring _directory)
{
	_directory = Conversion::RightTrimString(_directory, L"/");
	_directory = Conversion::RightTrimString(_directory, L"\\");
	if (_directory.empty()) return true;

	DWORD fileAttributes = ::GetFileAttributesW(_directory.c_str());
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		size_t slashIndex = _directory.find_last_of(L"\\/");
		if (slashIndex != wstring::npos) {
			CreateDirectoryRecursively(_directory.substr(0, slashIndex));
		}

		if (::CreateDirectoryW(_directory.c_str(), nullptr) == FALSE) {
			return false; // Could not create directory
		}
	}
	else { // Specified directory name already exists as a file or directory
		bool isDirectoryOrJunction = ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) || ((fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

		if (!isDirectoryOrJunction) {
			return false; // Could not create directory because a file with the same name exists
		}
	}
	return true;
}

File::File() : mEncoding(FILE_ENCODING::UNKNOWN), mFileOpened(false), mErrors(ERRORS::Show)
{
}

File::~File()
{
	Close();
}

bool File::Open(const wstring& _filename, ERRORS _err)
{
	mErrors = _err;

	if (mErrors == ERRORS::Show) wprintf(L"Opening file: '%s'\n", _filename.c_str());

	HANDLE hFile = CreateFileW(_filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		mFileOpened = false;
		if (mErrors == ERRORS::Show) wprintf(L"Error - The file '%s' cannot be opened\n", _filename.c_str());
		return false;
	}

	DWORD fileSize = GetFileSize(hFile, nullptr);
	if (fileSize == INVALID_FILE_SIZE) {
		DWORD err = GetLastError();
		if (err != NO_ERROR) {
			CloseHandle(hFile);
			mFileOpened = false;
			if (mErrors == ERRORS::Show) wprintf(L"Error - GetFileSize failed (INVALID_FILE_SIZE), WinAPI error #%u\n", err);
			return false;
		}
	}

	try {
		mData.resize(fileSize);
	}
	catch (...) {
		CloseHandle(hFile);
		mFileOpened = false;
		if (mErrors == ERRORS::Show) wprintf(L"Error - Unknown memory exception during resize()\n");
		return false;
	}

	DWORD bytesRead = 0;
	if (!::ReadFile(hFile, mData.data(), fileSize, &bytesRead, nullptr)) {
		CloseHandle(hFile);
		mData.clear();
		mFileOpened = false;
		if (mErrors == ERRORS::Show) wprintf(L"Error - ReadFile failed. File='%s' [bytesRead=%u]\n", _filename.c_str(), bytesRead);
		return false;
	}

	CloseHandle(hFile);

	if (bytesRead < fileSize) {
		mData.resize(bytesRead);
	}

	mFileOpened = true;
	if (mErrors == ERRORS::Show) wprintf(L"File has been loaded\n");

	DetectEncoding();

	return true;
}

void File::Close()
{
	mData.clear();
	mData.shrink_to_fit();
	mFileOpened = false;
	mErrors = ERRORS::Show;
	mEncoding = FILE_ENCODING::UNKNOWN;
}

vector<unsigned char>& File::GetDataVector()
{
	return mData;
}

const vector<unsigned char>& File::GetDataVector() const
{
	return mData;
}

size_t File::GetLength() const
{
	return mData.size();
}

File::FILE_ENCODING File::GetEncoding() const
{
	return mEncoding;
}

void File::DetectEncoding()
{
	if (mData.size() < 2) {
		mEncoding = FILE_ENCODING::UNKNOWN;
		return;
	}

	// Check for BOM
	if (mData.size() >= 3 && mData[0] == 0xEF && mData[1] == 0xBB && mData[2] == 0xBF) {// UTF-8 BOM: EF BB BF
		mEncoding = FILE_ENCODING::UTF8_BOM;
		return;
	}

	if (mData[0] == 0xFF && mData[1] == 0xFE) {// UTF-16 LE BOM: FF FE
		mEncoding = FILE_ENCODING::UTF16_LE_BOM;
		return;
	}

	if (mData[0] == 0xFE && mData[1] == 0xFF) {// UTF-16 BE BOM: FE FF
		mEncoding = FILE_ENCODING::UTF16_BE_BOM;
		return;
	}

	// Heuristic Detection (No BOM present)
	bool isAscii = true;
	for (unsigned int i = 0; i < mData.size(); ++i) {// Check for pure ASCII (all bytes < 128)
		if (mData[i] > 127) {
			isAscii = false;
			break;
		}
	}
	if (isAscii) {
		mEncoding = FILE_ENCODING::ASCII;
		return;
	}

	if (IsValidUtf8NoBom()) {// Check for valid UTF-8 sequences (without BOM)
		mEncoding = FILE_ENCODING::UTF8_NOBOM;
		return;
	}

	if (IsLikelyUtf16LeNoBom()) {// Check for likely UTF-16 LE (many zero bytes at odd indices)
		mEncoding = FILE_ENCODING::UTF16_LE_NOBOM;
		return;
	}
	if (IsLikelyUtf16BeNoBom()) {// Check for likely UTF-16 BE (many zero bytes at even indices)
		mEncoding = FILE_ENCODING::UTF16_BE_NOBOM;
		return;
	}

	mEncoding = FILE_ENCODING::ANSI;// Fallback: Assume ANSI (system default encoding)
}

// Validates if the data buffer contains a valid UTF-8 sequence without BOM.
bool File::IsValidUtf8NoBom() const
{
	// Simple validation of UTF-8 sequences
	for (unsigned int i = 0; i < mData.size(); ) {
		unsigned char c = mData[i];
		if (c < 0x80) {// 1-byte sequence (ASCII)
			i += 1;
		}
		else if ((c & 0xE0) == 0xC0) {// 2-byte sequence: Check continuity byte
			if (i + 1 >= mData.size() || (mData[i + 1] & 0xC0) != 0x80) return false;
			i += 2;
		}
		else if ((c & 0xF0) == 0xE0) {// 3-byte sequence: Check two continuity bytes
			if (i + 2 >= mData.size() ||
				(mData[i + 1] & 0xC0) != 0x80 ||
				(mData[i + 2] & 0xC0) != 0x80) return false;
			i += 3;
		}
		else if ((c & 0xF8) == 0xF0) {// 4-byte sequence: Check three continuity bytes
			if (i + 3 >= mData.size() ||
				(mData[i + 1] & 0xC0) != 0x80 ||
				(mData[i + 2] & 0xC0) != 0x80 ||
				(mData[i + 3] & 0xC0) != 0x80) return false;
			i += 4;
		}
		else {
			return false;// Invalid start byte for UTF-8
		}
	}
	return true;
}

// Heuristic check for UTF-16 Little Endian without BOM.
// Assumes that if >90% of high-order bytes are null (0x00), it's likely LE Unicode.
bool File::IsLikelyUtf16LeNoBom() const
{
	if (mData.size() < 4) return false;

	int zeroCount = 0;
	for (unsigned int i = 1; i + 1 < mData.size(); i += 2) {// Check odd indices (high byte in LE)
		if (mData[i] == 0) zeroCount++;
	}
	double ratio = (double)zeroCount / ((mData.size() / 2) - 1);
	return ratio > 0.90;
}

// Heuristic check for UTF-16 Big Endian without BOM.
// Assumes that if >90% of low-order bytes are null (0x00), it's likely BE Unicode.
bool File::IsLikelyUtf16BeNoBom() const
{
	if (mData.size() < 4) return false;

	int zeroCount = 0;
	for (unsigned int i = 0; i + 1 < mData.size(); i += 2) {
		if (mData[i + 1] == 0) zeroCount++;
	}
	double ratio = (double)zeroCount / ((mData.size() / 2) - 1);
	return ratio > 0.90;
}

wstring File::GetContentAsWString(CONVERT_END_OF_LINE _eol) const
{
	if (!mFileOpened || mData.empty()) return L"";

	wstring result;
	const BYTE* src = mData.data();
	size_t srcLen = mData.size();

	size_t offset = 0;// Calculate offset to skip BOM if present
	if (mEncoding == FILE_ENCODING::UTF8_BOM) offset = 3;
	else if (mEncoding == FILE_ENCODING::UTF16_LE_BOM) offset = 2;
	else if (mEncoding == FILE_ENCODING::UTF16_BE_BOM) offset = 2;

	switch (mEncoding) {
	case FILE_ENCODING::UTF8_BOM:
	case FILE_ENCODING::UTF8_NOBOM:
	{// Convert UTF-8 to Wide String
		int len = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(src + offset), static_cast<int>(srcLen - offset), nullptr, 0);
		if (len > 0) {
			result.resize(len);
			int res = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(src + offset), static_cast<int>(srcLen - offset), &result[0], len);
			if (len != res) {
				DWORD err = GetLastError();
				wprintf(L"MultiByteToWideChar conversion error: expected %d chars, got %d (error %lu)\n",
					len, res, err);
			}
		}
		break;
	}
	case FILE_ENCODING::UTF16_LE_BOM:
	case FILE_ENCODING::UTF16_LE_NOBOM:
	{// UTF-16 LE is native byte order on Windows, so direct cast works
		if (mEncoding == FILE_ENCODING::UTF16_LE_BOM || mEncoding == FILE_ENCODING::UTF16_LE_NOBOM) {
			// Safety check: ensure we have an even number of bytes (complete wchar_t)
			size_t availableBytes = srcLen - offset;
			if (availableBytes % 2 != 0) availableBytes--;// Discard trailing partial byte

			const wchar_t* ws = reinterpret_cast<const wchar_t*>(src + offset);
			size_t wlen = (srcLen - offset) / sizeof(wchar_t);
			result.assign(ws, wlen);
		}
		break;
	}
	case FILE_ENCODING::UTF16_BE_BOM:
	case FILE_ENCODING::UTF16_BE_NOBOM:
	{// UTF-16 BE requires byte swapping to match Windows Little Endian
		size_t availableBytes = srcLen - offset;
		if (availableBytes % 2 != 0) availableBytes--;

		size_t wlen = (srcLen - offset) / sizeof(wchar_t);
		result.resize(wlen);
		for (size_t i = 0; i < wlen; ++i) {
			uint16_t val = *reinterpret_cast<const uint16_t*>(src + offset + i * 2);
			// Swap bytes: (High << 8) | Low -> becomes (Low << 8) | High
			result[i] = static_cast<wchar_t>((val >> 8) | (val << 8));
		}
		break;
	}
	case FILE_ENCODING::ASCII:
	case FILE_ENCODING::ANSI:
	{// Convert ANSI/ASCII to Wide String using system codepage
		int cp = (mEncoding == FILE_ENCODING::ASCII) ? 1250 : CP_ACP; // 1250 ako fallback pre ASCII, inak ACP
		int len = MultiByteToWideChar(cp, 0, reinterpret_cast<const char*>(src + offset), static_cast<int>(srcLen - offset), nullptr, 0);
		if (len > 0) {
			result.resize(len);
			int res = MultiByteToWideChar(cp, 0, reinterpret_cast<const char*>(src + offset), static_cast<int>(srcLen - offset), &result[0], len);
			if (len != res) {
				DWORD err = GetLastError();
				wprintf(L"MultiByteToWideChar conversion error: expected %d chars, got %d (error %lu)\n",
					len, res, err);
			}
		}
	}
	break;

	default:
		// Unknown or Binary: Return empty string
		break;
	}

	if (_eol == CONVERT_END_OF_LINE::DoNotTouch)
		return result;

	wstring normalized;
	Windows2Unix(result, normalized);

	return normalized;
}

wstringstream File::GetContentAsStream(CONVERT_END_OF_LINE _eol) const
{
	wstringstream ss;
	ss.str(GetContentAsWString(_eol));
	return ss;
}

vector<unsigned char> File::ConvertToBytes(const wstring& _text, File::FILE_ENCODING _encoding)
{
	vector<unsigned char> result;

	result.clear();
	if (_text.empty()) return result;

	switch (_encoding) {
	case File::FILE_ENCODING::UTF8_BOM:
	case File::FILE_ENCODING::UTF8_NOBOM: {
		// Find the exact length, including the terminating NULL character
		int requiredSize = WideCharToMultiByte(CP_UTF8, 0, _text.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (requiredSize <= 0) return result;

		int dataSize = requiredSize - 1;

		size_t totalBytes = static_cast<size_t>(dataSize);
		if (_encoding == File::FILE_ENCODING::UTF8_BOM) {
			totalBytes += 3;// BOM
		}

		size_t allocSize = totalBytes + 1;
		result.resize(allocSize);

		size_t writeOffset = 0;
		if (_encoding == File::FILE_ENCODING::UTF8_BOM) {
			result[0] = 0xEF;
			result[1] = 0xBB;
			result[2] = 0xBF;
			writeOffset = 3;
		}

		int written = WideCharToMultiByte(CP_UTF8, 0, _text.c_str(), -1, reinterpret_cast<char*>(&result[writeOffset]), dataSize + 1, nullptr, nullptr);

		if (written > 0 && written <= dataSize + 1) {
			size_t finalSize = writeOffset + written - 1;
			result.resize(finalSize);
		}
		else {
			result.resize(writeOffset);// Conversion error -> trim it back to BOM
		}
		break;
	}

	case File::FILE_ENCODING::ANSI:
	case File::FILE_ENCODING::ASCII: {
		int cp = (_encoding == File::FILE_ENCODING::ASCII) ? 1250 : CP_ACP;

		int requiredSize = WideCharToMultiByte(cp, 0, _text.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (requiredSize <= 0) return result;

		int dataSize = requiredSize - 1;

		if (dataSize == 0) {
			result.clear();
			break;
		}

		result.resize(static_cast<size_t>(dataSize) + 1);

		int written = WideCharToMultiByte(cp, 0, _text.c_str(), -1, reinterpret_cast<char*>(result.data()), dataSize + 1, nullptr, nullptr);

		if (written > 0 && written == requiredSize) {
			result.resize(written - 1);
		}
		else {
			result.clear();
		}
		break;
	}

	case File::FILE_ENCODING::UTF16_LE_BOM:
	case File::FILE_ENCODING::UTF16_LE_NOBOM: {
		if (_encoding == File::FILE_ENCODING::UTF16_LE_BOM) {
			result.resize(2);
			result[0] = 0xFF;
			result[1] = 0xFE;
		}

		size_t byteCount = _text.length() * sizeof(wchar_t);
		if (byteCount == 0) break;

		size_t offset = result.size();
		result.resize(offset + byteCount);

		// Little Endian is native Windows
		memcpy(&result[offset], _text.c_str(), byteCount);
		break;
	}

	case File::FILE_ENCODING::UTF16_BE_BOM:
	case File::FILE_ENCODING::UTF16_BE_NOBOM: {
		if (_encoding == File::FILE_ENCODING::UTF16_BE_BOM) {
			result.resize(2);
			result[0] = 0xFE;
			result[1] = 0xFF;
		}

		size_t byteCount = _text.length() * sizeof(wchar_t);
		if (byteCount == 0) break;

		size_t offset = result.size();
		result.resize(offset + byteCount);

		// Big Endian - swap BYTE 
		const wchar_t* wSrc = _text.c_str();
		for (size_t i = 0; i < _text.length(); ++i) {
			uint16_t val = static_cast<uint16_t>(wSrc[i]);
			uint16_t swapped = (val >> 8) | (val << 8);

			size_t destIdx = offset + (i * 2);
			result[destIdx] = static_cast<unsigned char>((swapped >> 8) & 0xFF);
			result[destIdx + 1] = static_cast<unsigned char>(swapped & 0xFF);
		}
		break;
	}

	default:
		return result;
	}

	return result;
}

bool File::ReadTextFile(const wstring& _filename, wstringstream& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err)
{
	File file;
	if (!file.Open(_filename, _err))
		return false;

	_buffer = file.GetContentAsStream(_eol);
	return true;
}

bool File::ReadTextFile(const wstring& _filename, wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err)
{
	File file;
	if (!file.Open(_filename, _err))
		return false;

	_buffer = file.GetContentAsWString(_eol);
	return true;
}

bool File::ReadFile(const wstring& _filename, vector<unsigned char>& _buffer, ERRORS _err)
{
	File file;
	if (!file.Open(_filename, _err))
		return false;

	_buffer = file.GetDataVector();
	return true;
}

bool File::ReadFile(const wstring& _filename, BYTE*& _buffer, unsigned int& _length, ERRORS _err)
{
	_buffer = nullptr;
	_length = 0;

	File file;
	if (!file.Open(_filename, _err))
		return false;

	if (file.GetLength() == 0)
		return true;

	BYTE* data = new BYTE[file.mData.size()];
	memcpy(data, file.mData.data(), file.mData.size());

	_buffer = data;
	_length = static_cast<unsigned int>(file.mData.size());
	return true;
}

bool File::ReadFile(const wstring& _filename, unique_ptr<BYTE[]>& _buffer, unsigned int& _length, ERRORS _err)
{
	//std::unique_ptr<BYTE[]> data;
	//data.get();

	_buffer.reset();
	_length = 0;

	File file;
	if (!file.Open(_filename, _err))
		return false;

	if (file.GetLength() == 0)
		return true;

	_length = static_cast<unsigned int>(file.mData.size());

	unique_ptr<BYTE[]> temp(new BYTE[_length]);
	memcpy(temp.get(), file.mData.data(), file.mData.size());

	_buffer = std::move(temp);
	return true;
}

bool File::WriteFile(const wstring& _filename, const wstring& _buffer, const File::FILE_ENCODING& _type, CONVERT_END_OF_LINE _eol, ERRORS _err)
{
	if (_filename.empty()) {
		if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: filename is empty\n");
		return false;
	}

	wstring normalizedBuffer;
	if (_eol == CONVERT_END_OF_LINE::Convert) {
		Unix2Windows(_buffer, normalizedBuffer);
	}
	else {
		normalizedBuffer = _buffer;
	}

	wchar_t drive[_MAX_DRIVE] = { 0 };
	wchar_t dir[_MAX_DIR] = { 0 };
	if (_wsplitpath_s(_filename.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0) == 0) {
		wstring folder = wstring(drive) + wstring(dir);
		if (!folder.empty() && folder != L"\\" && folder != L"/") {
			if (!CreateDirectoryRecursively(folder)) return false;
		}
	}

	vector<unsigned char> bytes = ConvertToBytes(normalizedBuffer, _type);
	if (bytes.empty() && normalizedBuffer.empty() == false) {
		if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: byte conversion returned empty buffer\n");
		return false;
	}

	HANDLE hFile = CreateFileW(_filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: cannot create file '%s' (WinAPI error #%u)\n", _filename.c_str(), GetLastError());
		return false;
	}

	DWORD bytesWritten = 0;
	bool success = false;
	if (!bytes.empty()) {
		success = ::WriteFile(hFile, bytes.data(), (DWORD)bytes.size(), &bytesWritten, nullptr);
		if (!success) {
			if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed while writing to '%s' [bytesWritten=%u] WinAPI error #%u\n", _filename.c_str(), bytesWritten, GetLastError());
		}
	}
	else {
		success = true;
	}

	CloseHandle(hFile);
	return success;
}

bool File::WriteUTF8File(const wstring& _filename, const wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err)
{
	return WriteFile(_filename, _buffer, File::FILE_ENCODING::UTF8_NOBOM, _eol, _err);
}

bool File::WriteANSIFile(const wstring& _filename, const wstring& _buffer, CONVERT_END_OF_LINE _eol, ERRORS _err)
{
	return WriteFile(_filename, _buffer, File::FILE_ENCODING::ANSI, _eol, _err);
}

bool File::WriteFile(const wstring& _filename, const BYTE* _buffer, unsigned int _length, ERRORS _err)
{
	if (_filename.empty()) {
		if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: filename is empty\n");
		return false;
	}

	wchar_t drive[_MAX_DRIVE] = { 0 };
	wchar_t dir[_MAX_DIR] = { 0 };
	if (_wsplitpath_s(_filename.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0) == 0) {
		wstring folder = wstring(drive) + wstring(dir);
		if (!folder.empty() && folder != L"\\" && folder != L"/") {
			if (!CreateDirectoryRecursively(folder)) {
				if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: unable to create directory '%s'\n", folder.c_str());
				return false;
			}
		}
	}

	HANDLE hFile = CreateFileW(_filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: cannot create file '%s' (WinAPI error #%u)\n", _filename.c_str(), GetLastError());
		return false;
	}

	bool success = true;
	if (_length > 0 && _buffer != nullptr) {
		DWORD bytesWritten = 0;
		if (!::WriteFile(hFile, _buffer, _length, &bytesWritten, nullptr)) {
			if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed while writing to '%s' [bytesWritten=%u] WinAPI error #%u\n", _filename.c_str(), bytesWritten, GetLastError());
			success = false;
		}
		else if (bytesWritten != _length) {
			if (_err == ERRORS::Show) wprintf(L"Error - WriteFile failed: incomplete write to '%s' [written=%u expected=%u]\n", _filename.c_str(), bytesWritten, _length);
			success = false;
		}
	}

	CloseHandle(hFile);
	return success;
}

bool File::WriteFile(const wstring& _filename, const vector<unsigned char>& _buffer, ERRORS _err)
{
	return WriteFile(_filename, _buffer.data(), static_cast<unsigned int>(_buffer.size()), _err);
}

void File::Windows2Unix(const wstring& _windows, wstring& _unix)
{
	_unix.reserve(_windows.length());// Optimization: reserve to avoid reallocations

	for (size_t i = 0; i < _windows.length(); ++i) {
		wchar_t c = _windows[i];

		if (c == (wchar_t)END_OF_LINE::R) {
			if (i + 1 < _windows.length() && _windows[i + 1] == (wchar_t)END_OF_LINE::N) {// Check if followed by '\n' (Windows style CRLF)
				_unix += (wchar_t)END_OF_LINE::N;// Convert CRLF to LF
				++i;
			}
			else {// Isolated CR (Old Mac style)
				_unix += c;
			}
		}
		else if (c != (wchar_t)END_OF_LINE::R) {// Any other character
			_unix += c;
		}
	}
}

void File::Unix2Windows(const wstring& _unix, wstring& _windows)
{
	_windows = _unix;

	size_t pos = 0;
	while ((pos = _windows.find((wchar_t)END_OF_LINE::N, pos)) != wstring::npos) {
		if (pos == 0 || _windows[pos - 1] != (wchar_t)END_OF_LINE::R) {
			_windows.insert(pos, 1, (wchar_t)END_OF_LINE::R);
			pos += 2;
		}
		else {
			pos++;
		}
	}
}
