#ifndef _IFC_STRUCT_H_
#define _IFC_STRUCT_H_

#include <iostream>

enum IfcType
{
	ENUM_CONTAINS_RELATION,
	ENUM_DECOMPOSES_RELATION,
	ENUM_TREE_ITEM
};

struct IfcContains;
struct IfcDecomposedBy;
struct IfcRelation;
struct IfcItem
{
	IfcType type;
	int64_t model;
	int64_t entity;
	int64_t instance;
	char* global_id;
	bool is_base;
	IfcConnection* contains;
	IfcConnection* decomposed_by;
	IfcConnection* parent;
	IfcItem* next;
};

struct IfcConnection
{
	//IfcType type;
	int64_t model;
	IfcItem* items;
	IfcItem* parent;
};

struct IfcContains : IfcConnection
{
	IfcContains* next;
};

struct IfcDecomposedBy : IfcConnection
{
	IfcDecomposedBy* next;
};

struct IfcRelation : IfcConnection
{
	IfcRelation* next;
};

#endif