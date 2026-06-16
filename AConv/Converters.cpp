#pragma once

#include "Converters.h"
#include "Matcher.h"

void Converters::ConvertFromBytes(const vector<unsigned char>& _input, const vector<MappingEntry>& _map, ConvType _type, int _errorReportingLevel, wchar_t _errorCharacter, vector<unsigned char>& _outBytes, wstring& _outW)
{
	_outBytes.clear();
	_outW.clear();

	wchar_t buf[256];

	for (size_t pos = 0; pos < _input.size();) {
		const MappingEntry* m = Matcher::MatchBytes(_map, _input, pos);
		if (m) {
			if (m->dstKind == TOKEN_KIND::BYTE_SEQUENCE) {
				_outBytes.insert(_outBytes.end(), m->dstBytes.begin(), m->dstBytes.end());
			}
			else {
				_outW.append(m->dstW);
			}
			pos += m->srcBytes.size();
		}
		else {
			unsigned char b = _input[pos];
			if (_errorReportingLevel == 0) {
				if (_type == ConvType::BYTE_TO_BYTE) _outBytes.push_back(b);
				else _outW.push_back((wchar_t)b);
			}
			else if (_errorReportingLevel == 1) {
				if (_type == ConvType::BYTE_TO_BYTE) _outBytes.push_back(b);
				else _outW.push_back((wchar_t)b);

				swprintf_s(buf, L"Unknown byte at pos %zu: '%c' (%u) (0x%02X)\n", pos, b, (unsigned)b, (unsigned)b);
				wprintf(L"%s", buf);
			}
			else {
				if (_type == ConvType::BYTE_TO_BYTE) _outBytes.push_back((unsigned char)_errorCharacter);
				else _outW.push_back(_errorCharacter);

				swprintf_s(buf, L"Unknown byte at pos %zu: '%c' (%u) (0x%02X)\n", pos, b, (unsigned)b, (unsigned)b);
				wprintf(L"%s", buf);
			}
			++pos;
		}
	}
}

void Converters::ConvertFromWchars(const wstring& _input, const vector<MappingEntry>& _map, ConvType _type, int _errorReportingLevel, wchar_t _errorCharacter, vector<unsigned char>& _outBytes, wstring& _outW)
{
	_outBytes.clear();
	_outW.clear();

	wchar_t buf[256];

	for (size_t pos = 0; pos < _input.size();) {
		const MappingEntry* m = Matcher::MatchWchars(_map, _input, pos);
		if (m) {
			if (m->dstKind == TOKEN_KIND::BYTE_SEQUENCE) {
				_outBytes.insert(_outBytes.end(), m->dstBytes.begin(), m->dstBytes.end());
			}
			else {
				_outW.append(m->dstW);
			}
			pos += m->srcW.size();
		}
		else {
			wchar_t wc = _input[pos];
			if (_errorReportingLevel == 0) {
				if (_type == ConvType::WINDOWS_TO_BYTE) _outBytes.push_back((unsigned char)(wc & 0xFF));
				else _outW.push_back(wc);
			}
			else if (_errorReportingLevel == 1) {
				if (_type == ConvType::WINDOWS_TO_BYTE) _outBytes.push_back((unsigned char)(wc & 0xFF));
				else _outW.push_back(wc);

				swprintf_s(buf, L"Unknown wchar at pos %zu: '%lc' (%u) (0x%04X)\n", pos, wc, (unsigned)wc, (unsigned)wc);
				wprintf(L"%s", buf);
			}
			else {
				if (_type == ConvType::WINDOWS_TO_BYTE) _outBytes.push_back((unsigned char)_errorCharacter);
				else _outW.push_back(_errorCharacter);

				swprintf_s(buf, L"Unknown wchar at pos %zu: '%lc' (%u) (0x%04X)\n", pos, wc, (unsigned)wc, (unsigned)wc);
				wprintf(L"%s", buf);
			}
			++pos;
		}
	}
}
