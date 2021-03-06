#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"
#include "ifc_struct.h"
#include "read_ifc.h"

#include <iostream>
#include <string>
#include "merge_ifc.h"

static int_t CopyIfcInstance(int_t des_model, int_t source_model, int_t ins, int_t styled_item);
static void Explore(IfcItem* fitem, int_t site_type, IfcItem* sitem);
void Merge(ReadIFC& fifc, ReadIFC& sifc)
{
	IfcItem* fitem = fifc.GetStructure();
	IfcItem* sitem = sifc.GetStructure();
	Explore(fitem, fifc.site_type, sitem);
}

static std::wstring GetSchemaFile(int_t model)
{
	wchar_t * text = 0;
	__int64 i = 0;
	if (!GetSPFFHeaderItem(model, 9, i, sdaiUNICODE, (char **)&text)) {
		while (!GetSPFFHeaderItem(model, 9, i++, sdaiUNICODE, (char **)&text)) {
			text = 0;
		}
		return std::wstring(L"");
	}

	return std::wstring(text);
}
bool IsSameIfcVersion(const std::list<int_t>& models)
{
	if (0 == models.size())
		return false;

	std::wstring schema = GetSchemaFile(models.front());
	for (std::list<int_t>::const_iterator iter = models.begin(); iter != models.end(); iter++)
	{
		if (0 != schema.compare(GetSchemaFile(*iter)))
			return false;
	}

	return true;
}

