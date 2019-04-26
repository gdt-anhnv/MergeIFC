#include "read_ifc.h"
#include "ifc_struct.h"

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

#include <iostream>
#include <string>

void Merge(IfcItem* fitem, IfcItem* sitem);
int main()
{
	ReadIFC read_ifc(L"C:\\Users\\nguye\\Desktop\\structure2.ifc");
	read_ifc.Parse();

	ReadIFC read_ifc2(L"C:\\Users\\nguye\\Desktop\\endplate.ifc");
	read_ifc2.Parse();

	Merge(read_ifc.GetStructure(), read_ifc2.GetStructure());

	return 0;
}

static void CopyIfcInstance(int_t des_model, int_t source_model, int_t ins, int_t styled_item);
void Merge(IfcItem * fitem, IfcItem * sitem)
{
	while (sitem)
	{
		int_t ins = 0;
		owlBuildInstance(sitem->model, sitem->instance, &ins);
		if (!ins)
			continue;

		int_t vertice_size = 0;
		int_t index_ver = 0;
		CalculateInstance(ins, &vertice_size, &index_ver, nullptr);

		if (!vertice_size || !index_ver)
			continue;

	}
}

void CopyIfcInstance(int_t des_model, int_t source_model, int_t ins, int_t styled_item)
{
	int_t ifc_ent = sdaiGetInstanceType(ins);
	int_t no_arg = engiGetEntityNoArguments(ins);

	char* ent_name = nullptr;
	engiGetEntityName(ins, sdaiSTRING, &ent_name);
	int_t copy_ins = sdaiCreateInstanceBN(des_model, ent_name);

	for (int i = 0; i < no_arg; i++)
	{
		char* att_name = nullptr;
		engiGetEntityArgumentName(ifc_ent, i, sdaiSTRING, &att_name);
		switch (engiGetAttrTypeBN(ifc_ent, att_name))
		{
		case sdaiADB:
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
					double real_val = 0.0;
					sdaiGetADBValue(value_adb, sdaiREAL, &real_val);
					int_t* val_adb_target = (int_t*)sdaiCreateADB(sdaiREAL, (void*)&real_val);
					sdaiPutADBTypePath(val_adb_target, 1, type_path);
					sdaiPutAttrBN(copy_ins, att_name, sdaiADB, val_adb_target);
					break;
				default:
					break;
				}
			}
		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
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
		case sdaiINTEGER:
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

	}
}
