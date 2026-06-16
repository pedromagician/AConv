#include "Conversion.h"

static const wchar_t WHITE_SPACE_CHARACTERS[] = L" \t\n\v\f\r\u00A0";

int Conversion::ToInt(const wstring& _buff, int _base)
{
	return (int)_tcstol(_buff.c_str(), nullptr, _base);
}

wstring Conversion::LeftTrimString(const wstring& _input, const wstring& _chars)
{
	const size_t pos = _input.find_first_not_of(_chars);
	if (pos == wstring::npos)
		return wstring();

	return _input.substr(pos);
}

wstring Conversion::RightTrimString(const wstring& _input, const wstring& _chars)
{
	const size_t pos = _input.find_last_not_of(_chars);
	if (pos == wstring::npos)
		return wstring();

	return _input.substr(0, pos + 1);
}

wstring Conversion::TrimString(const wstring& _input, const wstring& _chars)
{
	const size_t end = _input.find_last_not_of(_chars);

	if (end == wstring::npos)
		return wstring();

	const size_t begin = _input.find_first_not_of(_chars);
	return _input.substr(begin, end - begin + 1);
}

wstring Conversion::TrimWhiteChar(const wstring& _val)
{
	return TrimString(_val, WHITE_SPACE_CHARACTERS);
}

wstring Conversion::ToLower(const wstring& _input)
{
	if (_input.empty())
		return {};

	wstring result = _input;
	CharLowerBuffW(&result[0], static_cast<DWORD>(result.size()));
	return result;
}

wstring Conversion::ToUpper(const wstring& _input)
{
	if (_input.empty())
		return {};

	wstring result = _input;
	CharUpperBuffW(&result[0], static_cast<DWORD>(result.size()));
	return result;
}

void Conversion::StringReplaceAll(wstring& _mess, const wstring& _oldStr, const wstring& _newStr)
{
	const size_t oldLen = _oldStr.length();
	const size_t newLen = _newStr.length();
	size_t position = 0;
	while ((position = _mess.find(_oldStr, position)) != wstring::npos)
	{
		_mess.replace(position, oldLen, _newStr);
		position += newLen;
	}
}

bool Conversion::StartsWith(const wstring& _str, const wstring& _val)
{
	return _str.size() >= _val.size() && equal(_val.begin(), _val.end(), _str.begin());
}

wstring Conversion::ParseEscapeString(const wstring& _str)
{
	wstring out;
	for (size_t i = 0; i < _str.size(); ++i) {
		wchar_t c = _str[i];
		if (c != L'\\') {
			out.push_back(c);
			continue;
		}
		if (i + 1 >= _str.size()) {
			out.push_back(L'\\');
			break;
		}
		wchar_t n = _str[++i];

		// \n, \r, \t, \xHH, \uHHHH
		switch (n) {
		case L'n': out.push_back(L'\n'); break;
		case L'r': out.push_back(L'\r'); break;
		case L't': out.push_back(L'\t'); break;
		case L'\\': out.push_back(L'\\'); break;
		case L'\'': out.push_back(L'\''); break;
		case L'"': out.push_back(L'"'); break;
		case L'x':
		case L'u':
		{
			int maxDigits = (n == L'x') ? 2 : 4;
			int val = 0;
			int digits = 0;
			while (i + 1 < _str.size() && digits < maxDigits) {
				wchar_t h = _str[i + 1];
				int d;
				if (h >= L'0' && h <= L'9') d = h - L'0';
				else if (h >= L'a' && h <= L'f') d = 10 + (h - L'a');
				else if (h >= L'A' && h <= L'F') d = 10 + (h - L'A');
				else break;
				val = (val << 4) | d;
				++i;
				++digits;
			}

			if (digits == 0) {
				out.push_back(L'\\');
				out.push_back(n);
			}
			else {
				if (val >= 0xD800 && val <= 0xDFFF) {
					out.push_back(L'\\');
					out.push_back(n);
					for (int k = 0; k < digits; ++k)
						out.push_back(_str[i - digits + 1 + k]);
				}
				else {
					out.push_back((wchar_t)val);
				}
			}
			break;
		}
		default:
			out.push_back(n);
			break;
		}
	}
	return out;
}

wstring Conversion::ConvertString2WString(const string& _inputString, UINT _codePage /* CP_ACP */)
{
	int len = MultiByteToWideChar(_codePage, 0, _inputString.c_str(), (int)_inputString.length(), nullptr, 0);

	wstring wstrTo(len, 0);
	int res = MultiByteToWideChar(_codePage, 0, _inputString.c_str(), (int)_inputString.length(), &wstrTo[0], len);
	if (len != res) {
		DWORD err = GetLastError();
		wprintf(L"MultiByteToWideChar conversion error: expected %d chars, got %d (error %lu)\n",
			len, res, err);
	}
	return wstrTo;
}

string Conversion::ConvertWString2String(const wstring& _inputWString, UINT _codePage /* CP_ACP*/)
{
	int len = WideCharToMultiByte(_codePage, 0, _inputWString.c_str(), (int)_inputWString.length(), nullptr, 0, 0, 0);

	string strTo(len, 0);
	int res = WideCharToMultiByte(_codePage, 0, _inputWString.c_str(), (int)_inputWString.length(), &strTo[0], len, 0, 0);
	if (len != res) {
		DWORD err = GetLastError();
		wprintf(L"WideCharToMultiByte conversion error: expected %d chars, got %d (error %lu)\n",
			len, res, err);
	}
	return strTo;
}

wstring Conversion::ConvertToCodePage(const wstring& _text, UINT _codePage)
{
	string newText = ConvertWString2String(_text, _codePage);
	return ConvertString2WString(newText, _codePage);
}