int_t CopyIfcInstance(int_t des_model, int_t source_model, int_t ins, int_t styled_item)
{
	int_t ifc_ent = sdaiGetInstanceType(ins);
	int_t no_arg = engiGetEntityNoArguments(ifc_ent);

	char* ent_name = nullptr;
	engiGetEntityName(ifc_ent, sdaiSTRING, &ent_name);
	int_t copy_ins = sdaiCreateInstanceBN(des_model, ent_name);

	int_t* style_item_aggr = nullptr;
	sdaiGetAttrBN(ins, "StyledByItem", sdaiAGGR, &style_item_aggr);
	int_t style_item_cnt = sdaiGetMemberCount(style_item_aggr);
	for (int i = 0; i < style_item_cnt; i++)
	{
		int_t style_item = 0;
		engiGetAggrElement(style_item_aggr, i, sdaiINSTANCE, &style_item);
		int_t copy_style_item = CopyIfcInstance(des_model, source_model, style_item, true);
		sdaiPutAttrBN(copy_style_item, "Item", sdaiINSTANCE, (void*)copy_ins);
	}

	for (int i = 0; i < no_arg; i++)
	{
		char* att_name = nullptr;
		engiGetEntityArgumentName(ifc_ent, i, sdaiSTRING, &att_name);
		int_t att_type = engiGetAttrTypeBN(ins, att_name);
		switch (att_type)
		{
		case sdaiADB:
		{
			int_t no_line = internalGetP21Line(ins);
			int_t* value_adb = nullptr;
			sdaiGetAttrBN(ins, att_name, sdaiADB, &value_adb);
			if (value_adb)
			{
				char* type_path = sdaiGetADBTypePath(value_adb, 0);
				int_t adb_type = sdaiGetADBType(value_adb);
				switch (adb_type)
				{
				case sdaiREAL:
				{
					double real_val = 0.0;
					sdaiGetADBValue(value_adb, sdaiREAL, &real_val);
					int_t* val_adb_target = (int_t*)sdaiCreateADB(sdaiREAL, (void*)&real_val);
					sdaiPutADBTypePath(val_adb_target, 1, type_path);
					sdaiPutAttrBN(copy_ins, att_name, sdaiADB, val_adb_target);
					break;
				}
				default:
					break;
				}
			}
			break;
		}
		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			char* enum_val = nullptr;
			sdaiGetAttrBN(ins, att_name, sdaiSTRING, &enum_val);
			if (enum_val)
			{
				size_t len = strlen(enum_val);
				char* buff = new char[len];
				memcpy(buff, &enum_val[1], len - 2);
				buff[len - 2] = 0;
				sdaiPutAttrBN(copy_ins, att_name, sdaiENUM, (void*)buff);
				delete[] buff;
			}
			break;
		}
		case sdaiINTEGER:
		{
			int int_val = 0;
			sdaiGetAttrBN(ins, att_name, sdaiINTEGER, &int_val);
			if (int_val)
				sdaiPutAttrBN(copy_ins, att_name, sdaiINTEGER, &int_val);
			else
			{
				int_val = 1;
				sdaiGetAttrBN(ins, att_name, sdaiINTEGER, &int_val);
				if (0 == int_val)
					sdaiPutAttrBN(copy_ins, att_name, sdaiINTEGER, &int_val);
			}
			break;
		}
		case sdaiREAL:
		{
			double real_val = 0;
			sdaiGetAttrBN(ins, att_name, sdaiREAL, &real_val);
			if (real_val)
			{
				sdaiPutAttrBN(copy_ins, att_name, sdaiREAL, &real_val);
			}
			else
			{
				real_val = 1;
				sdaiGetAttrBN(ins, att_name, sdaiREAL, &real_val);
				if (0 == real_val)
					sdaiPutAttrBN(copy_ins, att_name, sdaiREAL, &real_val);
			}
			break;
		}
		case sdaiINSTANCE:
		{
			if (styled_item && 0 == std::string(att_name).compare((char*)"Item"))
				continue;

			int_t content = 0;
			sdaiGetAttrBN(ins, att_name, sdaiINSTANCE, &content);
			if (content)
			{
				content = CopyIfcInstance(des_model, source_model, content, false);
				sdaiPutAttrBN(copy_ins, att_name, sdaiINSTANCE, (void*)content);
			}
			break;
		}
		case sdaiSTRING:
		{
			wchar_t* txt_content = nullptr;
			sdaiGetAttrBN(ins, att_name, sdaiUNICODE, &txt_content);
			if (txt_content)
				sdaiPutAttrBN(copy_ins, att_name, sdaiUNICODE, (void*)txt_content);
			break;
		}
		case sdaiADB + 128:
		{
			int_t line_no = internalGetP21Line(ins);
			int_t* aggr = sdaiCreateAggrBN(copy_ins, att_name);
			int_t* aggr_data = 0;
			sdaiGetAttrBN(ins, att_name, sdaiAGGR, &aggr_data);
			int_t aggr_cnt = sdaiGetMemberCount(aggr_data);
			for (int i = 0; i < aggr_cnt; i++)
			{
				int_t* value_adb = nullptr;
				engiGetAggrElement(aggr, i, sdaiADB, &value_adb);
				if (value_adb)
				{
					char* type_path = sdaiGetADBTypePath(value_adb, 0);
					int_t adb_type = sdaiGetADBType(value_adb);
					switch (adb_type)
					{
					case sdaiREAL:
						double real_val = 0;
						sdaiGetADBValue(value_adb, sdaiREAL, &real_val);
						int line_no = internalGetP21Line(copy_ins);
						int_t* value_adb_target = (int_t*)sdaiCreateADB(sdaiREAL, (void*)&real_val);
						sdaiPutADBTypePath(value_adb_target, 1, type_path);
						sdaiPutAttrBN(copy_ins, att_name, sdaiADB, value_adb_target);
						break;
					}
				}
			}
			break;
		}
		case sdaiINSTANCE + 128:
		{
			int_t att_type = 0;
			engiGetEntityArgumentType(ifc_ent, i, &att_type);

			char* name = nullptr;
			int_t inverse = 0;
			engiGetEntityProperty(ifc_ent, i, &name, 0, 0, 0, 0, 0, 0, 0, 0, 0, &inverse);
			if (0 == inverse)
			{
				int_t* my_aggr = sdaiCreateAggrBN(copy_ins, att_name);
				if (nullptr == my_aggr)
					break;
				int_t* aggr = 0;
				sdaiGetAttrBN(ins, att_name, sdaiAGGR, &aggr);
				int aggr_cnt = sdaiGetMemberCount(aggr);
				for (int i = 0; i < aggr_cnt; i++)
				{
					int_t content = 0;
					engiGetAggrElement(aggr, i, sdaiINSTANCE, &content);
					if (content)
					{
						content = CopyIfcInstance(des_model, source_model, content, false);
						sdaiAppend((int_t)my_aggr, sdaiINSTANCE, (void*)content);
					}
				}
			}
			break;
		}
		case sdaiSTRING + 128:
		{
			int_t* my_aggr = sdaiCreateAggrBN(copy_ins, att_name);
			int_t* aggr = 0;
			sdaiGetAttrBN(ins, att_name, sdaiAGGR, &aggr);
			int_t aggr_cnt = sdaiGetMemberCount(aggr);
			for (int_t i = 0; i < aggr_cnt; i++)
			{
				wchar_t* value = nullptr;
				engiGetAggrElement(aggr, i, sdaiUNICODE, &value);
				sdaiAppend((int_t)my_aggr, sdaiUNICODE, value);
			}
			break;
		}
		case sdaiREAL + 128:
		{
			int_t* my_aggr = sdaiCreateAggrBN(copy_ins, att_name);
			int_t* aggr = 0;
			sdaiGetAttrBN(ins, att_name, sdaiAGGR, &aggr);
			int_t aggr_cnt = sdaiGetMemberCount(aggr);
			for (int_t i = 0; i < aggr_cnt; i++)
			{
				wchar_t	* value = nullptr;
				engiGetAggrElement(aggr, i, sdaiREAL, &value);

				sdaiAppend((int_t)my_aggr, sdaiREAL, &value);
			}
			break;
		}
		default:
			break;
		}
	}

	return copy_ins;
}

