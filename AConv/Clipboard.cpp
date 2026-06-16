#include "Clipboard.h"
#include "Conversion.h"

bool Clipboard::ReadBytesFromClipboard(vector<unsigned char>& _outBytes)
{
	string strTmp;
	wstring wstrTmp;

	wprintf(L"Reading bytes from the clipboard\n");
	if (Clipboard::GetUnicode(wstrTmp)) {
		strTmp = Conversion::ConvertWString2String(wstrTmp);
		_outBytes.assign(strTmp.begin(), strTmp.end());
		return true;
	}

	if (Clipboard::GetUTF8(strTmp) || Clipboard::GetANSI(strTmp)) {
		_outBytes.assign(strTmp.begin(), strTmp.end());
		return true;
	}

	wprintf(L"Failed to read data from clipboard\n");
	return false;
}

bool Clipboard::ReadWstringFromClipboard(wstring& _buffer)
{
	wprintf(L"Reading text data from clipboard\n");
	if (Clipboard::GetUnicode(_buffer))
		return true;

	string strTmp;
	if (Clipboard::GetUTF8(strTmp) && !strTmp.empty()) {
		_buffer = Conversion::ConvertString2WString(strTmp);
		return true;
	}

	if (Clipboard::GetANSI(strTmp) && !strTmp.empty()) {
		_buffer = Conversion::ConvertString2WString(strTmp);
		return true;
	}

	wprintf(L"Failed to read data from clipboard\n");
	return false;
}

vector<Clipboard::ClipEntry> Clipboard::ReadClipboardAllFormats()
{
	vector<ClipEntry> out;

	if (!OpenClipboard(nullptr))
		return out;

	UINT fmt = 0;
	while ((fmt = EnumClipboardFormats(fmt)) != 0) {
		HANDLE hData = GetClipboardData(fmt);
		if (!hData)
			continue;

		void* ptr = GlobalLock(hData);
		if (!ptr)
			continue;

		SIZE_T size = GlobalSize(hData);
		if (size == 0) {
			GlobalUnlock(hData);
			continue;
		}

		ClipEntry entry;
		entry.format = fmt;

		char name[256];
		if (GetClipboardFormatNameA(fmt, name, sizeof(name)))
			entry.formatName = name;
		else {
			switch (fmt) {
			case CF_TEXT: entry.formatName = "CF_TEXT"; break;
			case CF_UNICODETEXT: entry.formatName = "CF_UNICODETEXT"; break;
			case CF_DIB: entry.formatName = "CF_DIB"; break;
			case CF_HDROP: entry.formatName = "CF_HDROP"; break;
			default: entry.formatName = "UNKNOWN";
			}
		}

		entry.data.resize(size);
		memcpy(entry.data.data(), ptr, size);

		GlobalUnlock(hData);
		out.push_back(move(entry));
	}

	CloseClipboard();
	return out;
}

HGLOBAL Clipboard::AllocAndCopy(const void* _data, SIZE_T _size)
{
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, _size);
	if (!hMem)
		return nullptr;

	void* ptr = GlobalLock(hMem);
	memcpy(ptr, _data, _size);
	GlobalUnlock(hMem);
	return hMem;
}

bool Clipboard::SetUnicode(const wstring& _text)
{
	if (!OpenClipboard(nullptr))
		return false;
	EmptyClipboard();

	SIZE_T size = (_text.size() + 1) * sizeof(wchar_t);
	HGLOBAL hMem = AllocAndCopy(_text.c_str(), size);
	if (!hMem) {
		CloseClipboard();
		return false;
	}

	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
	return true;
}

bool Clipboard::GetUnicode(wstring& _out)
{
	if (!OpenClipboard(nullptr))
		return false;

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	if (!h) {
		CloseClipboard();
		return false;
	}

	wchar_t* ptr = (wchar_t*)GlobalLock(h);
	if (!ptr) {
		CloseClipboard();
		return false;
	}

	_out = ptr;
	GlobalUnlock(h);
	CloseClipboard();
	return true;
}

bool Clipboard::SetANSI(const string& _text)
{
	if (!OpenClipboard(nullptr))
		return false;
	EmptyClipboard();

	SIZE_T size = _text.size() + 1;
	HGLOBAL hMem = AllocAndCopy(_text.c_str(), size);
	if (!hMem) {
		CloseClipboard();
		return false;
	}

	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return true;
}

bool Clipboard::GetANSI(string& _out)
{
	if (!OpenClipboard(nullptr))
		return false;

	HANDLE h = GetClipboardData(CF_TEXT);
	if (!h) {
		CloseClipboard();
		return false;
	}

	char* ptr = (char*)GlobalLock(h);
	if (!ptr) {
		CloseClipboard();
		return false;
	}

	_out = ptr;
	GlobalUnlock(h);
	CloseClipboard();
	return true;
}

bool Clipboard::SetUTF8(const string& _text)
{
	return SetBytes(L"UTF8_TEXT", vector<uint8_t>(_text.begin(), _text.end()));
}

bool Clipboard::GetUTF8(string& _out)
{
	vector<uint8_t> data;
	if (!GetBytes(L"UTF8_TEXT", data))
		return false;

	_out.assign(data.begin(), data.end());
	return true;
}

bool Clipboard::SetBytes(const wstring& _formatName, const vector<uint8_t>& _data)
{
	UINT fmt = RegisterClipboardFormatW(_formatName.c_str());
	if (!fmt)
		return false;

	if (!OpenClipboard(nullptr))
		return false;
	EmptyClipboard();

	HGLOBAL hMem = AllocAndCopy(_data.data(), _data.size());
	if (!hMem) {
		CloseClipboard();
		return false;
	}

	SetClipboardData(fmt, hMem);
	CloseClipboard();
	return true;
}

bool Clipboard::GetBytes(const wstring& _formatName, vector<uint8_t>& _out)
{
	UINT fmt = RegisterClipboardFormatW(_formatName.c_str());
	if (!fmt)
		return false;

	if (!OpenClipboard(nullptr))
		return false;

	HANDLE h = GetClipboardData(fmt);
	if (!h) {
		CloseClipboard();
		return false;
	}

	uint8_t* ptr = (uint8_t*)GlobalLock(h);
	if (!ptr) {
		CloseClipboard();
		return false;
	}

	SIZE_T size = GlobalSize(h);
	_out.assign(ptr, ptr + size);

	GlobalUnlock(h);
	CloseClipboard();
	return true;
}
