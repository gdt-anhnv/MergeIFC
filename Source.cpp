#include "read_ifc.h"

#include "ifcengine/include/engine.h"
#include "ifcengine/include/ifcengine.h"

#include <iostream>
#include <string>

int main()
{
	ReadIFC read_ifc(L"C:\\Users\\nguye\\Desktop\\1plant.ifc");
	read_ifc.Parse();

	return 0;
}