void ExploreContains(
	IfcItem * item,
	int_t site_type,
	IfcContains* contains)
{
	while (contains)
	{
		Explore(item, site_type, contains->items);
		contains = contains->next;
	}
}

void ExploreDecomposedBy(
	IfcItem * item,
	int_t site_type,
	IfcDecomposedBy* decomposed_by)
{
	while (decomposed_by)
	{
		Explore(item, site_type, decomposed_by->items);
		decomposed_by = decomposed_by->next;
	}
}

IfcItem* FitElement(IfcItem* fitem, int_t site_type, int_t ins, IfcItem* sitem);
void Explore(IfcItem * fitem, int_t site_type, IfcItem * sitem)
{
	while (sitem)
	{
		ExploreContains(fitem, site_type, dynamic_cast<IfcContains*>(sitem->contains));
		ExploreDecomposedBy(fitem, site_type, dynamic_cast<IfcDecomposedBy*>(sitem->decomposed_by));

		int_t ins = 0;
		owlBuildInstance(sitem->model, sitem->instance, &ins);
		if (!ins)
		{
			sitem = sitem->next;
			continue;
		}

		int_t vertice_size = 0;
		int_t index_ver = 0;
		CalculateInstance(ins, &vertice_size, &index_ver, nullptr);

		if (!vertice_size || !index_ver)
		{
			sitem = sitem->next;
			continue;
		}

		int_t copy_ins = CopyIfcInstance(fitem->model, sitem->model, sitem->instance, false);
		FitElement(fitem, site_type, copy_ins, sitem);

		sitem = sitem->next;
	}
}

