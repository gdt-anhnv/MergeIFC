#ifndef _IFC_STRUCT_H_
#define _IFC_STRUCT_H_

#include "ifcengine/include/engdef.h"

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
struct IfcConnection;
struct IfcItem
{
	IfcType type;
	int_t model;
	int_t entity;
	int_t instance;
	char* global_id;
	bool is_base;
	IfcConnection* contains;
	IfcConnection* decomposed_by;
	IfcConnection* parent;
	IfcItem* next;
};

struct IfcConnection
{
	IfcType type;
	int_t model;
	IfcItem* items;
	IfcItem* parent;

	IfcConnection();
	virtual ~IfcConnection();
};

struct IfcContains : public IfcConnection
{
	IfcContains* next;

	IfcContains();
	~IfcContains();
};

struct IfcDecomposedBy : public IfcConnection
{
	IfcDecomposedBy* next;
	IfcDecomposedBy();
	~IfcDecomposedBy();
};

struct IfcRelation : public IfcConnection
{
	IfcRelation* next;
	IfcRelation();
	~IfcRelation();
};

#endif