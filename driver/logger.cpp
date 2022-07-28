#include "logger.hpp"

UNICODE_STRING logger::registry_path[] = {
	RTL_CONSTANT_STRING(L"\\REGISTRY\\MACHINE\\System\\ControlSet001\\Control"),
	RTL_CONSTANT_STRING(L"\\REGISTRY\\MACHINE\\System\\ControlSet001\\Enum")
};

NTSTATUS logger::install_registry_routine(PDRIVER_OBJECT drv_object)
{
	UNICODE_STRING altitude{};
	RtlInitUnicodeString(&altitude, L"136800");

	if (!NT_SUCCESS(CmRegisterCallbackEx(
		logger::registry_callback,
		&altitude,
		drv_object,
		nullptr,
		&logger::cookie,
		nullptr
	)))
	{
		DBG_PRINT("Failed CmRegisterCallbackEx");
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		DBG_PRINT("Success CmRegisterCallbackEx");
		return STATUS_SUCCESS;
	}
}

NTSTATUS logger::registry_callback(PVOID callback_context, PVOID arg1, PVOID arg2)
{
	REG_NOTIFY_CLASS notify_class = (REG_NOTIFY_CLASS)((ULONG_PTR)arg1);

	if (notify_class == RegNtPreOpenKeyEx)
	{
		PREG_OPEN_KEY_INFORMATION_V1 open_key_info = (PREG_OPEN_KEY_INFORMATION_V1)arg2;
		if (check_registry(open_key_info->RootObject, open_key_info->CompleteName))
		{
			DBG_PRINT("[ callback ] Success prevent!");
			return STATUS_ACCESS_DENIED;
		}
	}
}

BOOLEAN logger::check_registry(PVOID root_object, PUNICODE_STRING complete_name)
{
	PCUNICODE_STRING root_object_name;
	ULONG_PTR root_object_id;
	UNICODE_STRING key_path{ 0 };

	if (root_object)
	{
		// CmCallbackGetKeyObjectID 함수를 통해 root_object를 통해 root_object_name에 경로를 저장.
		if (!NT_SUCCESS(CmCallbackGetKeyObjectID(&logger::cookie, root_object, &root_object_id, &root_object_name)))
		{
			DBG_PRINT("Failed CmCallbackGetKeyObjectID");
			return FALSE;
		}

		// 만약 complete_name이 유효하다면 root_object_name과 complete_name의 문자열을 합친다.
		if (complete_name->Length && complete_name->Buffer)
		{
			key_path.MaximumLength = root_object_name->Length + complete_name->Length + (sizeof(WCHAR) * 2);
			key_path.Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, key_path.MaximumLength, 'awag');

			if (!key_path.Buffer) {
				DBG_PRINT("Failed ExAllocatePoolWithTag");
				return FALSE;
			}

			swprintf(key_path.Buffer, L"%wZ\\%wZ", root_object_name, complete_name);
			key_path.Length = root_object_name->Length + complete_name->Length + (sizeof(WCHAR));

			return check_eqaul_path(&key_path);

		}
	}
	else
	{
		// complete_name은 절대 경로 또는 상대 경로를 가지는데 root_object가 유효하지 않으면 complete_name에 전체 경로가 있음을 의미.
		return check_eqaul_path(complete_name);

	}
	if (key_path.Buffer) ExFreePoolWithTag(key_path.Buffer, 'awag');
	return FALSE;
}

BOOLEAN logger::check_eqaul_path(PUNICODE_STRING key_path)
{
	BOOLEAN matched = FALSE;
	ULONG count = sizeof(logger::registry_path) / sizeof(UNICODE_STRING);

	for (ULONG i = 0; i < count; ++i)
	{
		if (RtlEqualUnicodeString(key_path, &logger::registry_path[i], TRUE))
		{
			matched = TRUE;
			break;
		}
	}

	return matched;
}

NTSTATUS logger::delete_registry_routine()
{
	NTSTATUS status;

	status = CmUnRegisterCallback(logger::cookie);
	if (!NT_SUCCESS(status))
	{
		DBG_PRINT("Failed CmUnRegisterCallback\n");
		return status;
	}
	else
	{
		DBG_PRINT("Success CmUnRegisterCallback\n");
		return status;
	}
}