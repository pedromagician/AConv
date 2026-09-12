#pragma once

#include "stdafx.h"
#include "DictionaryStructures.h"

// Index over a vector<MappingEntry>, grouped by the first byte/wchar of the
// source pattern, so matching at a given position only has to check the
// candidates that could possibly match there instead of the whole table.
struct MatchIndex {
	map<unsigned char, vector<const MappingEntry*>> byByte;
	map<wchar_t, vector<const MappingEntry*>> byWChar;
};

class Matcher {
public:
	static MatchIndex BuildIndex(const vector<MappingEntry>& _map);

	static const MappingEntry* MatchBytes(const MatchIndex& _index, const vector<unsigned char>& _data, size_t _pos);
	static const MappingEntry* MatchWchars(const MatchIndex& _index, const wstring& _data, size_t _pos);

private:
	Matcher() = delete;
	~Matcher() = delete;
};
