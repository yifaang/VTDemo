#include "vtstruct.h"
#include <intrin.h>
#include "vtdefine.h"
#include <ntddk.h>

//Check CPU support Virtual Technology
BOOLEAN Check_CPUID() {
	int Ecx[4];
	__cpuid(Ecx,1);
	return (Ecx[2] >> 5) & 1;	//Ecx 第六位是否为1
}
//Check VT-x Enable
BOOLEAN Check_MSR() {
	__int64 VMX_Control_feild = __readmsr(MSR_IA32_FEATURE_CONTROL);
	return (VMX_Control_feild % 2);
}

//Check VT is Enable
BOOLEAN VT_Enable() {
	_CR0* cr0 = (_CR0*)__readcr0();
	_CR4* cr4 = (_CR4*)__readcr4();
	if (cr0->PE==1 &&cr0->NE==1&&cr0->PG == 1)
	{
		if (cr4->VMXE==1)  //VMX Lock
		{
			return TRUE;
		}
	}
	return FALSE;
}

