#pragma once

#include "Convert.h"
#include "DictionaryParser.h"
#include "MappingBuilder.h"
#include "Converters.h"
#include "Conversion.h"
#include "Clipboard.h"
#include <iomanip>

bool Convert::Dump(const wstring& _inputFile, const wstring& _outputFile, bool _hex /* false */, File::FILE_ENCODING _outputFileType /* File::FILE_ENCODING::UTF8 */)
{
	vector<unsigned char> inBytes;
	wstring buffer;
	if (_inputFile.empty()) {//CLIPBOARD
		if (Clipboard::ReadBytesFromClipboard(inBytes) == false)
			return false;
	}
	else {
		Log::Info(L"Opening input file (binary): %s\n", _inputFile.c_str());
		if (!File::ReadFile(_inputFile, inBytes, File::ERRORS::Show)) {
			Log::Error(L"Failed to read input file as bytes\n");
			return false;
		}
	}

	wstringstream ss;
	if (_hex) {
		ss << uppercase << hex << setfill(L'0');
		for (unsigned char b : inBytes) {
			ss << L"0x" << setw(2) << (int)b << L"\n";
		}
	}
	else {
		for (size_t i = 0; i < inBytes.size(); ++i) {
			ss << (int)inBytes[i] << L"-\n";
		}
	}
	buffer = ss.str();

	if (_outputFile.empty()) {//CLIPBOARD
		Log::Info(L"Writing data to clipboard\n");
		if (!Clipboard::SetUnicode(buffer)) {
			Log::Error(L"Failed to write data to clipboard\n");
			return false;
		}
	}
	else {
		Log::Info(L"Dump file: %s\n", _outputFile.c_str());
		if (!File::WriteFile(_outputFile, buffer, _outputFileType, File::CONVERT_END_OF_LINE::Convert, File::ERRORS::Show)) {
			Log::Error(L"Failed to write output file\n");
			return false;
		}
	}
	return true;
}

