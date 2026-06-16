#pragma once

#include "stdafx.h"

#include <vector>
#include <map>
#include <string>
#include <tchar.h>

class CommandLine
{
public:
	struct ColorRGB {
		unsigned char r;
		unsigned char g;
		unsigned char b;

		ColorRGB() : r(0), g(0), b(0) {}
		ColorRGB(unsigned char _r, unsigned char _g, unsigned char _b) : r(_r), g(_g), b(_b) {}
	};

	enum class ParamType {
		BOOL,
		INT,
		STRING,
		ENUM,
		CHAR,
		COLOR
	};

	enum class RequiredParam {
		Optional = false,
		Required = true
	};

	struct ParamDef {
		bool seen;

		vector<wstring> names;
		wstring description;
		ParamType type;

		RequiredParam required;
		bool hasDefault;

		bool* outBool;
		int* outInt;
		wstring* outString;
		int* outEnum;
		wchar_t* outChar;
		ColorRGB* outColor;

		map<wstring, int> enumMap;

		bool defaultBool;
		int defaultInt;
		wstring defaultString;
		int defaultEnum;
		wchar_t defaultChar;
		ColorRGB defaultColor;

		ParamDef();
	};

private:
	vector<ParamDef> mParams;
	map<wstring, size_t> mAliasMap;
	bool mHelp;

	void AddParamBase(ParamDef& _p);

public:
	CommandLine();

	void AddHelp(const vector<wstring>& _names, const wstring& _desc, bool& _outVar);
	void AddBool(const vector<wstring>& _names, const wstring& _desc, bool& _outVar);
	void AddBool(const vector<wstring>& _names, const wstring& _desc, bool& _outVar, RequiredParam _required, bool _defaultValue);
	void AddInt(const vector<wstring>& _names, const wstring& _desc, int& _outVar);
	void AddInt(const vector<wstring>& _names, const wstring& _desc, int& _outVar, RequiredParam _required, int _defaultValue);
	void AddString(const vector<wstring>& _names, const wstring& _desc, wstring& _outVar);
	void AddString(const vector<wstring>& _names, const wstring& _desc, wstring& _outVar, RequiredParam _required, const wstring& _defaultValue);
	void AddEnum(const vector<wstring>& _names, const wstring& _desc, const map<wstring, int>& _enumMap, int& _outVar);
	void AddEnum(const vector<wstring>& _names, const wstring& _desc, const map<wstring, int>& _enumMap, int& _outVar, RequiredParam _required, int _defaultValue);
	void AddChar(const vector<wstring>& _names, const wstring& _desc, wchar_t& _outVar);
	void AddChar(const vector<wstring>& _names, const wstring& _desc, wchar_t& _outVar, RequiredParam _required, wchar_t _defaultValue);
	void AddColor(const vector<wstring>& _names, const wstring& _desc, ColorRGB& _outVar);
	void AddColor(const vector<wstring>& _names, const wstring& _desc, ColorRGB& _outVar, RequiredParam _required, const ColorRGB& _defaultValue);

	bool ParseCommandLine(int _argc, wchar_t** _argv, int& _correctCount);
	bool WasProvided(const wstring& _name) const;

	void Help();
};
