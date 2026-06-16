#include "CommandLine.h"
#include "Conversion.h"
#include "version.h"
#include <set>

CommandLine::CommandLine() : mHelp(false)
{
}

CommandLine::ParamDef::ParamDef()
	: type(ParamType::BOOL),
	required(RequiredParam::Optional), hasDefault(false),
	outBool(nullptr), outInt(nullptr), outString(nullptr), outEnum(nullptr), outChar(nullptr), outColor(nullptr),
	defaultBool(false),
	defaultInt(0),
	defaultEnum(0),
	defaultChar(0),
	description(L""),
	defaultString(L""),
	seen(false)
{}

void CommandLine::AddParamBase(ParamDef& _p)
{
	// check duplicates inside this parameter
	set<wstring> localSet;
	for (auto& nm : _p.names) {
		wstring key = Conversion::ToLower(nm);
		if (!localSet.insert(key).second) {
			wprintf(L"Duplicate alias '%ls' in parameter definition\n", nm.c_str());
			throw runtime_error("Duplicate alias in parameter");
		}
	}

	// check duplicates across all parameters
	for (auto& nm : _p.names) {
		wstring key = Conversion::ToLower(nm);

		if (mAliasMap.count(key)) {
			size_t other = mAliasMap[key];
			wprintf(L"Parameter alias conflict: '%ls' is already used by parameter '%ls'\n", nm.c_str(), mParams[other].names[0].c_str());
			throw runtime_error("Duplicate parameter alias");
		}
		mAliasMap[key] = mParams.size();
	}

	mParams.push_back(_p);
}

void CommandLine::AddHelp(const vector<wstring>& _names, const wstring& _desc, bool& _outVar)
{
	mHelp = true;
	AddBool(_names, _desc, _outVar);
}

