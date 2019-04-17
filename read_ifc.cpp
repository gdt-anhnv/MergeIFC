#include "read_ifc.h"
#include "ifc_struct.h"

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

ReadIFC::ReadIFC(const std::wstring & fn) :
	filename(fn),
	ifc_model(nullptr),
	model(0)
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
	wchar_t* schema = new wchar_t[wcslen(L"D:\\Team TP HCM\\Projects\\MergeIFC\\x64\\Debug\\IFC2X3_TC1.exp") + 1];
	memcpy(&schema[0], L"D:\\Team TP HCM\\Projects\\MergeIFC\\x64\\Debug\\IFC2X3_TC1.exp", sizeof(wchar_t));

	wchar_t* fp = new wchar_t[wcslen(filename.c_str()) + 1];
	memcpy(fp, filename.c_str(), sizeof(wchar_t));

	int64_t model = sdaiOpenModelBNUnicode(0, fp, schema);
	delete[] schema;
	delete[] fp;

	if (!model)
		return;

	ifc_model = CreateProject();
}

IfcItem* ReadIFC::CreateProject()
{
	rel_aggregates_type = sdaiGetEntity(model, "IFCRELAGGREGATES");
	rel_contained_in_spatial_struct = sdaiGetEntity(model, "IFCRELCONTAINEDINSPATIALSTRUCTURE");
	site_type = sdaiGetEntity(model, "IFCSITE");

	IfcItem* project_items = nullptr;

	int_t* proj_instances = sdaiGetEntityExtentBN(model, "IFCPROJECT");
	int_t num_proj_instances = sdaiGetMemberCount(proj_instances);

	for (int i = 0; i < num_proj_instances; i++)
	{
		int proj_ins = 0;
		engiGetAggrElement(proj_instances, i, sdaiINSTANCE, &proj_ins);
		IfcItem* proj_item = CreateObject(proj_ins, nullptr);

		proj_item->next = project_items;
		project_items = proj_item;
	}

	return project_items;
}

IfcItem * ReadIFC::CreateObject(int64_t obj_ins, IfcConnection * parent)
{
	IfcItem* item = CreateItem(obj_ins, parent);
	item->contains = CreateContains(item);
	item->decomposed_by = CreateDecomposedBy(item);

	return item;
}

IfcItem * ReadIFC::CreateItem(int64_t instance, IfcConnection * parent)
{
	IfcItem* item = new IfcItem();

	item->model = model;
	item->instance = instance;
	item->entity = sdaiGetInstanceType(instance);
	item->global_id = nullptr;
	sdaiGetAttrBN(instance, "GlobalId", sdaiSTRING, &item->global_id);
	item->is_base = site_type == item->entity;

	item->contains = nullptr;
	item->decomposed_by = nullptr;

	item->parent = parent;
	item->next = nullptr;

	return item;
}

IfcContains * ReadIFC::CreateContains(IfcItem * item)
{
	IfcContains* contains = nullptr;
	int64_t* rel_spatials = nullptr;
	sdaiGetAttrBN(item->instance, "ContainsElements", sdaiAGGR, &rel_spatials);
	int64_t rel_cnt = sdaiGetMemberCount(rel_spatials);

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

		IfcContains* rel_contains = CreateRelationContains(rel_spatial_ins, item);
		rel_contains->next = contains;
		contains = rel_contains;

		for (int j = 0; j < obj_ins_cnt; j++)
		{
			int64_t obj_ins = 0;
			engiGetAggrElement(obj_instances, j, sdaiINSTANCE, &obj_ins);
			if (!obj_ins)
				continue;

			IfcItem* obj_item = CreateObject(obj_ins, rel_contains);
			obj_item->next = rel_contains->items;
			contains->items = obj_item;
		}
	}

	return contains;
}

IfcDecomposedBy * ReadIFC::CreateDecomposedBy(IfcItem * item)
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

		IfcDecomposedBy* decomposed_by_ins = CreateRelDecomposedBy(rel_decomposed_by_ins, item);
		decomposed_by_ins->next = decomposed_by;
		decomposed_by = decomposed_by_ins;

		for (int j = 0; j < obj_ins_cnt; j++)
		{
			int_t obj_ins = 0;
			engiGetAggrElement(obj_instances, j, sdaiINSTANCE, &obj_ins);
			IfcItem* item = CreateObject(obj_ins, decomposed_by_ins);

			item->next = decomposed_by_ins->items;
			decomposed_by_ins->items = item;
		}
	}

	return decomposed_by;
}

IfcContains * ReadIFC::CreateRelationContains(int64_t ins, IfcItem * parent)
{
	IfcContains* contains = new IfcContains();

	contains->model = model;
	contains->items = nullptr;
	contains->parent = parent;
	contains->next = nullptr;

	return contains;
}

IfcDecomposedBy * ReadIFC::CreateRelDecomposedBy(int64_t ins, IfcItem * parent)
{
	IfcDecomposedBy* decomposed_by = new IfcDecomposedBy();

	decomposed_by->model = model;
	decomposed_by->items = nullptr;
	decomposed_by->parent = parent;
	decomposed_by->next = nullptr;

	return decomposed_by;
}
