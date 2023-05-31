#include <ntddk.h>
#include "CheckEnable.h"

VMX_Struct VMX_Mem;


BOOLEAN CheckEnable() {				//Check Support Cupid Instruction  MSR CR4 Lock
	if (Check_CPUID()||Check_MSR()|| VT_Enable())
	{
		return TRUE;
	}
	return FALSE;
}


NTSTATUS VMX_Mem_Control(DWORD32 Mem_Control) {		//VMX Memory Control  0:Allocate Memory 1:Free Memory
	switch (Mem_Control)
	{
	case 0:
		VMX_Mem.VMXON_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000,'vmx');
		VMX_Mem.VMCS_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000,'vmcs');
		VMX_Mem.VMXON_Region_PA = MmGetPhysicalAddress(VMX_Mem.VMXON_Region);
		VMX_Mem.VMCS_Region_PA = MmGetPhysicalAddress(VMX_Mem.VMCS_Region);
		RtlZeroMemory(VMX_Mem.VMXON_Region, 0x1000);
		RtlZeroMemory(VMX_Mem.VMCS_Region, 0x1000);
		*(PULONG)VMX_Mem.VMXON_Region = 1;
		*(PULONG)VMX_Mem.VMCS_Region = 1;
		return STATUS_SUCCESS;
	case 1:
		ExFreePoolWithTag(VMX_Mem.VMXON_Region, 'vmx');
		ExFreePoolWithTag(VMX_Mem.VMCS_Region, 'vmcs');
		return STATUS_SUCCESS;
	default:
		DbgPrint("Mem_Control Feilds is Wrong\n");
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS Cr4_Control() {

}


NTSTATUS StartVT() {
	//Check VT Support
	if (!CheckEnable())
	{
		//DbgPrint("Check Vt Is Not Support");
		return STATUS_UNSUCCESSFUL;
	}
	VMX_Mem_Control(0);
	Cr4_Control();
	return STATUS_SUCCESS;
}


NTSTATUS StopVt() {
	VMX_Mem_Control(1);
	return STATUS_SUCCESS;
}




NTSTATUS DriverUnload() {
	StopVt();
	DbgPrint("Driver Unload Successful");
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pdriver, PUNICODE_STRING RegistryPath) {
	StartVT();
	pdriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}