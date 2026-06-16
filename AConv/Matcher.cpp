#pragma once

#include "Matcher.h"

const MappingEntry* Matcher::MatchBytes(const vector<MappingEntry>& _map, const vector<unsigned char>& _data, size_t _pos)
{
	const MappingEntry* best = nullptr;
	size_t bestLen = 0;

	for (size_t i = 0; i < _map.size(); ++i) {
		const MappingEntry& m = _map[i];
		if (m.srcKind != TOKEN_KIND::BYTE_SEQUENCE)
			continue;
		size_t len = m.srcBytes.size();
		if (len == 0 || _pos + len > _data.size())
			continue;
		bool ok = true;
		for (size_t j = 0; j < len; ++j) {
			if (_data[_pos + j] != m.srcBytes[j]) {
				ok = false;
				break;
			}
		}
		if (!ok)
			continue;
		if (len > bestLen) {
			bestLen = len;
			best = &m;
		}
	}
	return best;
}

const MappingEntry* Matcher::MatchWchars(const vector<MappingEntry>& _map, const wstring& _data, size_t _pos)
{
	const MappingEntry* best = nullptr;
	size_t bestLen = 0;

	for (size_t i = 0; i < _map.size(); ++i) {
		const MappingEntry& m = _map[i];
		if (m.srcKind != TOKEN_KIND::WINDOWS_STRING)
			continue;
		size_t len = m.srcW.size();
		if (len == 0 || _pos + len > _data.size())
			continue;
		bool ok = true;
		for (size_t j = 0; j < len; ++j) {
			if (_data[_pos + j] != m.srcW[j]) {
				ok = false;
				break;
			}
		}
		if (!ok)
			continue;
		if (len > bestLen) {
			bestLen = len;
			best = &m;
		}
	}
	return best;
}
