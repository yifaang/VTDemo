#include <ntddk.h>
#include "VtFunc.h"
VMX_Struct VMX_Mem;



NTSTATUS VMX_Mem_Control(DWORD32 Mem_Control) {		//VMX Memory Control  0:Allocate Memory 1:Free Memory
	switch (Mem_Control)
	{
	case 0:
		VMX_Mem.VMXON_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'vmx');
		VMX_Mem.VMCS_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'vmcs');
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

NTSTATUS Cr_Regester_Control(DWORD32 VMXE_Control) {
	__int64 Cr0_fixed0 = __readmsr(IA32_VMX_CR0_FIXED0);
	__int64 Cr0_fixed1 = __readmsr(IA32_VMX_CR0_FIXED1);
	__int64 Cr4_fixed0 = __readmsr(IA32_VMX_CR4_FIXED0);
	__int64 Cr4_fixed1 = __readmsr(IA32_VMX_CR4_FIXED1);
	_CR0 Cr0;
	_CR4 Cr4;
	*(PULONG64)&Cr0 = __readcr0();
	*(PULONG64)&Cr4 =  __readcr4() ;
	switch (VMXE_Control)
	{
	case 0:
		*(PULONG64)&Cr0 &= Cr0_fixed1;
		*(PULONG64)&Cr0 |= Cr0_fixed0;
		*(PULONG64)&Cr4 &= Cr4_fixed1;
		*(PULONG64)&Cr4 |= Cr4_fixed0;
		Cr4.VMXE = (unsigned)1;
		__writecr0(*(PULONG64)&Cr0);
		__writecr4(*(PULONG64)&Cr4);
		return STATUS_SUCCESS;
	case 1:
		__writecr0(*(PULONG64)&Cr0);
		__writecr4(0x0000000000350ef8);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS StartVT() {


	DbgPrint("VT Start ...\n");
	VMX_Mem_Control(0);
	Cr_Regester_Control(0);

	if (__vmx_on(&VMX_Mem.VMXON_Region_PA)) {
		DbgPrint("Entry VMX Fail");
	}
	return STATUS_SUCCESS;
}


NTSTATUS StopVt() {
	__vmx_off();
	Cr_Regester_Control(1);
	VMX_Mem_Control(1);
	return STATUS_SUCCESS;
}



void DriverUnload(PDRIVER_OBJECT driver)
{
	if (!VT_Enable())
	{
		DbgPrint("VT Already Close\n");
		VMX_Mem_Control(1);
		return;
	}
	StopVt();
	DbgPrint("Driver is unloading...\r\n");
	return;
}



NTSTATUS DriverEntry(PDRIVER_OBJECT  driver, PUNICODE_STRING RegistryPath)
{
	driver->DriverUnload = DriverUnload;
	if (!(Check_CPUID()&& Check_MSR()&& VT_Support()))//Check Support Cupid Instruction  MSR CR0 Lock
	{
		DbgPrint("CPU Don't Support");
		return STATUS_SUCCESS;
	}
	if (VT_Enable())
	{
		DbgPrint("VT Already Starting\n");
		return STATUS_SUCCESS;
	}
	StartVT();
	return STATUS_SUCCESS;
}