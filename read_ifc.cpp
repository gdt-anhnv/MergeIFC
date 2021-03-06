#include "read_ifc.h"
#include "ifc_struct.h"

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

int_t ReadIFC::rel_defined_by_properties = 0;

ReadIFC::ReadIFC(const std::wstring & fn) :
	filename(fn),
	ifc_model(nullptr),
	model(0),
	rel_aggregates_type(0),
	rel_contained_in_spatial_struct(0),
	site_type(0)
{
}

ReadIFC::~ReadIFC()
{
	if (ifc_model)
	{

	}
}

void ReadIFC::Parse()
{
	model = sdaiOpenModelBNUnicode(0, (void*)filename.c_str(), L"D:\\Github\\MergeIFC\\ifcengine\\schema\\IFC4.exp");

	if (!model)
		return;

	ifc_model = CreateProject();
}

IfcItem * ReadIFC::GetStructure() const
{
	return ifc_model;
}

int_t ReadIFC::GetModel() const
{
	return model;
}

void ReadIFC::Release()
{
	if (model) {
		cleanMemory(model, 4);
		sdaiCloseModel(model);
	}
}

IfcItem* ReadIFC::CreateProject()
{
	rel_aggregates_type = sdaiGetEntity(model, "IFCRELAGGREGATES");
	rel_contained_in_spatial_struct = sdaiGetEntity(model, "IFCRELCONTAINEDINSPATIALSTRUCTURE");
	site_type = sdaiGetEntity(model, "IFCSITE");
	rel_defined_by_properties = sdaiGetEntity(model, "IFCRELDEFINESBYPROPERTIES");

	IfcItem* project_items = nullptr;

	int_t* proj_instances = sdaiGetEntityExtentBN(model, "IFCPROJECT");
	int_t num_proj_instances = sdaiGetMemberCount(proj_instances);

	for (int i = 0; i < num_proj_instances; i++)
	{
		int_t proj_ins = 0;
		engiGetAggrElement(proj_instances, i, sdaiINSTANCE, &proj_ins);
		IfcItem* proj_item = CreateObject(
			proj_ins,
			model,
			site_type,
			rel_contained_in_spatial_struct,
			rel_aggregates_type,
			nullptr);

		proj_item->next = project_items;
		project_items = proj_item;
	}

	return project_items;
}

IfcItem * ReadIFC::CreateObject(
	int_t obj_ins,
	int_t model,
	int_t site_type,
	int_t rel_contained_in_spatial_struct,
	int_t rel_aggregates_type,
	IfcConnection * parent)
{
	IfcItem* item = CreateItem(model, obj_ins, site_type, parent);
	item->contains = CreateContains(item, rel_contained_in_spatial_struct, site_type, rel_aggregates_type, model);
	item->decomposed_by = CreateDecomposedBy(item, model, site_type, rel_contained_in_spatial_struct, rel_aggregates_type);
	//item->has_properties = CreateRelProperty(item, model, site_type, rel_contained_in_spatial_struct, rel_aggregates_type);

	return item;
}

IfcItem * ReadIFC::CreateItem(int_t model, int_t instance, int_t site_type, IfcConnection * parent)
{
	IfcItem* item = new IfcItem();

	item->model = model;
	item->instance = instance;
	item->entity = sdaiGetInstanceType(instance);
	item->global_id = nullptr;
	sdaiGetAttrBN(instance, "GlobalId", sdaiSTRING, &item->global_id);
	item->is_base = site_type == item->entity;

	char* ent_name = nullptr;
	engiGetEntityName(item->entity, sdaiSTRING, &ent_name);
	int_t no_arg = engiGetEntityNoArguments(item->entity);

	item->contains = nullptr;
	item->decomposed_by = nullptr;
	item->has_properties = nullptr;

	item->parent = parent;
	item->next = nullptr;

	return item;
}

IfcContains * ReadIFC::CreateContains(
	IfcItem * item,
	int_t rel_contained_in_spatial_struct,
	int_t site_type,
	int_t rel_aggregates_type,
	int_t model)
{
	IfcContains* contains = nullptr;
	int_t* rel_spatials = nullptr;
	sdaiGetAttrBN(item->instance, "ContainsElements", sdaiAGGR, &rel_spatials);
	int_t rel_cnt = sdaiGetMemberCount(rel_spatials);

	char* ent_name = nullptr;
	engiGetEntityName(sdaiGetInstanceType(item->instance), sdaiSTRING, &ent_name);

	for (int i = 0; i < rel_cnt; i++)
	{
		int64_t rel_spatial_ins = 0;
		engiGetAggrElement(rel_spatials, i, sdaiINSTANCE, &rel_spatial_ins);
		if (!rel_spatial_ins)
			continue;

		if (sdaiGetInstanceType(rel_spatial_ins) != rel_contained_in_spatial_struct)
			continue;

		int64_t* obj_instances = nullptr;
		sdaiGetAttrBN(rel_spatial_ins, "RelatedElements", sdaiAGGR, &obj_instances);
		int64_t obj_ins_cnt = sdaiGetMemberCount(obj_instances);
		if (!obj_ins_cnt)
			continue;

		IfcContains* rel_contains = CreateRelationContains(rel_spatial_ins, model, item);
		rel_contains->next = contains;
		contains = rel_contains;

		for (int j = 0; j < obj_ins_cnt; j++)
		{
			int64_t obj_ins = 0;
			engiGetAggrElement(obj_instances, j, sdaiINSTANCE, &obj_ins);
			if (!obj_ins)
				continue;

			char* ent_name = nullptr;
			engiGetEntityName(sdaiGetInstanceType(obj_ins), sdaiSTRING, &ent_name);
			IfcItem* obj_item = CreateObject(
				obj_ins,
				model,
				site_type,
				rel_contained_in_spatial_struct,
				rel_aggregates_type,
				rel_contains);
			obj_item->next = rel_contains->items;
			contains->items = obj_item;
		}
	}

	return contains;
}

