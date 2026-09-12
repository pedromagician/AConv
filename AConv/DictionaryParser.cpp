#pragma once

#include "DictionaryParser.h"
#include "File.h"
#include "Conversion.h"
#include "Clipboard.h"

long long DictionaryParser::ScoreTextNGrams(const wstring& _text)
{
	if (_text.size() < 2) return 0;

	long long score = 0;

	// bigrams
	for (size_t i = 0; i + 1 < _text.size(); ++i) {
		wchar_t a = _text[i];
		wchar_t b = _text[i + 1];

		// typical combinations for "normal" text
		if (iswalpha(a) && iswalpha(b)) {
			score += 2;
		}
		if (iswspace(a) && iswalpha(b)) {
			score += 1;
		}
		if (iswalpha(a) && iswspace(b)) {
			score += 1;
		}
	}

	// trigrams
	for (size_t i = 0; i + 2 < _text.size(); ++i) {
		wchar_t a = _text[i];
		wchar_t b = _text[i + 1];
		wchar_t c = _text[i + 2];

		if (iswalpha(a) && iswalpha(b) && iswalpha(c)) {
			score += 3;
		}
		if (iswspace(a) && iswalpha(b) && iswalpha(c)) {
			score += 2;
		}
	}

	return score;
}

long long DictionaryParser::ScoreByteNGrams(const vector<unsigned char>& _input)
{
	if (_input.size() < 2) return 0;

	long long score = 0;

	// bigrams
	for (size_t i = 0; i + 1 < _input.size(); ++i) {
		unsigned char a = _input[i];
		unsigned char b = _input[i + 1];

		// typical combinations: letter-letter, letter-space, space-letter
		bool aPrint = (a >= 0x20 && a <= 0x7E);
		bool bPrint = (b >= 0x20 && b <= 0x7E);

		if (aPrint && bPrint) {
			score += 2;
		}
		if (aPrint && b == ' ') {
			score += 1;
		}
		if (a == ' ' && bPrint) {
			score += 1;
		}
	}

	// trigrams
	for (size_t i = 0; i + 2 < _input.size(); ++i) {
		unsigned char a = _input[i];
		unsigned char b = _input[i + 1];
		unsigned char c = _input[i + 2];

		bool aPrint = (a >= 0x20 && a <= 0x7E);
		bool bPrint = (b >= 0x20 && b <= 0x7E);
		bool cPrint = (c >= 0x20 && c <= 0x7E);

		if (aPrint && bPrint && cPrint) {
			score += 3;
		}
		if (a == ' ' && bPrint && cPrint) {
			score += 2;
		}
	}

	return score;
}

long long DictionaryParser::ScoreByteDictionary(const vector<unsigned char>& _input, const IndexedDict& _dict)
{
	long long score = 0;
	size_t i = 0;

	// longest match
	while (i < _input.size()) {
		unsigned char b = _input[i];

		auto it = _dict.index.find(b);
		if (it == _dict.index.end()) {
			i++;
			continue;
		}

		const auto& candidates = it->second;
		bool matched = false;

		for (const auto* p : candidates) {
			const auto& seq = p->left.bytes;
			size_t len = seq.size();

			if (i + len <= _input.size() &&
				memcmp(&_input[i], seq.data(), len) == 0)
			{
				score += (long long)len * 3; // longer sequences = higher scores
				i += len;
				matched = true;
				break;
			}
		}

		if (!matched)
			i++;
	}

	// text heuristics – decoding and checking "meaningful" characters
	long long textScore = 0;
	wstring decoded;

	i = 0;
	while (i < _input.size()) {
		unsigned char b = _input[i];
		auto it = _dict.index.find(b);

		if (it == _dict.index.end()) {
			decoded.push_back(L'?');
			i++;
			continue;
		}

		bool matched = false;
		for (const auto* p : it->second) {
			const auto& seq = p->left.bytes;
			size_t len = seq.size();

			if (i + len <= _input.size() &&
				memcmp(&_input[i], seq.data(), len) == 0)
			{
				decoded += p->right.wstr;
				i += len;
				matched = true;
				break;
			}
		}

		if (!matched) {
			decoded.push_back(L'?');
			i++;
		}
	}

	for (wchar_t c : decoded) {
		if (iswalpha(c) || iswdigit(c) || iswspace(c))
			textScore += 2;
		else if (c == L'?')
			textScore -= 10;
		else
			textScore += 1;
	}

	// N-gram heuristic applied to decoded text
	long long ngramScore = ScoreTextNGrams(decoded);

	return score + textScore + ngramScore / 5;
}

