#pragma once
#include <ntifs.h>
#include <stdint.h>
#include <wchar.h>

#define DBG_PRINT(s, ...)	DbgPrintEx(77, 0, "[gawaln] %s\n", s, ##__VA_ARGS__)

namespace logger
{
	UNICODE_STRING registry_path[];
	inline LARGE_INTEGER cookie{};

	NTSTATUS install_registry_routine(PDRIVER_OBJECT drv_object);
	NTSTATUS registry_callback(PVOID callback_context, PVOID arg1, PVOID arg2);
	BOOLEAN check_registry(PVOID root_object, PUNICODE_STRING complete_name);
	BOOLEAN check_eqaul_path(PUNICODE_STRING key_path);

	NTSTATUS delete_registry_routine();
}