IfcHasProperty * ReadIFC::CreateRelProperty(
	IfcItem* item,
	int_t model,
	int_t site_type,
	int_t rel_contained_in_spatial_struct,
	int_t rel_aggregates_type)
{
	IfcHasProperty* has_properties = nullptr;
	int_t* rel_defines = nullptr;
	sdaiGetAttrBN(item->instance, "IsDefinedBy", sdaiAGGR, &rel_defines);
	int_t rel_cnt = sdaiGetMemberCount(rel_defines);

	char* ent_name = nullptr;
	engiGetEntityName(item->entity, sdaiSTRING, &ent_name);

	for (int i = 0; i < rel_cnt; i++)
	{
		int64_t rel_defined_ins = 0;
		engiGetAggrElement(rel_defines, i, sdaiINSTANCE, &rel_defined_ins);
		if (!rel_defined_ins)
			continue;

		if (sdaiGetInstanceType(rel_defined_ins) != ReadIFC::rel_defined_by_properties)
			continue;

		int64_t* obj_instances = nullptr;
		sdaiGetAttrBN(rel_defined_ins, "RelatedObjects", sdaiAGGR, &obj_instances);
		int64_t obj_ins_cnt = sdaiGetMemberCount(obj_instances);
		if (!obj_ins_cnt)
			continue;

		IfcHasProperty* rel_properties = CreateRelationProperty(rel_defined_ins, model, item);
		rel_properties->next = has_properties;
		has_properties = rel_properties;

		for (int j = 0; j < obj_ins_cnt; j++)
		{
			int64_t obj_ins = 0;
			engiGetAggrElement(obj_instances, j, sdaiINSTANCE, &obj_ins);
			if (!obj_ins)
				continue;

			char* ent_name = nullptr;
			engiGetEntityName(sdaiGetInstanceType(obj_ins), sdaiSTRING, &ent_name);
			IfcItem* obj_item = CreateObject(
				obj_ins,
				model,
				site_type,
				rel_contained_in_spatial_struct,
				rel_aggregates_type,
				rel_properties);
			obj_item->next = rel_properties->items;
			has_properties->items = obj_item;
		}
	}

	return has_properties;
}

IfcDecomposedBy * ReadIFC::CreateDecomposedBy(IfcItem * item,
	int_t model,
	int_t site_type,
	int_t rel_contained_in_spatial_struct,
	int_t rel_aggregates_type)
{
	IfcDecomposedBy* decomposed_by = nullptr;
	int_t* ifc_rel_decomposed_by = nullptr;
	int ifc_rel_decomposed_by_cnt = 0;

	sdaiGetAttrBN(item->instance, "IsDecomposedBy", sdaiAGGR, &ifc_rel_decomposed_by);
	if (nullptr == ifc_rel_decomposed_by)
		return nullptr;

	ifc_rel_decomposed_by_cnt = sdaiGetMemberCount(ifc_rel_decomposed_by);
	for (int i = 0; i < ifc_rel_decomposed_by_cnt; i++)
	{
		int_t rel_decomposed_by_ins = 0;
		engiGetAggrElement(ifc_rel_decomposed_by, i, sdaiINSTANCE, &rel_decomposed_by_ins);
		if (rel_aggregates_type != sdaiGetInstanceType(rel_decomposed_by_ins))
			continue;

		int_t* obj_instances = nullptr;
		sdaiGetAttrBN(rel_decomposed_by_ins, "RelatedObjects", sdaiAGGR, &obj_instances);
		int_t obj_ins_cnt = sdaiGetMemberCount(obj_instances);
		if (0 == obj_ins_cnt)
			continue;

		IfcDecomposedBy* decomposed_by_ins = CreateRelDecomposedBy(rel_decomposed_by_ins, model, item);
		decomposed_by_ins->next = decomposed_by;
		decomposed_by = decomposed_by_ins;

		for (int j = 0; j < obj_ins_cnt; j++)
		{
			int_t obj_ins = 0;
			engiGetAggrElement(obj_instances, j, sdaiINSTANCE, &obj_ins);

			char* ent_name = nullptr;
			engiGetEntityName(sdaiGetInstanceType(obj_ins), sdaiSTRING, &ent_name);
			IfcItem* item = CreateObject(
				obj_ins,
				model,
				site_type,
				rel_contained_in_spatial_struct,
				rel_aggregates_type,
				decomposed_by_ins);

			item->next = decomposed_by_ins->items;
			decomposed_by_ins->items = item;
		}
	}

	return decomposed_by;
}

IfcContains * ReadIFC::CreateRelationContains(int_t ins, int_t model, IfcItem * parent)
{
	IfcContains* contains = new IfcContains();

	contains->model = model;
	contains->items = nullptr;
	contains->parent = parent;
	contains->next = nullptr;

	return contains;
}

IfcHasProperty * ReadIFC::CreateRelationProperty(int_t ins, int_t model, IfcItem * parent)
{
	IfcHasProperty* properties = new IfcHasProperty();

	properties->model = model;
	properties->items = nullptr;
	properties->parent = parent;
	properties->next = nullptr;

	return properties;
}

IfcDecomposedBy * ReadIFC::CreateRelDecomposedBy(int_t ins, int_t model, IfcItem * parent)
{
	IfcDecomposedBy* decomposed_by = new IfcDecomposedBy();

	decomposed_by->model = model;
	decomposed_by->items = nullptr;
	decomposed_by->parent = parent;
	decomposed_by->next = nullptr;

	return decomposed_by;
}
