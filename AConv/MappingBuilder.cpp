#pragma once

#include "MappingBuilder.h"

// Creating a mapping for a specific type of ConvType
void MappingBuilder::Build(const vector<DictionaryPair>& _pairs, ConvType _type, bool _switchLeftRightSidesOfDictionary, vector<MappingEntry>& _out)
{
	_out.clear();

	TOKEN_KIND srcKind, dstKind;
	switch (_type) {
	case ConvType::BYTE_TO_BYTE:
		srcKind = TOKEN_KIND::BYTE_SEQUENCE;
		dstKind = TOKEN_KIND::BYTE_SEQUENCE;
		break;
	case ConvType::BYTE_TO_WINDOWS:
		srcKind = TOKEN_KIND::BYTE_SEQUENCE;
		dstKind = TOKEN_KIND::WINDOWS_STRING;
		break;
	case ConvType::WINDOWS_TO_BYTE:
		srcKind = TOKEN_KIND::WINDOWS_STRING;
		dstKind = TOKEN_KIND::BYTE_SEQUENCE;
		break;
	case ConvType::WINDOWS_TO_WINDOWS:
		srcKind = TOKEN_KIND::WINDOWS_STRING;
		dstKind = TOKEN_KIND::WINDOWS_STRING;
		break;
	default:
		wprintf(L"[Build] Unsupported ConvType %d – aborting\n", (int)_type);
		return;
	}

	for (size_t i = 0; i < _pairs.size(); ++i) {
		const DictionaryPair& p = _pairs[i];

		const DictionaryToken* srcTok = nullptr;
		const DictionaryToken* dstTok = nullptr;

		if (!_switchLeftRightSidesOfDictionary) {
			if (p.left.kind == srcKind && p.right.kind == dstKind) {
				srcTok = &p.left;
				dstTok = &p.right;
			}
			else if (p.right.kind == srcKind && p.left.kind == dstKind) {
				srcTok = &p.right;
				dstTok = &p.left;
			}
		}
		else {
			// Flip the dictionary
			if (p.left.kind == srcKind && p.right.kind == dstKind) {
				srcTok = &p.right;
				dstTok = &p.left;
			}
			else if (p.right.kind == srcKind && p.left.kind == dstKind) {
				srcTok = &p.left;
				dstTok = &p.right;
			}
		}

		if (!srcTok || !dstTok)
			continue;

		MappingEntry me;
		me.srcKind = srcKind;
		me.dstKind = dstKind;

		if (srcKind == TOKEN_KIND::BYTE_SEQUENCE)
			me.srcBytes = srcTok->bytes;
		else
			me.srcW = srcTok->wstr;

		if (dstKind == TOKEN_KIND::BYTE_SEQUENCE)
			me.dstBytes = dstTok->bytes;
		else
			me.dstW = dstTok->wstr;

		// empty
		if (srcKind == TOKEN_KIND::BYTE_SEQUENCE && me.srcBytes.empty()) {
			wprintf(L"[Build] Pair %zu skipped – empty srcBytes\n", i);
			continue;
		}
		if (srcKind == TOKEN_KIND::WINDOWS_STRING && me.srcW.empty()) {
			wprintf(L"[Build] Pair %zu skipped – empty srcW\n", i);
			continue;
		}

		if (dstKind == TOKEN_KIND::BYTE_SEQUENCE && me.dstBytes.empty()) {
			wprintf(L"[Build] Pair %zu skipped – empty dstBytes\n", i);
			continue;
		}
		if (dstKind == TOKEN_KIND::WINDOWS_STRING && me.dstW.empty()) {
			wprintf(L"[Build] Pair %zu skipped – empty dstW\n", i);
			continue;
		}

		_out.push_back(me);
	}

	wprintf(L"Built %zu mapping entries for conversion\n", _out.size());
}
