#include "PCH.h"

#include "InterfaceExample.h"
#include "Problem.h"
#include "SQLiteDatabase.h"

#include "Global.h"

int main()
{
	interfaceMain();
	
	FaultNet_Sim::Problem::Join();
	FaultNet_Sim::SQLiteDatabase::Get()->Join();

	return 0;
}