long long DictionaryParser::ScoreStringDictionary(const vector<unsigned char>& _input, const vector<DictionaryPair>& _pairs)
{
	long long score = 0;
	map<unsigned char, bool> known;
	for (const auto& p : _pairs) {
		if (p.right.kind == TOKEN_KIND::BYTE_SEQUENCE &&
			p.right.bytes.size() == 1)
		{
			known[p.right.bytes[0]] = true;
		}
	}

	// coverage
	for (unsigned char b : _input) {
		if (known.count(b))
			score += 2;
	}

	int freq[256] = { 0 };
	for (unsigned char b : _input) freq[b]++;

	for (auto& kv : known) {
		unsigned char b = kv.first;
		score += freq[b];
	}

	// N-gram heuristic on bytes
	long long ngramScore = ScoreByteNGrams(_input);

	return score + ngramScore / 5;
}

long long DictionaryParser::ScoreStringStringDictionary(const wstring& _text, const vector<DictionaryPair>& _pairs)
{
	long long score = 0;

	// number of occurrences of left-hand patterns (\a, \b, \c, ...)
	for (const auto& p : _pairs) {
		if (p.left.kind != TOKEN_KIND::WINDOWS_STRING)
			continue;

		const wstring& pat = p.left.wstr;
		if (pat.empty())
			continue;

		size_t pos = 0;
		while (true) {
			pos = _text.find(pat, pos);
			if (pos == wstring::npos)
				break;
			score += 50; // each pattern found has a weight
			pos += pat.size();
		}
	}

	// n-gram heuristic applied to the original text
	score += ScoreTextNGrams(_text);

	return score;
}

IndexedDict DictionaryParser::BuildIndex(const vector<DictionaryPair>& _pairs)
{
	IndexedDict out;
	for (const auto& p : _pairs) {
		if (p.left.kind != TOKEN_KIND::BYTE_SEQUENCE)
			continue;
		if (p.left.bytes.empty())
			continue;

		unsigned char first = p.left.bytes[0];
		out.index[first].push_back(&p);
	}

	for (auto& kv : out.index) {
		auto& vec = kv.second;
		sort(vec.begin(), vec.end(), [](const DictionaryPair* a, const DictionaryPair* b) { return a->left.bytes.size() > b->left.bytes.size(); });
	}

	return out;
}

