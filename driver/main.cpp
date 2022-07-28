#include "logger.hpp"

VOID DriverUnload(PDRIVER_OBJECT drv_object)
{
	DBG_PRINT("unload point called");
	logger::delete_registry_routine();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT drv_object, PUNICODE_STRING reg_path)
{
	DBG_PRINT("entry point called");
	drv_object->DriverUnload = DriverUnload;
	return logger::install_registry_routine(drv_object);
}