IfcItem* FindInstanceByGlobalId(IfcItem* item, char* global_id);
IfcItem* FindInstanceIsBase(IfcItem* item);
IfcItem* FitElement(IfcItem* fitem, int_t site_type, int_t copy_ins, IfcItem* sitem)
{
	if (nullptr == sitem->parent || nullptr == sitem->parent->parent)
		return sitem;

	IfcItem* connected_item = FindInstanceByGlobalId(fitem, sitem->parent->parent->global_id);
	IfcItem* parent_item = nullptr;

	if (nullptr == connected_item)
	{
		if (sitem->parent->parent->is_base)
			connected_item = FindInstanceIsBase(fitem);
		else
		{
			int_t parent_ins = CopyIfcInstance(
				fitem->model,
				sitem->model,
				sitem->parent->parent->instance, false);

			connected_item = ReadIFC::CreateItem(fitem->model, parent_ins, site_type, nullptr);
			parent_item = FitElement(fitem, site_type, parent_ins, sitem->parent->parent);
		}
	}

	IfcContains* contains = dynamic_cast<IfcContains*>(sitem->parent);
	if (nullptr != contains)
	{
		int_t rel_contained_spatial = sdaiCreateInstanceBN(fitem->model, "IFCRELCONTAINEDINSPATIALSTRUCTURE");
		sdaiPutAttrBN(rel_contained_spatial, "RelatingStructure", sdaiINSTANCE, (void*)connected_item->instance);
		int_t* my_aggr = sdaiCreateAggrBN(rel_contained_spatial, "RelatedElements");
		sdaiAppend((int_t)my_aggr, sdaiINSTANCE, (void*)copy_ins);
		if (parent_item)
		{
			IfcContains* sub_contains = ReadIFC::CreateRelationContains(fitem->instance, fitem->model, parent_item);
			sub_contains->next = dynamic_cast<IfcContains*>(parent_item->contains);
			parent_item->contains = sub_contains;
			sub_contains->items = connected_item;
		}
	}

	IfcDecomposedBy* decomposed_by = dynamic_cast<IfcDecomposedBy*>(sitem->parent);
	if (nullptr != decomposed_by)
	{
		int_t rel_aggregates_ins = sdaiCreateInstanceBN(fitem->model, "IFCRELAGGREGATES");
		sdaiPutAttrBN(rel_aggregates_ins, "RelatingObject", sdaiINSTANCE, (void*)connected_item->instance);
		int_t* my_aggr = sdaiCreateAggrBN(rel_aggregates_ins, "RelatedObjects");
		sdaiAppend((int_t)my_aggr, sdaiINSTANCE, (void*)copy_ins);
		if (parent_item)
		{
			IfcDecomposedBy* decomposed_by = ReadIFC::CreateRelDecomposedBy(copy_ins, fitem->model, parent_item);
			decomposed_by->next = dynamic_cast<IfcDecomposedBy*>(parent_item->decomposed_by);
			parent_item->decomposed_by = decomposed_by;
			decomposed_by->items = connected_item;
		}
	}

	IfcHasProperty* has_properties = dynamic_cast<IfcHasProperty*>(sitem->parent);
	if (nullptr != has_properties)
	{
		int_t rel_prop_ins = sdaiCreateInstanceBN(fitem->model, "IFCRELDEFINESBYPROPERTIES");
		sdaiPutAttrBN(rel_prop_ins, "RelatingPropertyDefinition", sdaiINSTANCE, (void*)connected_item->instance);
		int_t* my_aggr = sdaiCreateAggrBN(rel_prop_ins, "RelatedObjects");
		sdaiAppend((int_t)my_aggr, sdaiINSTANCE, (void*)copy_ins);
		if (parent_item)
		{
			IfcHasProperty* prop = ReadIFC::CreateRelationProperty(copy_ins, fitem->model, parent_item);
			prop->next = dynamic_cast<IfcHasProperty*>(parent_item->has_properties);
			parent_item->has_properties = prop;
			has_properties->items = connected_item;
		}
	}

	return connected_item;
}

IfcItem* FindInstanceByGlobalId(IfcItem* item, char* global_id)
{
	while (item)
	{
		IfcContains* contains = (IfcContains*)item->contains;
		while (contains)
		{
			IfcItem* value = FindInstanceByGlobalId(contains->items, global_id);
			if (value)
				return value;

			contains = contains->next;
		}

		IfcDecomposedBy* decomposed_by = (IfcDecomposedBy*)item->decomposed_by;
		while (decomposed_by)
		{
			IfcItem* value = FindInstanceByGlobalId(decomposed_by->items, global_id);
			if (value)
				return value;

			decomposed_by = decomposed_by->next;
		}

		if (0 == std::string(global_id).compare(item->global_id))
			return item;

		item = item->next;
	}

	return nullptr;
}

IfcItem * FindInstanceIsBase(IfcItem * item)
{
	while (item)
	{
		if (item->is_base)
			return item;

		IfcContains* contains = (IfcContains*)item->contains;
		while (contains)
		{
			IfcItem* value = FindInstanceIsBase(contains->items);
			if (value)
				return value;
			contains = contains->next;
		}

		IfcDecomposedBy* decomposed_by = (IfcDecomposedBy*)item->decomposed_by;
		while (decomposed_by)
		{
			IfcItem* value = FindInstanceIsBase(decomposed_by->items);
			if (value)
				return value;
			decomposed_by = decomposed_by->next;
		}

		item = item->next;
	}

	return nullptr;
}
