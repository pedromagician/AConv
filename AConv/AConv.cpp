#include "stdafx.h"

Debug theDebug;

#include "File.h"
#include "Conversion.h"
#include "Locale.h"
#include "CommandLine.h"
#include "ConvTypes.h"
#include "DictionaryParser.h"
#include "Convert.h"

int _tmain(int _argc, _TCHAR* _argv[])
{
	if (Locale::EnableLocale() == false)
		Log::Error(L"Error - Locale initialization failed\n");

	int correctParameters = 0;
	bool help = false;
	bool createDefaultKeyMap = false;
	bool decdump = false, hexdump = false;
	bool detect = false;
	wstring inputFileName = L"";
	wstring outputFileName = L"";
	bool inputClipboard = false;
	bool outputClipboard = false;
	wstring keyMapFileName = L"keymap.dict";
	wstring dictMask = L"dict\\*.dict";
	bool invertCodeMap = false;
	int errorReportingLevel = -1;
	wchar_t errorCharacter = '\0';

	map<wstring, int> listOfLogLevels {
		{L"all",		(int)Log::Level::All},
		{L"important",	(int)Log::Level::Important},
		{L"errors",		(int)Log::Level::Errors},
		{L"silent",		(int)Log::Level::Silent},
	};
	int logLevel = (int)Log::Level::All;

	map<wstring, int> listOfConversionTypes {
		{L"byte2byte",			(int)ConvType::BYTE_TO_BYTE},
		{L"byte2windows",		(int)ConvType::BYTE_TO_WINDOWS},
		{L"windows2byte",		(int)ConvType::WINDOWS_TO_BYTE},
		{L"windows2windows",	(int)ConvType::WINDOWS_TO_WINDOWS},
		{L"b2b",				(int)ConvType::BYTE_TO_BYTE},
		{L"b2w",				(int)ConvType::BYTE_TO_WINDOWS},
		{L"w2b",				(int)ConvType::WINDOWS_TO_BYTE},
		{L"w2w",				(int)ConvType::WINDOWS_TO_WINDOWS},
	};
	int conversionType = (int)ConvType::UNKNOWN;

	map<wstring, int> listOfOutputFileFormats {
		{L"ansi",		(int)File::FILE_ENCODING::ANSI},
		{L"utf8",		(int)File::FILE_ENCODING::UTF8},
		{L"utf8nobom",	(int)File::FILE_ENCODING::UTF8_NOBOM},
		{L"utf8bom",	(int)File::FILE_ENCODING::UTF8_BOM},
		{L"utf8withbom",(int)File::FILE_ENCODING::UTF8_BOM},
		{L"utf8+bom",	(int)File::FILE_ENCODING::UTF8_BOM},
		{L"bom",		(int)File::FILE_ENCODING::UTF8_BOM},
	};
	int outputFileFormat = (int)File::FILE_ENCODING::UTF8;

	CommandLine cmd;
	cmd.AddHelp({ L"help", L"h", L"?" },
		L"Displays this help information",
		help);
	cmd.AddString({ L"input", L"i" },
		L"Specifies the source file name",
		inputFileName);
	cmd.AddString({ L"output", L"o" },
		L"Specifies the destination file name. If omitted, the output file name is generated as 'inputFileName.out'",
		outputFileName);
	cmd.AddBool({ L"inputClipboard", L"ic" },
		L"Specifies that the input data should be read from the system clipboard instead of a file.",
		inputClipboard);
	cmd.AddBool({ L"outputClipboard", L"oc" },
		L"Specifies that the converted output should be written to the system clipboard instead of a file.",
		outputClipboard);
	cmd.AddBool({ L"decdump", L"dd" },
		L"Write the value of each byte from the input file to the output file in text form as a number in decimal notation.",
		decdump);
	cmd.AddBool({ L"hexdump", L"hd" },
		L"Write the value of each byte from the input file to the output file in text form as a number in hexadecimal notation.",
		hexdump);
	cmd.AddBool({ L"detect", L"d" },
		L"Detect the encoding of the input file.",
		detect);
	cmd.AddBool({ L"generatekeymap", L"g" },
		L"Generates a default character map and saves it to 'keyMap.dict'",
		createDefaultKeyMap);
	cmd.AddString({ L"keymap", L"k"},
		L"Specifies a single character map file used for conversion. This option applies only when converting. Default: 'keyMap.dict'.",
		keyMapFileName);
	cmd.AddString({ L"dict" },
		L"Specifies dictionary files used for encoding detection. Supports masks (e.g., 'dict\\*.dict') and typically expands to multiple files.",
		dictMask);
	cmd.AddEnum({ L"type", L"t" },
		L"Specifies the conversion type:",
		listOfConversionTypes, conversionType);
	cmd.AddBool({ L"invert", L"inv" },
		L"Inverts the character map. (Useful mainly for Windows‑to‑Windows or Byte-to-Byte conversions)",
		invertCodeMap);
	cmd.AddEnum({ L"format", L"f" },
		L"Sets the output file format (Windows mode) to:",
		listOfOutputFileFormats, outputFileFormat);
	cmd.AddInt({ L"errorlevel", L"e" }, L"Controls how unknown characters are handled :\
		\n\t    2(default) – Replace unknown characters with the 'error character' and display a warning\
		\n\t    1 – Do not replace, display a warning only\
		\n\t    0 – Do not replace and do not display warnings",
		errorReportingLevel);
	cmd.AddChar({ L"errorcharacter", L"c" },
		L"Specifies the replacement character used for unknown characters. The default replacement character is '?'",
		errorCharacter);
	cmd.AddEnum({ L"log", L"l" },
		L"Controls console logging:\
		\n\t    all (default) – Print everything\
		\n\t    important – Print only important messages (errors, warnings, results)\
		\n\t    errors – Print only errors\
		\n\t    silent – Print nothing",
		listOfLogLevels, logLevel);

	if (!cmd.ParseCommandLine(_argc, _argv, correctParameters)) {
		Log::Error(L"Run with -help for usage information.\n");
		return 0;
	}
	Log::SetLevel((Log::Level)logLevel);

	if (correctParameters == 0 || help) {
		cmd.Help();
		return 0;
	}

	if (createDefaultKeyMap) {
		return DictionaryParser::CreateDefaultKeyMap();
	}

	if (inputFileName.empty() && inputClipboard == false) {
		Log::Error(L"Error - unknown input file\n");
		return 1;
	}

	if (inputFileName.empty() == false && inputClipboard == true) {
		Log::Error(L"Error - input from file or clipboard?\n");
		return 1;
	}

	if (outputFileName.empty() == false && outputClipboard == true) {
		Log::Error(L"Error - save to file or clipboard?\n");
		return 1;
	}

	if (outputClipboard == false) {
		if (outputFileName.empty()) {
			outputFileName = inputFileName + L".out";
		}
		if (!outputFileName.empty() && ((outputFileName.back() == L'\\' || outputFileName.back() == L'/') || outputFileName.back() == L':')) {
			wstring newName = inputFileName.substr(inputFileName.find_last_of(L"/\\") + 1);
			if (newName.empty()) newName = inputFileName;
			outputFileName = outputFileName + newName + L".out";
		}
	}

	if (decdump || hexdump) {
		return Convert::Dump(inputFileName, outputFileName, hexdump, (File::FILE_ENCODING)outputFileFormat) ? 0 : 1;
	}

	if (detect) {
		wstring folder, extensionNoDot;
		File::SplitMask(dictMask, folder, extensionNoDot, L"dict", L"dict");
		vector<wstring> dicts = File::LoadFileNames(folder, extensionNoDot);
		if (dicts.empty()) {
			Log::Error(L"No keymap/dictionary files found\n  Folder: '%s'\n  Extension: '%s'\n  Mask example: %s\\*.%s\n", folder.c_str(), extensionNoDot.c_str(), folder.c_str(), extensionNoDot.c_str());
			return 1;
		}

		auto result = DictionaryParser::DetectEncoding(inputFileName, dicts);
		wprintf(L"\n");
		for (auto& record : result) {
			wprintf(L"%s => score %lld, confidence %.2f\n", record.encoding.c_str(), record.rawScore, record.confidence);
		}
		return 0;
	}

	if (ConvType(conversionType) == ConvType::UNKNOWN) {
		Log::Error(L"Unknown conversion mode\n");
		return 1;
	}

	return Convert::Run(inputFileName, outputFileName, keyMapFileName,
		ConvType(conversionType), 
		invertCodeMap, 
		errorReportingLevel, errorCharacter, 
		(File::FILE_ENCODING)outputFileFormat
	) ? 0 : 1;
}