vector<DictScore> DictionaryParser::DetectEncoding(const wstring& _inputFile, const vector<wstring>& _dictionaryFiles)
{
	vector<unsigned char> input;
	wstring inputText;

	if (_inputFile.empty()) {//CLIPBOARD
		if (Clipboard::ReadWstringFromClipboard(inputText) == false)
			return {};

		if (Clipboard::ReadBytesFromClipboard(input) == false)
			return {};
	}
	else {
		File file;
		if (!file.Open(_inputFile, File::ERRORS::Show))
			return {};

		wprintf(L"Encoding: %s\n", File::String(file.GetEncoding()).c_str());

		input = file.GetDataVector();
		inputText = file.GetContentAsWString(File::CONVERT_END_OF_LINE::Convert);
	}

	vector<DictScore> results;
	long long bestScore = 1;
	for (const auto& dictFile : _dictionaryFiles) {
		vector<DictionaryPair> pairs;
		int cp = -1;
		bool toLower = false;
		bool toUpper = false;
		int errLevel = 0;
		wchar_t errChar = 0;
		wstring encoding;

		if (!DictionaryParser::Parse(dictFile, pairs, cp, toLower, toUpper, errLevel, errChar, encoding, File::ERRORS::Hide)) {
			wprintf(L"Skipping dictionary %s (parse error)\n", dictFile.c_str());
			continue;
		}

		if (pairs.empty())
			continue;

		long long score = 0;

		TOKEN_KIND leftKind = pairs[0].left.kind;
		TOKEN_KIND rightKind = pairs[0].right.kind;

		if (leftKind == TOKEN_KIND::BYTE_SEQUENCE) {
			IndexedDict idx = BuildIndex(pairs);
			score = ScoreByteDictionary(input, idx);
		}
		else if (leftKind == TOKEN_KIND::WINDOWS_STRING && rightKind == TOKEN_KIND::BYTE_SEQUENCE) {
			score = ScoreStringDictionary(input, pairs);
		}
		else if (leftKind == TOKEN_KIND::WINDOWS_STRING && rightKind == TOKEN_KIND::WINDOWS_STRING) {
			score = ScoreStringStringDictionary(inputText, pairs);
		}
		else {
			continue;
		}

		bestScore = max(bestScore, score);
		results.push_back({ dictFile, encoding, score, 0.0 });
	}

	// confidence scoring
	for (auto& r : results) {
		r.confidence = (double)r.rawScore / (double)bestScore;
		if (r.confidence < 0) r.confidence = 0;
		if (r.confidence > 1) r.confidence = 1;
	}

	sort(results.begin(), results.end(), [](const DictScore& a, const DictScore& b) { return a.rawScore > b.rawScore; });

	return results;
}

bool DictionaryParser::ParseByteList(const wstring& _str, bool _hexMode, vector<unsigned char>& _out)
{
	_out.clear();
	wstringstream ss(_str);
	wstring item;

	while (getline(ss, item, L',')) {
		item = Conversion::TrimWhiteChar(item);
		if (item.empty())
			continue;
		unsigned int v = 0;

		wstring lower = Conversion::ToLower(item);
		if (_hexMode) {
			if (Conversion::StartsWith(lower, L"0x"))
				lower = lower.substr(2);

			if (lower.empty())
				return false;

			if (swscanf_s(lower.c_str(), L"%x", &v) != 1)
				return false;
		}
		else {
			if (swscanf_s(item.c_str(), L"%u", &v) != 1)
				return false;
		}

		if (v > 0xFF)
			return false;
		_out.push_back((unsigned char)v);
	}
	return !_out.empty();
}