void CommandLine::AddBool(const vector<wstring>& _names, const wstring& _desc, bool& _outVar)
{
	AddBool(_names, _desc, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddBool(const vector<wstring>& _names, const wstring& _desc, bool& _outVar, RequiredParam _required, bool _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::BOOL;
	p.outBool = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultBool = _defaultValue;

	AddParamBase(p);
}

void CommandLine::AddInt(const vector<wstring>& _names, const wstring& _desc, int& _outVar)
{
	AddInt(_names, _desc, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddInt(const vector<wstring>& _names, const wstring& _desc, int& _outVar, RequiredParam _required, int _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::INT;
	p.outInt = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultInt = _defaultValue;

	AddParamBase(p);
}

void CommandLine::AddString(const vector<wstring>& _names, const wstring& _desc, wstring& _outVar)
{
	AddString(_names, _desc, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddString(const vector<wstring>& _names, const wstring& _desc, wstring& _outVar, RequiredParam _required, const wstring& _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::STRING;
	p.outString = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultString = _defaultValue;

	AddParamBase(p);
}

void CommandLine::AddEnum(const vector<wstring>& _names, const wstring& _desc, const map<wstring, int>& _enumMap, int& _outVar)
{
	AddEnum(_names, _desc, _enumMap, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddEnum(const vector<wstring>& _names, const wstring& _desc, const map<wstring, int>& _enumMap, int& _outVar, RequiredParam _required, int _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::ENUM;
	p.enumMap = _enumMap;
	p.outEnum = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultEnum = _defaultValue;

	AddParamBase(p);
}

void CommandLine::AddChar(const vector<wstring>& _names, const wstring& _desc, wchar_t& _outVar)
{
	AddChar(_names, _desc, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddChar(const vector<wstring>& _names, const wstring& _desc, wchar_t& _outVar, RequiredParam _required, wchar_t _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::CHAR;
	p.outChar = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultChar = _defaultValue;

	AddParamBase(p);
}

void CommandLine::AddColor(const vector<wstring>& _names, const wstring& _desc, ColorRGB& _outVar)
{
	AddColor(_names, _desc, _outVar, RequiredParam::Optional, _outVar);
}

void CommandLine::AddColor(const vector<wstring>& _names, const wstring& _desc, ColorRGB& _outVar, RequiredParam _required, const ColorRGB& _defaultValue)
{
	ParamDef p;
	p.names = _names;
	p.description = _desc;
	p.type = ParamType::COLOR;
	p.outColor = &_outVar;
	p.required = _required;
	p.hasDefault = (_required == RequiredParam::Optional);
	p.defaultColor = _defaultValue;

	AddParamBase(p);
}

bool CommandLine::ParseCommandLine(int _argc, wchar_t** _argv, int& _correctCount)
{
	_correctCount = 0;

	// apply defaults
	for (size_t i = 0; i < mParams.size(); i++) {
		ParamDef& p = mParams[i];
		if (p.hasDefault) {
			switch (p.type) {
			case ParamType::BOOL:   *p.outBool = p.defaultBool; break;
			case ParamType::INT:    *p.outInt = p.defaultInt; break;
			case ParamType::STRING: *p.outString = p.defaultString; break;
			case ParamType::ENUM:   *p.outEnum = p.defaultEnum; break;
			case ParamType::CHAR:	*p.outChar = p.defaultChar; break;
			case ParamType::COLOR:  *p.outColor = p.defaultColor; break;
			default: break;
			}
		}
	}

	for (int i = 1; i < _argc; i++) {
		wstring arg = _argv[i];
		if (arg.size() > 0) {
			if (arg.rfind(L"--", 0) == 0) arg = arg.substr(2);
			else if (arg[0] == L'-' || arg[0] == L'/') arg = arg.substr(1);
		}

		wstring value;
		size_t eq = arg.find(L'=');
		if (eq != wstring::npos) {
			value = arg.substr(eq + 1);
			arg = arg.substr(0, eq);
		}

		ParamDef* found = 0;
		for (size_t p = 0; p < mParams.size(); p++) {
			for (size_t n = 0; n < mParams[p].names.size(); n++) {
				if (Conversion::ToLower(mParams[p].names[n]) == Conversion::ToLower(arg)) {
					found = &mParams[p];
					break;
				}
			}
			if (found) break;
		}

		if (!found) {
			wprintf(L"Unknown parameter: -%s\n", arg.c_str());
			return false;
		}

		switch (found->type) {
		case ParamType::BOOL: {
			if (value.empty()) {
				*found->outBool = true;
			}
			else {
				if (value == L"1" || Conversion::ToLower(value) == L"true" || Conversion::ToLower(value) == L"on")
					*found->outBool = true;
				else if (value == L"0" || Conversion::ToLower(value) == L"false")
					*found->outBool = false;
				else {
					wprintf(L"Invalid boolean value: %s\n", value.c_str());
					return false;
				}
			}
			found->seen = true;
			_correctCount++;
			break;
		}
		case ParamType::INT: {
			if (!value.empty()) {
				*found->outInt = _wtoi(value.c_str());
			}
			else {
				if (i + 1 >= _argc) return false;
				*found->outInt = _wtoi(_argv[++i]);
			}
			found->seen = true;
			_correctCount++;
			break;
		}
		case ParamType::STRING: {
			if (!value.empty()) {
				*found->outString = value;
			}
			else {
				if (i + 1 >= _argc) return false;
				*found->outString = _argv[++i];
			}
			*found->outString = Conversion::ParseEscapeString(*found->outString);
			found->seen = true;
			_correctCount++;
			break;
		}
		case ParamType::ENUM: {
			if (!value.empty()) {
				wstring valLower = Conversion::ToLower(value);
				bool matched = false;
				for (auto& kv : found->enumMap) {
					if (Conversion::ToLower(kv.first) == valLower) {
						*found->outEnum = kv.second;
						matched = true;
						break;
					}
				}
				if (!matched) {
					wprintf(L"Invalid enum value: %s\n", value.c_str());
					return false;
				}
			}
			else {
				if (i + 1 >= _argc) return false;
				wstring val = _argv[++i];

				wstring valLower = Conversion::ToLower(val);
				bool matched = false;
				for (auto& kv : found->enumMap) {
					if (Conversion::ToLower(kv.first) == valLower) {
						*found->outEnum = kv.second;
						matched = true;
						break;
					}
				}
				if (!matched) {
					wprintf(L"Invalid enum value: %s\n", val.c_str());
					return false;
				}
			}
			found->seen = true;
			_correctCount++;
			break;
		}
		case ParamType::CHAR: {
			if (!value.empty()) {
				value = Conversion::TrimWhiteChar(value);
				if (value.empty()) {
					wprintf(L"Invalid char value for -%s\n", found->names[0].c_str());
					return false;
				}
				*found->outChar = value[0];
			}
			else {
				if (i + 1 >= _argc) return false;
				wstring val = Conversion::TrimWhiteChar(_argv[++i]);
				if (val.empty()) {
					wprintf(L"Invalid char value for -%s\n", found->names[0].c_str());
					return false;
				}
				*found->outChar = val[0];
			}
			found->seen = true;
			_correctCount++;
			break;
		}

		case ParamType::COLOR: {
			wstring valStr;
			if (!value.empty()) {
				valStr = value;
			}
			else {
				if (i + 1 >= _argc) {
					wprintf(L"Missing value for color parameter -%s\n", found->names[0].c_str());
					return false;
				}
				valStr = _argv[++i];
			}

			valStr = Conversion::TrimString(valStr, L"#");

			if (valStr.size() == 3) { // expand #abc → aabbcc
				wstring expanded;
				expanded.reserve(6);
				for (wchar_t c : valStr)
					expanded.append(2, c);

				valStr = expanded;
			}

			if (valStr.size() < 6) {
				wprintf(L"Invalid color format for -%s (expected #RRGGBB, RRGGBB, #RGB or RGB)\n", found->names[0].c_str());
				return false;
			}

			try {
				wstring hexPart = valStr.substr(0, 6);

				for (wchar_t c : hexPart) {
					if (!iswxdigit(c)) {
						wprintf(L"Invalid character in color value for -%s: %lc\n", found->names[0].c_str(), c);
						return false;
					}
				}

				unsigned long r = wcstol(hexPart.substr(0, 2).c_str(), nullptr, 16);
				unsigned long g = wcstol(hexPart.substr(2, 2).c_str(), nullptr, 16);
				unsigned long b = wcstol(hexPart.substr(4, 2).c_str(), nullptr, 16);

				if (r > 255 || g > 255 || b > 255) {
					wprintf(L"Color values must be between 0 and 255\n");
					return false;
				}

				*found->outColor = ColorRGB((unsigned char)r, (unsigned char)g, (unsigned char)b);
			}
			catch (...) {
				wprintf(L"Error parsing color value for -%s\n", found->names[0].c_str());
				return false;
			}

			found->seen = true;
			_correctCount++;
			break;
		}

		}
	}

	// check required
	for (size_t i = 0; i < mParams.size(); i++) {
		if (mParams[i].required == RequiredParam::Required) {
			if (!mParams[i].seen) {
				if (mHelp == false)
					wprintf(L"Missing required parameter: -%s\n", mParams[i].names[0].c_str());
				return false;
			}
		}
	}

	return true;
}

void CommandLine::Help()
{
	wprintf(L"%hs %hs\n", VER_PRODUCTNAME_STR, VER_FILE_VERSION_STR);
	wprintf(L"\t%hs\n\n", VER_FILE_DESCRIPTION_STR);
	wprintf(L"Examples:\n");
	wprintf(L"\t%hs -t Title -m Message\n", VER_ORIGINAL_FILENAME_STR);
	wprintf(L"\n");
	wprintf(L"Options:\n");

	for (size_t ip = 0; ip < mParams.size(); ip++) {
		const ParamDef& p = mParams[ip];

		// name
		wprintf(L"  ");
		for (size_t n = 0; n < p.names.size(); n++) {
			wprintf(L"-%ls", p.names[n].c_str());
			if (n + 1 < p.names.size()) wprintf(L", ");
		}

		// description
		wprintf(L"\n        %ls\n", p.description.c_str());

		// enum
		if (p.type == ParamType::ENUM && p.enumMap.size() != 0) {
			map<int, vector<wstring> > grouped;

			for (map<wstring, int>::const_iterator it = p.enumMap.begin(); it != p.enumMap.end(); ++it) {
				grouped[it->second].push_back(it->first);
			}

			wprintf(L"        Values:\n");

			for (map<int, vector<wstring> >::iterator g = grouped.begin(); g != grouped.end(); ++g) {
				const vector<wstring>& names = g->second;

				if (names.empty())
					continue;

				wprintf(L"            %ls", names[0].c_str());

				// alias
				if (names.size() > 1) {
					wprintf(L" (");
					for (size_t i = 1; i < names.size(); i++) {
						wprintf(L"%ls", names[i].c_str());
						if (i + 1 < names.size())
							wprintf(L", ");
					}
					wprintf(L")");
				}

				wprintf(L"\n");
			}
		}

		if (p.type == ParamType::COLOR) {
			wprintf(L"        Format: #RRGGBB or RRGGBB (Hex)\n");
			if (p.hasDefault) {
				wprintf(L"        Default: #%02X%02X%02X\n", p.defaultColor.r, p.defaultColor.g, p.defaultColor.b);
			}
		}

		wprintf(L"\n");
	}
}

bool CommandLine::WasProvided(const wstring& _name) const
{
	wstring key = Conversion::TrimWhiteChar(Conversion::ToLower(_name));

	auto it = mAliasMap.find(key);
	if (it == mAliasMap.end())
		return false;

	const ParamDef& p = mParams[it->second];
	return p.seen;
}
