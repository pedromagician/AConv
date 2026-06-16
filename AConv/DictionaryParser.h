#pragma once

#include "stdafx.h"
#include "DictionaryStructures.h"
#include "File.h"

class DictionaryParser {
public:
	static bool Parse(const wstring& _dictFile, vector<DictionaryPair>& _pairs, int& _codePage, bool& _toLower, bool& _toUpper, int& _errorReportingLevel, wchar_t& _errorCharacter, wstring& _encoding, File::ERRORS _err = File::ERRORS::Show);
	static bool CreateDefaultKeyMap();

	static vector<DictScore> DetectEncoding(const wstring& inputFile, const vector<wstring>& dictionaryFiles);

private:
	DictionaryParser() = delete;
	~DictionaryParser() = delete;

	static bool ParseByteList(const wstring& _str, bool _hexMode, vector<unsigned char>& _out);

	static IndexedDict BuildIndex(const vector<DictionaryPair>& pairs);
	static long long ScoreStringDictionary(const vector<unsigned char>& input, const vector<DictionaryPair>& pairs);
	static 	long long ScoreByteDictionary(const vector<unsigned char>& input, const IndexedDict& dict);
	static long long ScoreByteNGrams(const vector<unsigned char>& input);
	static long long ScoreTextNGrams(const wstring& text);
	static long long ScoreStringStringDictionary(const wstring& text, const vector<DictionaryPair>& pairs);
};
