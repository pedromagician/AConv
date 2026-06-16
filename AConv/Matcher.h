#pragma once

#include "stdafx.h"
#include "DictionaryStructures.h"

class Matcher {
public:
	static const MappingEntry* MatchBytes(const vector<MappingEntry>& _map, const vector<unsigned char>& _data, size_t _pos);
	static const MappingEntry* MatchWchars(const vector<MappingEntry>& _map, const wstring& _data, size_t _pos);

private:
	Matcher() = delete;
	~Matcher() = delete;
};
