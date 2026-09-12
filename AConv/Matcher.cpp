#pragma once

#include "Matcher.h"

MatchIndex Matcher::BuildIndex(const vector<MappingEntry>& _map)
{
	MatchIndex index;

	for (const auto& m : _map) {
		if (m.srcKind == TOKEN_KIND::BYTE_SEQUENCE) {
			if (!m.srcBytes.empty())
				index.byByte[m.srcBytes[0]].push_back(&m);
		}
		else {
			if (!m.srcW.empty())
				index.byWChar[m.srcW[0]].push_back(&m);
		}
	}

	// Longest pattern first, so the first successful match is the longest one -
	// same "longest match wins" rule the previous linear scan implemented.
	// stable_sort keeps ties in their original _map order, matching the old
	// strict "len > bestLen" tie-breaking (first one encountered wins).
	for (auto& kv : index.byByte) {
		stable_sort(kv.second.begin(), kv.second.end(), [](const MappingEntry* a, const MappingEntry* b) {
			return a->srcBytes.size() > b->srcBytes.size();
		});
	}
	for (auto& kv : index.byWChar) {
		stable_sort(kv.second.begin(), kv.second.end(), [](const MappingEntry* a, const MappingEntry* b) {
			return a->srcW.size() > b->srcW.size();
		});
	}

	return index;
}

const MappingEntry* Matcher::MatchBytes(const MatchIndex& _index, const vector<unsigned char>& _data, size_t _pos)
{
	auto it = _index.byByte.find(_data[_pos]);
	if (it == _index.byByte.end())
		return nullptr;

	for (const MappingEntry* m : it->second) {
		size_t len = m->srcBytes.size();
		if (_pos + len > _data.size())
			continue;

		bool ok = true;
		for (size_t j = 0; j < len; ++j) {
			if (_data[_pos + j] != m->srcBytes[j]) {
				ok = false;
				break;
			}
		}
		if (ok)
			return m;
	}
	return nullptr;
}

const MappingEntry* Matcher::MatchWchars(const MatchIndex& _index, const wstring& _data, size_t _pos)
{
	auto it = _index.byWChar.find(_data[_pos]);
	if (it == _index.byWChar.end())
		return nullptr;

	for (const MappingEntry* m : it->second) {
		size_t len = m->srcW.size();
		if (_pos + len > _data.size())
			continue;

		bool ok = true;
		for (size_t j = 0; j < len; ++j) {
			if (_data[_pos + j] != m->srcW[j]) {
				ok = false;
				break;
			}
		}
		if (ok)
			return m;
	}
	return nullptr;
}
