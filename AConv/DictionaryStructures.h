#pragma once

enum class TOKEN_KIND {
	BYTE_SEQUENCE,
	WINDOWS_STRING
};

struct DictionaryToken {
	TOKEN_KIND kind;
	vector<unsigned char> bytes;
	wstring wstr;
};

struct DictionaryPair {
	DictionaryToken left;
	DictionaryToken right;
};

struct MappingEntry {
	TOKEN_KIND srcKind;
	TOKEN_KIND dstKind;
	vector<unsigned char> srcBytes;
	wstring srcW;
	vector<unsigned char> dstBytes;
	wstring dstW;
};

struct DictScore {
	wstring dictName; 
	wstring encoding;
	long long rawScore;
	double confidence;
};

struct IndexedDict {
	// map: firstByte -> list of pairs starting with that byte
	map<unsigned char, vector<const DictionaryPair*>> index;
};