bool DictionaryParser::Parse(const wstring& _dictFile, vector<DictionaryPair>& _pairs, int& _codePage, bool& _toLower, bool& _toUpper, int& _errorReportingLevel, wchar_t& _errorCharacter, wstring& _encoding, File::ERRORS _err /* File::ERRORS::Show */)
{
	_pairs.clear();
	_codePage = -1;
	_encoding = _dictFile;

	enum class DictSection {
		NONE,
		META,
		SETTINGS,
		DICTIONARY
	};

	DictSection section = DictSection::NONE;

	if (_err == File::ERRORS::Show) wprintf(L"Opening dictionary: %s", _dictFile.c_str());

	File file;
	if (!file.Open(_dictFile, _err)) {
		if (_err == File::ERRORS::Show) wprintf(L"The file could not be loaded\n");
		return false;
	}
	wstring content = file.GetContentAsWString(File::CONVERT_END_OF_LINE::Convert);

	if (_err == File::ERRORS::Show) wprintf(L"Encoding: %s\n", File::String(file.GetEncoding()).c_str());

	bool hexMode = false;// dec/hex
	wchar_t separator = L'-';

	// default pair type
	wstring currentLeftType = L"byte";
	wstring currentRightType = L"string";

	wstringstream ss(content);
	wstring line;
	size_t lineNo = 0;

	while (getline(ss, line)) {
		++lineNo;

		// remove CR
		if (!line.empty() && line.back() == L'\r')
			line.pop_back();

		wstring trimmed = Conversion::TrimWhiteChar(line);
		if (trimmed.empty())
			continue;

		// SECTION HANDLING
		if (trimmed.size() >= 2 && trimmed.front() == L'[' && trimmed.find_last_of(L']') == trimmed.size() - 1) {
			wstring sec = trimmed.substr(1, trimmed.size() - 2);
			sec = Conversion::TrimWhiteChar(sec);
			wstring lower = Conversion::ToLower(sec);

			if (lower == L"meta") {
				section = DictSection::META;
				continue;
			}
			if (lower == L"settings") {
				section = DictSection::SETTINGS;
				continue;
			}
			if (lower == L"dictionary") {
				section = DictSection::DICTIONARY;
				continue;
			}

			// dictionary pair type only valid in DICTIONARY
			if (section == DictSection::DICTIONARY) {
				if (Conversion::StartsWith(lower, L"separator")) {
					wstring strSeparator = sec.substr(9);
					strSeparator = Conversion::TrimWhiteChar(strSeparator);
					if (strSeparator.size() < 1) continue;
					separator = strSeparator[0];
					if (_err == File::ERRORS::Show) wprintf(L"Dictionary separator: %c\n", separator);
					continue;
				}
				if (lower == L"dec") {
					hexMode = false;
					if (_err == File::ERRORS::Show) wprintf(L"Dictionary number base: DEC\n");
					continue;
				}
				if (lower == L"hex") {
					hexMode = true;
					if (_err == File::ERRORS::Show) wprintf(L"Dictionary number base: HEX\n");
					continue;
				}

				size_t dash = lower.find(L'-');
				if (dash != wstring::npos) {
					currentLeftType = Conversion::TrimWhiteChar(lower.substr(0, dash));
					currentRightType = Conversion::TrimWhiteChar(lower.substr(dash + 1));
					if (_err == File::ERRORS::Show) wprintf(L"Dictionary pair type: [%s - %s]\n", currentLeftType.c_str(), currentRightType.c_str());
					continue;
				}
			}

			// unknown section → ignore
			continue;
		}

		// META SECTION
		if (section == DictSection::META) {
			if (trimmed[0] == L';') {
				wstring meta = Conversion::LeftTrimString(trimmed, L";");
				meta = Conversion::TrimWhiteChar(meta);
				wstring lower = Conversion::ToLower(meta);

				if (Conversion::StartsWith(lower, L"encoding:")) {
					meta = Conversion::TrimWhiteChar(meta.substr(9));
					_encoding = meta;
				}

				if (Conversion::StartsWith(lower, L"name:") || Conversion::StartsWith(lower, L"author:") || Conversion::StartsWith(lower, L"version:")) {
					if (_err == File::ERRORS::Show) wprintf(L"%s\n", meta.c_str());
				}
			}
			continue;
		}

		// SETTINGS SECTION
		if (section == DictSection::SETTINGS) {
			if (trimmed[0] == L';')
				continue;

			size_t eq = trimmed.find(L'=');
			if (eq == wstring::npos)
				continue;

			wstring key = Conversion::TrimWhiteChar(trimmed.substr(0, eq));
			wstring val = Conversion::TrimWhiteChar(trimmed.substr(eq + 1));
			wstring lowerKey = Conversion::ToLower(key);
			wstring lowerVal = Conversion::ToLower(val);

			if (lowerKey == L"number-base") {
				hexMode = (lowerVal == L"hex");
				if (_err == File::ERRORS::Show) wprintf(L"Dictionary number base: %s\n", hexMode ? L"HEX" : L"DEC");
			}
			else if (lowerKey == L"code-page") {
				if (_err == File::ERRORS::Show) wprintf(L"Code Page: %s\n", val.c_str());
				_codePage = (unsigned int)Conversion::ToInt(val);
			}
			else if (lowerKey == L"tolower") {
				_toLower = (lowerVal == L"true" || lowerVal == L"on" || lowerVal == L"1");
				if (_err == File::ERRORS::Show) wprintf(L"ToLower: %s\n", _toLower ? L"true" : L"false");
			}
			else if (lowerKey == L"toupper") {
				_toUpper = (lowerVal == L"true" || lowerVal == L"on" || lowerVal == L"1");
				if (_err == File::ERRORS::Show) wprintf(L"ToUpper: %s\n", _toUpper ? L"true" : L"false");
			}
			else if (lowerKey == L"errorlevel") {
				if (_errorReportingLevel < 0) _errorReportingLevel = Conversion::ToInt(val);
				if (_errorReportingLevel < 0 || _errorReportingLevel > 2) _errorReportingLevel = 2;

				if (_err == File::ERRORS::Show) wprintf(L"Error level: %u\n", _errorReportingLevel);
			}
			else if (lowerKey == L"errorcharacter") {
				if (_errorCharacter == '\0') {
					if (val.empty() == false) {
						val = Conversion::TrimWhiteChar(val);
						_errorCharacter = val[0];
					}
					else _errorCharacter = '?';
				}

				if (_err == File::ERRORS::Show) wprintf(L"Error character: %lc\n", _errorCharacter);
			}
			continue;
		}

		// DICTIONARY SECTION
		if (section != DictSection::DICTIONARY)
			continue;

		// data row: left-right
		size_t dash = line.find_first_of(separator);
		if (dash == 0 || dash == wstring::npos) {
			if (_err == File::ERRORS::Show) wprintf(L"Ignoring invalid dictionary line %zu: %s\n", lineNo, line.c_str());
			continue;
		}

		wstring leftPart = line.substr(0, dash);
		wstring rightPart = line.substr(dash + 1);

		DictionaryPair pair;

		// LEFT
		wstring leftTypeLower = Conversion::ToLower(currentLeftType);
		if (leftTypeLower == L"byte") {
			pair.left.kind = TOKEN_KIND::BYTE_SEQUENCE;
			if (!ParseByteList(leftPart, hexMode, pair.left.bytes)) {
				if (_err == File::ERRORS::Show) wprintf(L"Ignoring invalid byte list at line %zu (left)\n", lineNo);
				continue;
			}
		}
		else if (leftTypeLower == L"string") {
			pair.left.kind = TOKEN_KIND::WINDOWS_STRING;
			pair.left.wstr = leftPart;
		}
		else if (leftTypeLower == L"escape sequence") {
			pair.left.kind = TOKEN_KIND::WINDOWS_STRING;
			pair.left.wstr = Conversion::ParseEscapeString(leftPart);
		}
		else {
			if (_err == File::ERRORS::Show) wprintf(L"Ignoring dictionary line %zu: unknown left type '%s'\n", lineNo, currentLeftType.c_str());
			continue;
		}

		// RIGHT
		wstring rightTypeLower = Conversion::ToLower(currentRightType);
		if (rightTypeLower == L"byte") {
			pair.right.kind = TOKEN_KIND::BYTE_SEQUENCE;
			if (!ParseByteList(rightPart, hexMode, pair.right.bytes)) {
				if (_err == File::ERRORS::Show) wprintf(L"Ignoring invalid byte list at line %zu (right)\n", lineNo);
				continue;
			}
		}
		else if (rightTypeLower == L"string") {
			pair.right.kind = TOKEN_KIND::WINDOWS_STRING;
			pair.right.wstr = rightPart;
		}
		else if (rightTypeLower == L"escape sequence") {
			pair.right.kind = TOKEN_KIND::WINDOWS_STRING;
			pair.right.wstr = Conversion::ParseEscapeString(rightPart);
		}
		else {
			if (_err == File::ERRORS::Show) wprintf(L"Ignoring dictionary line %zu: unknown right type '%s'\n", lineNo, currentRightType.c_str());
			continue;
		}

		_pairs.push_back(pair);
	}

	return true;
}

