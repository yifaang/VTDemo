#include "vtdefine.h"
#include "vtstruct.h"
#include <intrin.h>
#include <ntddk.h>




USHORT RegGetCs();
USHORT RegGetDs();
USHORT RegGetEs();
USHORT RegGetSs();
USHORT RegGetFs();
USHORT RegGetGs();



ULONG64 GetIdtBase();
USHORT  GetIdtLimit();
ULONG64 GetGdtBase();
USHORT  GetGdtLimit();
ULONG64 RegGetRflags();

USHORT GetTrSelector();

void Asm_vmcall();



__declspec(noinline) void   ExithandlerPoint() {
	int VMX_Exit_Reason = 0;
	__vmx_vmread(VM_EXIT_REASON, &VMX_Exit_Reason);
	DbgPrint("VM_EXIT_REASON %p\n", VMX_Exit_Reason);
	__debugbreak();
}


__declspec(noinline) void VmxGuest() {
	Asm_vmcall();
}



//Check CPU support Virtual Technology
BOOLEAN Check_CPUID() {
	int Ecx[4];
	__cpuid(Ecx, 1);
	return (Ecx[2] >> 5) & 1;	//Ecx 第六位是否为1
}
//Check VT-x Enable
BOOLEAN Check_MSR() {
	__int64 VMX_Control_feild = __readmsr(MSR_IA32_FEATURE_CONTROL);
	return (VMX_Control_feild % 2);
}

//Check VT is Enable
BOOLEAN VT_Support() {
	_CR0 cr0;
	*(PULONG64)&cr0 =__readcr0();
	if (cr0.PE==1&&cr0.NE==1&&cr0.PG==1)
	{
		return TRUE;
	}
	return FALSE;
}

BOOLEAN VT_Enable() {
	_CR4 cr4;
	*(PULONG64)&cr4 = __readcr4();
	if (cr4.VMXE==1)
	{
		return TRUE;
	}
	return FALSE;
}