bool Convert::Run(const wstring& _inputFile, const wstring& _outputFile, const wstring& _dictionary,
	ConvType _type, bool _switchLeftRightSidesOfDictionary,
	int _errorReportingLevel, wchar_t _errorCharacter,
	File::FILE_ENCODING _outputFileType /* File::FILE_ENCODING::UTF8 */
)
{
	vector<DictionaryPair> dictPairs;
	int codePage = -1;
	bool toLower = false;
	bool toUpper = false;
	wstring encoding;
	if (!DictionaryParser::Parse(_dictionary, dictPairs, codePage, toLower, toUpper, _errorReportingLevel, _errorCharacter, encoding)) {
		Log::Error(L"Failed to parse dictionary\n");
		return false;
	}

	if (_errorReportingLevel < 0 || _errorReportingLevel > 2) _errorReportingLevel = 2;
	if (_errorCharacter == '\0') _errorCharacter = L'?';

	bool emptyIsOk = (_type == ConvType::WINDOWS_TO_WINDOWS && (toLower == true || toUpper == true || codePage != -1));

	vector<MappingEntry> mappings;
	MappingBuilder::Build(dictPairs, _type, _switchLeftRightSidesOfDictionary, mappings);
	if (mappings.empty() && emptyIsOk == false) {
		Log::Error(L"No usable mappings for requested conversion\n");
		return false;
	}

	vector<unsigned char> inBytes;
	wstring inW;

	if (_type == ConvType::UNKNOWN) {
		Log::Error(L"Unknown conversion mode\n");
		return false;
	}
	else if (_type == ConvType::BYTE_TO_BYTE || _type == ConvType::BYTE_TO_WINDOWS) {
		if (_inputFile.empty()) {//CLIPBOARD
			if (Clipboard::ReadBytesFromClipboard(inBytes) == false)
				return false;
		}
		else {
			Log::Info(L"Opening input file (binary): %s\n", _inputFile.c_str());
			if (!File::ReadFile(_inputFile, inBytes, File::ERRORS::Show)) {
				Log::Error(L"Failed to read input file as bytes\n");
				return false;
			}
		}
	}
	else {
		if (_inputFile.empty()) {//CLIPBOARD
			if (Clipboard::ReadWstringFromClipboard(inW) == false)
				return false;
		}
		else {
			Log::Info(L"Opening input text file: %s\n", _inputFile.c_str());
			if (!File::ReadTextFile(_inputFile, inW, File::CONVERT_END_OF_LINE::Convert, File::ERRORS::Show)) {
				Log::Error(L"Failed to read input file\n");
				return false;
			}
		}

		if (codePage >= 0)
			inW = Conversion::ConvertToCodePage(inW, (UINT) codePage);

		if (toLower)
			inW = Conversion::ToLower(inW);

		if (toUpper)
			inW = Conversion::ToUpper(inW);
	}

	vector<unsigned char> outBytes;
	wstring outW;

	Log::Info(L"Starting conversion...\n");

	switch (_type) {
	case ConvType::BYTE_TO_BYTE:
		Converters::ConvertFromBytes(inBytes, mappings, _type, _errorReportingLevel, _errorCharacter, outBytes, outW);

		if (_outputFile.empty()) {//CLIPBOARD
			string str(outBytes.begin(), outBytes.end());
			Log::Info(L"Writing data to clipboard\n");
			if (!Clipboard::SetANSI(str)) {
				Log::Error(L"Failed to write data to clipboard\n");
				return false;
			}
		}
		else {
			Log::Info(L"Writing output file (binary): %s\n", _outputFile.c_str());
			if (!File::WriteFile(_outputFile, outBytes, File::ERRORS::Show)) {
				Log::Error(L"Failed to write output file\n");
				return false;
			}
		}
		break;

	case ConvType::BYTE_TO_WINDOWS:
		Converters::ConvertFromBytes(inBytes, mappings, _type, _errorReportingLevel, _errorCharacter, outBytes, outW);

		if (_outputFile.empty()) {//CLIPBOARD
			Log::Info(L"Writing data to clipboard\n");
			if (!Clipboard::SetUnicode(outW)) {
				Log::Error(L"Failed to write data to clipboard\n");
				return false;
			}
		}
		else {
			Log::Info(L"Writing output file (text, %s): %s\n", File::String(_outputFileType).c_str(), _outputFile.c_str());
			if (!File::WriteFile(_outputFile, outW, _outputFileType, File::CONVERT_END_OF_LINE::Convert, File::ERRORS::Show)) {
				Log::Error(L"Failed to write output file\n");
				return false;
			}
		}
		break;

	case ConvType::WINDOWS_TO_BYTE:
		Converters::ConvertFromWchars(inW, mappings, _type, _errorReportingLevel, _errorCharacter, outBytes, outW);

		if (_outputFile.empty()) {//CLIPBOARD
			string str(outBytes.begin(), outBytes.end());
			Log::Info(L"Writing data to clipboard\n");
			if (!Clipboard::SetANSI(str)) {
				Log::Error(L"Failed to write data to clipboard\n");
				return false;
			}
		}
		else {
			Log::Info(L"Writing output file (binary): %s\n", _outputFile.c_str());
			if (!File::WriteFile(_outputFile, outBytes, File::ERRORS::Show)) {
				Log::Error(L"Failed to write output file\n");
				return false;
			}
		}
		break;

	case ConvType::WINDOWS_TO_WINDOWS:
		Converters::ConvertFromWchars(inW, mappings, _type, _errorReportingLevel, _errorCharacter, outBytes, outW);

		if (_outputFile.empty()) {//CLIPBOARD
			Log::Info(L"Writing data to clipboard\n");
			if (!Clipboard::SetUnicode(outW)) {
				Log::Error(L"Failed to write data to clipboard\n");
				return false;
			}
		}
		else {
			Log::Info(L"Writing output file (text, %s): %s\n", File::String(_outputFileType).c_str(), _outputFile.c_str());
			if (!File::WriteFile(_outputFile, outW, _outputFileType, File::CONVERT_END_OF_LINE::Convert, File::ERRORS::Show)) {
				Log::Error(L"Failed to write output file\n");
				return false;
			}
		}
		break;

	case ConvType::UNKNOWN:
	default:
		Log::Error(L"Unknown conversion mode\n");
		return false;
	}

	Log::Warn(L"Conversion finished\n");
	return true;
}
