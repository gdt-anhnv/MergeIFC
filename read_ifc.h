#ifndef _READ_IFC_H_
#define _READ_IFC_H_

#include <iostream>
#include <string>

struct IfcItem;
struct IfcRelation;
struct IfcContains;
struct IfcDecomposedBy;
class ReadIFC
{
private:
	std::wstring filename;
	IfcItem* ifc_model;
	int64_t model;

	int64_t rel_aggregates_type;
	int64_t rel_contained_in_spatial_struct;
	int64_t site_type;

public:
	ReadIFC(const std::wstring& fn);
	~ReadIFC();

	void Parse();

private:
	IfcItem* CreateProject();
	IfcItem* CreateObject(
		int64_t obj_ins,
		IfcRelation* parent);
	IfcItem * CreateItem(
		int64_t instance,
		IfcRelation * parent);
	IfcContains* CreateContains(IfcItem* item);
	IfcDecomposedBy* CreateDecomposedBy(IfcItem* item);
	IfcContains* CreateRelationContains(int64_t ins, IfcItem* parent);
};

#endif