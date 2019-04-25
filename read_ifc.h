#ifndef _READ_IFC_H_
#define _READ_IFC_H_

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

#include <iostream>
#include <string>

struct IfcItem;
struct IfcRelation;
struct IfcContains;
struct IfcConnection;
struct IfcDecomposedBy;
class ReadIFC
{
private:
	std::wstring filename;
	IfcItem* ifc_model;
	int_t model;

	int_t rel_aggregates_type;
	int_t rel_contained_in_spatial_struct;
	int_t site_type;

public:
	ReadIFC(const std::wstring& fn);
	~ReadIFC();

	void Parse();

	IfcItem* GetStructure() const;
	int_t GetModel() const;

private:
	IfcItem* CreateProject();
	IfcItem* CreateObject(
		int64_t obj_ins,
		IfcConnection* parent);
	IfcItem * CreateItem(
		int64_t instance,
		IfcConnection* parent);
	IfcContains* CreateContains(IfcItem* item);
	IfcDecomposedBy* CreateDecomposedBy(IfcItem* item);
	IfcContains* CreateRelationContains(int64_t ins, IfcItem* parent);
	IfcDecomposedBy* CreateRelDecomposedBy(int64_t ins, IfcItem* parent);
};

#endif