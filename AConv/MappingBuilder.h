#pragma once

#include "stdafx.h"
#include "DictionaryStructures.h"
#include "ConvTypes.h"

class MappingBuilder {
public:
	static void Build(const vector<DictionaryPair>& _pairs, ConvType _type, bool _switchLeftRightSidesOfDictionary, vector<MappingEntry>& _out);

private:
	MappingBuilder() = delete;
	~MappingBuilder() = delete;
};
