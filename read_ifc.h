#ifndef _READ_IFC_H_
#define _READ_IFC_H_

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

#include <iostream>
#include <string>

struct IfcItem;
//struct IfcRelation;
struct IfcContains;
struct IfcConnection;
struct IfcDecomposedBy;
struct IfcHasProperty;
class ReadIFC
{
private:
	std::wstring filename;
	IfcItem* ifc_model;
	int_t model;

public:
	int_t rel_aggregates_type;
	int_t rel_contained_in_spatial_struct;
	int_t site_type;
	static int_t rel_defined_by_properties;

public:
	ReadIFC(const std::wstring& fn);
	~ReadIFC();

	void Parse();

	IfcItem* GetStructure() const;
	int_t GetModel() const;
	void Release();

	static IfcItem * CreateItem(
		int_t model,
		int_t instance,
		int_t site_type,
		IfcConnection* parent);
	static IfcContains* CreateContains(
		IfcItem* item,
		int_t rel_contained_in_spatial_struct,
		int_t site_type,
		int_t rel_aggregates_type,
		int_t model);
	static IfcContains* CreateRelationContains(
		int_t ins,
		int_t model,
		IfcItem* parent);
	static IfcHasProperty* CreateRelationProperty(
		int_t ins,
		int_t model,
		IfcItem* parent);
	static IfcItem* CreateObject(
		int_t obj_ins,
		int_t model,
		int_t site_type,
		int_t rel_contained_in_spatial_struct,
		int_t rel_aggregates_type,
		IfcConnection* parent);
	static IfcDecomposedBy* CreateDecomposedBy(
		IfcItem* item,
		int_t model,
		int_t site_type,
		int_t rel_contained_in_spatial_struct,
		int_t rel_aggregates_type);
	static IfcDecomposedBy* CreateRelDecomposedBy(
		int_t ins,
		int_t model,
		IfcItem* parent);
	static IfcHasProperty* CreateRelProperty(
		IfcItem* item,
		int_t model,
		int_t site_type,
		int_t rel_contained_in_spatial_struct,
		int_t rel_aggregates_type);

	IfcItem* CreateProject();
};

#endif