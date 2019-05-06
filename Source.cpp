#include "read_ifc.h"
#include "ifc_struct.h"
#include "merge_ifc.h"

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

#include <iostream>
#include <string>

int main()
{
	ReadIFC read_ifc(L"C:\\Users\\nguye\\Desktop\\structure2.ifc");
	read_ifc.Parse();

	ReadIFC read_ifc2(L"C:\\Users\\nguye\\Desktop\\endplate.ifc");
	read_ifc2.Parse();

	Merge(read_ifc, read_ifc2);

	sdaiSaveModelBNUnicode(read_ifc.GetStructure()->model, L"C:\\Users\\nguye\\Desktop\\structure21.ifc");

	read_ifc.Release();
	read_ifc2.Release();

	return 0;
}

