#ifndef _MERGE_IFC_H_
#define _MERGE_IFC_H_

#include "ifcengine/include/engine.h"
#include "ifcengine/include/engdef.h"

#include <iostream>
#include <list>

class ReadIFC;
void Merge(ReadIFC& fifc, ReadIFC& sifc);
bool IsSameIfcVersion(const std::list<int_t>& models);

#endif