void CreateDefaultKeyMapHelper(map<BYTE, wstring>& _km)
{
	_km.clear();
	_km.insert(pair<BYTE, wstring>((BYTE)0, L"CTRL+, ♥"));
	_km.insert(pair<BYTE, wstring>((BYTE)1, L"CTRL+a ├"));
	_km.insert(pair<BYTE, wstring>((BYTE)2, L"CTRL+b │"));
	_km.insert(pair<BYTE, wstring>((BYTE)3, L"CTRL+c ┘"));
	_km.insert(pair<BYTE, wstring>((BYTE)4, L"CTRL+d ┤"));
	_km.insert(pair<BYTE, wstring>((BYTE)5, L"CTRL+e ┐"));
	_km.insert(pair<BYTE, wstring>((BYTE)6, L"CTRL+f ◄"));
	_km.insert(pair<BYTE, wstring>((BYTE)7, L"CTRL+g ►"));
	_km.insert(pair<BYTE, wstring>((BYTE)8, L"CTRL+h ▲"));
	_km.insert(pair<BYTE, wstring>((BYTE)9, L"CTRL+i ░"));
	_km.insert(pair<BYTE, wstring>((BYTE)10, L"CTRL+j ▼"));
	_km.insert(pair<BYTE, wstring>((BYTE)11, L"CTRL+k ▒"));
	_km.insert(pair<BYTE, wstring>((BYTE)12, L"CTRL+l ▓"));
	_km.insert(pair<BYTE, wstring>((BYTE)13, L"CTRL+m ═"));
	_km.insert(pair<BYTE, wstring>((BYTE)14, L"CTRL+n ▬"));
	_km.insert(pair<BYTE, wstring>((BYTE)15, L"CTRL+o █"));
	_km.insert(pair<BYTE, wstring>((BYTE)16, L"CTRL+p ☺"));
	_km.insert(pair<BYTE, wstring>((BYTE)17, L"CTRL+q ┌"));
	_km.insert(pair<BYTE, wstring>((BYTE)18, L"CTRL+r ─"));
	_km.insert(pair<BYTE, wstring>((BYTE)19, L"CTRL+s ┼"));
	_km.insert(pair<BYTE, wstring>((BYTE)20, L"CTRL+t •"));
	_km.insert(pair<BYTE, wstring>((BYTE)21, L"CTRL+u ≈"));
	_km.insert(pair<BYTE, wstring>((BYTE)22, L"CTRL+v ║"));
	_km.insert(pair<BYTE, wstring>((BYTE)23, L"CTRL+w ┬"));
	_km.insert(pair<BYTE, wstring>((BYTE)24, L"CTRL+x ┴"));
	_km.insert(pair<BYTE, wstring>((BYTE)25, L"CTRL+y ▐"));
	_km.insert(pair<BYTE, wstring>((BYTE)26, L"CTRL+z └"));
	_km.insert(pair<BYTE, wstring>((BYTE)27, L"ESC ☼"));
	_km.insert(pair<BYTE, wstring>((BYTE)28, L"up arrow"));
	_km.insert(pair<BYTE, wstring>((BYTE)29, L"down arrow"));
	_km.insert(pair<BYTE, wstring>((BYTE)30, L"left arrow"));
	_km.insert(pair<BYTE, wstring>((BYTE)31, L"right arrow"));
	_km.insert(pair<BYTE, wstring>((BYTE)32, L" "));
	_km.insert(pair<BYTE, wstring>((BYTE)33, L"!"));
	_km.insert(pair<BYTE, wstring>((BYTE)34, L"\""));
	_km.insert(pair<BYTE, wstring>((BYTE)35, L"#"));
	_km.insert(pair<BYTE, wstring>((BYTE)36, L"$"));
	_km.insert(pair<BYTE, wstring>((BYTE)37, L"%"));
	_km.insert(pair<BYTE, wstring>((BYTE)38, L"&"));
	_km.insert(pair<BYTE, wstring>((BYTE)39, L"'"));
	_km.insert(pair<BYTE, wstring>((BYTE)40, L"("));
	_km.insert(pair<BYTE, wstring>((BYTE)41, L")"));
	_km.insert(pair<BYTE, wstring>((BYTE)42, L"*"));
	_km.insert(pair<BYTE, wstring>((BYTE)43, L"+"));
	_km.insert(pair<BYTE, wstring>((BYTE)44, L","));
	_km.insert(pair<BYTE, wstring>((BYTE)46, L"."));
	_km.insert(pair<BYTE, wstring>((BYTE)47, L"/"));
	_km.insert(pair<BYTE, wstring>((BYTE)48, L"0"));
	_km.insert(pair<BYTE, wstring>((BYTE)49, L"1"));
	_km.insert(pair<BYTE, wstring>((BYTE)50, L"2"));
	_km.insert(pair<BYTE, wstring>((BYTE)51, L"3"));
	_km.insert(pair<BYTE, wstring>((BYTE)52, L"4"));
	_km.insert(pair<BYTE, wstring>((BYTE)53, L"5"));
	_km.insert(pair<BYTE, wstring>((BYTE)54, L"6"));
	_km.insert(pair<BYTE, wstring>((BYTE)55, L"7"));
	_km.insert(pair<BYTE, wstring>((BYTE)56, L"8"));
	_km.insert(pair<BYTE, wstring>((BYTE)57, L"9"));
	_km.insert(pair<BYTE, wstring>((BYTE)58, L":"));
	_km.insert(pair<BYTE, wstring>((BYTE)59, L";"));
	_km.insert(pair<BYTE, wstring>((BYTE)60, L"<"));
	_km.insert(pair<BYTE, wstring>((BYTE)61, L"="));
	_km.insert(pair<BYTE, wstring>((BYTE)62, L">"));
	_km.insert(pair<BYTE, wstring>((BYTE)63, L"?"));
	_km.insert(pair<BYTE, wstring>((BYTE)64, L"@"));
	_km.insert(pair<BYTE, wstring>((BYTE)65, L"A"));
	_km.insert(pair<BYTE, wstring>((BYTE)66, L"B"));
	_km.insert(pair<BYTE, wstring>((BYTE)67, L"C"));
	_km.insert(pair<BYTE, wstring>((BYTE)68, L"D"));
	_km.insert(pair<BYTE, wstring>((BYTE)69, L"E"));
	_km.insert(pair<BYTE, wstring>((BYTE)70, L"F"));
	_km.insert(pair<BYTE, wstring>((BYTE)71, L"G"));
	_km.insert(pair<BYTE, wstring>((BYTE)72, L"H"));
	_km.insert(pair<BYTE, wstring>((BYTE)73, L"I"));
	_km.insert(pair<BYTE, wstring>((BYTE)74, L"J"));
	_km.insert(pair<BYTE, wstring>((BYTE)75, L"K"));
	_km.insert(pair<BYTE, wstring>((BYTE)76, L"L"));
	_km.insert(pair<BYTE, wstring>((BYTE)77, L"M"));
	_km.insert(pair<BYTE, wstring>((BYTE)78, L"N"));
	_km.insert(pair<BYTE, wstring>((BYTE)79, L"O"));
	_km.insert(pair<BYTE, wstring>((BYTE)80, L"P"));
	_km.insert(pair<BYTE, wstring>((BYTE)81, L"Q"));
	_km.insert(pair<BYTE, wstring>((BYTE)82, L"R"));
	_km.insert(pair<BYTE, wstring>((BYTE)83, L"S"));
	_km.insert(pair<BYTE, wstring>((BYTE)84, L"T"));
	_km.insert(pair<BYTE, wstring>((BYTE)85, L"U"));
	_km.insert(pair<BYTE, wstring>((BYTE)86, L"V"));
	_km.insert(pair<BYTE, wstring>((BYTE)87, L"W"));
	_km.insert(pair<BYTE, wstring>((BYTE)88, L"X"));
	_km.insert(pair<BYTE, wstring>((BYTE)89, L"Y"));
	_km.insert(pair<BYTE, wstring>((BYTE)90, L"Z"));
	_km.insert(pair<BYTE, wstring>((BYTE)91, L"["));
	_km.insert(pair<BYTE, wstring>((BYTE)92, L"\\"));
	_km.insert(pair<BYTE, wstring>((BYTE)93, L"]"));
	_km.insert(pair<BYTE, wstring>((BYTE)94, L"^"));
	_km.insert(pair<BYTE, wstring>((BYTE)95, L"_"));
	_km.insert(pair<BYTE, wstring>((BYTE)96, L"CTRL+. ♦"));
	_km.insert(pair<BYTE, wstring>((BYTE)97, L"a"));
	_km.insert(pair<BYTE, wstring>((BYTE)98, L"b"));
	_km.insert(pair<BYTE, wstring>((BYTE)99, L"c"));
	_km.insert(pair<BYTE, wstring>((BYTE)100, L"d"));
	_km.insert(pair<BYTE, wstring>((BYTE)101, L"e"));
	_km.insert(pair<BYTE, wstring>((BYTE)102, L"f"));
	_km.insert(pair<BYTE, wstring>((BYTE)103, L"g"));
	_km.insert(pair<BYTE, wstring>((BYTE)104, L"h"));
	_km.insert(pair<BYTE, wstring>((BYTE)105, L"i"));
	_km.insert(pair<BYTE, wstring>((BYTE)106, L"j"));
	_km.insert(pair<BYTE, wstring>((BYTE)107, L"k"));
	_km.insert(pair<BYTE, wstring>((BYTE)108, L"l"));
	_km.insert(pair<BYTE, wstring>((BYTE)109, L"m"));
	_km.insert(pair<BYTE, wstring>((BYTE)110, L"n"));
	_km.insert(pair<BYTE, wstring>((BYTE)111, L"o"));
	_km.insert(pair<BYTE, wstring>((BYTE)112, L"p"));
	_km.insert(pair<BYTE, wstring>((BYTE)113, L"q"));
	_km.insert(pair<BYTE, wstring>((BYTE)114, L"r"));
	_km.insert(pair<BYTE, wstring>((BYTE)115, L"s"));
	_km.insert(pair<BYTE, wstring>((BYTE)116, L"t"));
	_km.insert(pair<BYTE, wstring>((BYTE)117, L"u"));
	_km.insert(pair<BYTE, wstring>((BYTE)118, L"v"));
	_km.insert(pair<BYTE, wstring>((BYTE)119, L"w"));
	_km.insert(pair<BYTE, wstring>((BYTE)120, L"x"));
	_km.insert(pair<BYTE, wstring>((BYTE)121, L"y"));
	_km.insert(pair<BYTE, wstring>((BYTE)122, L"z"));
	_km.insert(pair<BYTE, wstring>((BYTE)123, L"CTRL+; ♣"));
	_km.insert(pair<BYTE, wstring>((BYTE)124, L"|"));
	_km.insert(pair<BYTE, wstring>((BYTE)125, L"CLEAR"));
	_km.insert(pair<BYTE, wstring>((BYTE)126, L"DELETE"));
	_km.insert(pair<BYTE, wstring>((BYTE)127, L"TAB 	"));
}

bool DictionaryParser::CreateDefaultKeyMap()
{
	map<BYTE, wstring> defaultKeyMap;
	CreateDefaultKeyMapHelper(defaultKeyMap);

	wstring bufferKeyMap = LR"(
[meta]
; Name: --name--
; Encoding: xxx
; Author: Name
; Version: 1.0

[settings]
number-base = dec

[dictionary]

[separator =]
[byte-string]
45=-

[separator -]
[byte-escape sequence]
10-\n

[byte-string]
)";

	for (const auto& it : defaultKeyMap) {
		bufferKeyMap += to_wstring((int)it.first) + L"-" + it.second + L"\n";
	}

	wstring keyMapFileName = L"defaultKeyMap.dict";
	File::WriteANSIFile(keyMapFileName, bufferKeyMap, File::CONVERT_END_OF_LINE::Convert, File::ERRORS::Show);

	wstring mess = L"Create: " + keyMapFileName + L"\n";
	wprintf(mess.c_str());

	return true;
}
