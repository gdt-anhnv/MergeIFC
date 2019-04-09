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
	IfcContains* contains;
	IfcDecomposedBy* decomposed_by;
	IfcRelation* parent;
	IfcItem* next;
};

struct IfcContains
{
	IfcType type;
	int64_t model;
	IfcItem* items;
	IfcItem* parent;
	IfcContains* next;
};

struct IfcDecomposedBy
{
	IfcType type;
	int64_t model;
	IfcItem* items;
	IfcItem* parent;
	IfcDecomposedBy* next;
};

struct IfcRelation
{
	IfcType type;
	int64_t model;
	IfcItem* items;
	IfcItem* parent;
	IfcRelation* next;
};

#endif