#include <ntddk.h>
#include "VtFunc.h"
VMX_Struct VMX_Mem;






static ULONG  VmxAdjustControls(ULONG Ctl, ULONG Msr)
{
	LARGE_INTEGER MsrValue;
	MsrValue.QuadPart = (ULONG64)__readmsr(Msr);
	Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
	Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
	//DbgPrint("Msr %p \t %X\n", Msr,Ctl);
	return Ctl;
}

void VMCS_Setup() {
	SEGMENT_SELECTOR SegmentSelector;
	ULONG64 GdtBase, IdtBase;
	GdtBase = GetGdtBase();
	IdtBase = GetIdtBase();

	//1.Guest state Fields
	__vmx_vmwrite(GUEST_CR0, __readcr0());
	__vmx_vmwrite(GUEST_CR3, __readcr3());
	__vmx_vmwrite(GUEST_CR4, __readcr4());
	
	__vmx_vmwrite(GUEST_DR7, 0x400);
	__vmx_vmwrite(GUEST_RFLAGS, RegGetRflags() & ~0x200); //cli
	
	__vmx_vmwrite(GUEST_ES_SELECTOR, RegGetEs() & 0xfff8);
	__vmx_vmwrite(GUEST_CS_SELECTOR, RegGetCs() & 0xfff8);
	__vmx_vmwrite(GUEST_DS_SELECTOR, RegGetDs() & 0xfff8);
	__vmx_vmwrite(GUEST_FS_SELECTOR, RegGetFs() & 0xfff8);
	__vmx_vmwrite(GUEST_GS_SELECTOR, RegGetGs() & 0xfff8);
	__vmx_vmwrite(GUEST_SS_SELECTOR, RegGetSs() & 0xfff8);
	__vmx_vmwrite(GUEST_TR_SELECTOR, GetTrSelector() & 0xfff8);

	__vmx_vmwrite(GUEST_ES_AR_BYTES, 0x10000);
	__vmx_vmwrite(GUEST_FS_AR_BYTES, 0x10000);
	__vmx_vmwrite(GUEST_DS_AR_BYTES, 0x10000);
	__vmx_vmwrite(GUEST_SS_AR_BYTES, 0x10000);
	__vmx_vmwrite(GUEST_GS_AR_BYTES, 0x10000);
	__vmx_vmwrite(GUEST_LDTR_AR_BYTES, 0x10000);

	__vmx_vmwrite(GUEST_CS_AR_BYTES, 0x209b);
	__vmx_vmwrite(GUEST_CS_BASE,0x0);
	__vmx_vmwrite(GUEST_CS_LIMIT,0x0);

	__vmx_vmwrite(GUEST_TR_AR_BYTES,0x008b);
	__vmx_vmwrite(GUEST_TR_BASE, 0xffffa700857f6000);
	__vmx_vmwrite(GUEST_TR_LIMIT,0x0067);

	__vmx_vmwrite(GUEST_GDTR_BASE, (ULONG64)GdtBase);
	__vmx_vmwrite(GUEST_GDTR_LIMIT, GetGdtLimit());
	__vmx_vmwrite(GUEST_LDTR_BASE, (ULONG64)IdtBase);
	__vmx_vmwrite(GUEST_IDTR_LIMIT, GetIdtLimit());

	//GUEST_IA32_DEBUGCTL = 0x00002802,
	//GUEST_IA32_DEBUGCTL_HIGH = 0x00002803,
	__vmx_vmwrite(GUEST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS) & 0xFFFFFFFF);
	__vmx_vmwrite(GUEST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP) & 0xFFFFFFFF);
	__vmx_vmwrite(GUEST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP) & 0xFFFFFFFF);


	__vmx_vmwrite(GUEST_RSP, ((ULONG64)VMX_Mem.VTStack) + 0x11000);
	__vmx_vmwrite(GUEST_RIP, (ULONG64)VmxGuest);
	
	__vmx_vmwrite(VMCS_LINK_POINTER, 0xffffffff);
	__vmx_vmwrite(VMCS_LINK_POINTER_HIGH, 0xffffffff);

		
	//2.Host State Fields
	__vmx_vmwrite(HOST_CR0, __readcr0());
	__vmx_vmwrite(HOST_CR3, __readcr3());
	__vmx_vmwrite(HOST_CR4, __readcr4());

	__vmx_vmwrite(HOST_CS_SELECTOR, RegGetCs()& 0xfff8);
	__vmx_vmwrite(HOST_DS_SELECTOR, RegGetDs()& 0xfff8);
	__vmx_vmwrite(HOST_ES_SELECTOR, RegGetEs()& 0xfff8);
	__vmx_vmwrite(HOST_SS_SELECTOR, RegGetSs()& 0xfff8);
	__vmx_vmwrite(HOST_FS_SELECTOR, RegGetFs()& 0xfff8);
	__vmx_vmwrite(HOST_GS_SELECTOR, RegGetGs()& 0xfff8);
	__vmx_vmwrite(HOST_TR_SELECTOR, GetTrSelector()& 0xfff8);

	__vmx_vmwrite(HOST_FS_BASE, __readmsr(MSR_FS_BASE));
	__vmx_vmwrite(HOST_GS_BASE, __readmsr(MSR_GS_BASE));
	__vmx_vmwrite(HOST_TR_BASE, 0xffffa700857f6000);
	
	__vmx_vmwrite(HOST_GDTR_BASE, (ULONG64)GdtBase);
	__vmx_vmwrite(HOST_IDTR_BASE, (ULONG64)IdtBase);
	
	__vmx_vmwrite(HOST_IA32_SYSENTER_CS,__readmsr(MSR_IA32_SYSENTER_CS)& 0xFFFFFFFF);
	__vmx_vmwrite(HOST_IA32_SYSENTER_ESP,__readmsr(MSR_IA32_SYSENTER_ESP)& 0xFFFFFFFF);
	__vmx_vmwrite(HOST_IA32_SYSENTER_EIP,__readmsr(MSR_IA32_SYSENTER_EIP)& 0xFFFFFFFF);
	
	__vmx_vmwrite(HOST_RSP, (ULONG64)VMX_Mem.VTStack+0x1000);
	__vmx_vmwrite(HOST_RIP, (ULONG64)ExithandlerPoint);

	//3.Vm-Crontol Fields
		//3.1	VM Execution Control 
		__vmx_vmwrite(PIN_BASED_VM_EXEC_CONTROL, VmxAdjustControls(0, MSR_IA32_VMX_PINBASED_CTLS));
		__vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, VmxAdjustControls(0, MSR_IA32_VMX_PROCBASED_CTLS));
		//3.2 VM exit Control 
		__vmx_vmwrite(VM_EXIT_CONTROLS, VmxAdjustControls(VM_EXIT_IA32E_MODE | VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
		//__vmx_vmwrite(VM_EXIT_CONTROLS, VmxAdjustControls(0, MSR_IA32_VMX_EXIT_CTLS));
		//3.3 VM Entry Crontol 
		__vmx_vmwrite(VM_ENTRY_CONTROLS, VmxAdjustControls(VM_ENTRY_IA32E_MODE, MSR_IA32_VMX_ENTRY_CTLS));
		//__vmx_vmwrite(VM_ENTRY_CONTROLS, VmxAdjustControls(0, MSR_IA32_VMX_ENTRY_CTLS));
}



NTSTATUS VMX_Mem_Control(DWORD32 Mem_Control) {		//VMX Memory Control  0:Allocate Memory 1:Free Memory
	switch (Mem_Control)
	{
	case 0:
		VMX_Mem.VMXON_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'vmx');
		VMX_Mem.VMCS_Region = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'vmcs');
		VMX_Mem.VTStack = ExAllocatePoolWithTag(NonPagedPool,0x20000,'stak');
		VMX_Mem.VMXON_Region_PA = MmGetPhysicalAddress(VMX_Mem.VMXON_Region);
		VMX_Mem.VMCS_Region_PA = MmGetPhysicalAddress(VMX_Mem.VMCS_Region);
		RtlZeroMemory(VMX_Mem.VMXON_Region, 0x1000);
		RtlZeroMemory(VMX_Mem.VMCS_Region, 0x1000);
		RtlZeroMemory(VMX_Mem.VTStack, 0x20000);
		*(PULONG)VMX_Mem.VMXON_Region = 1;  //write VMCS_ID
		*(PULONG)VMX_Mem.VMCS_Region = 1;
		return STATUS_SUCCESS;
	case 1:
		ExFreePoolWithTag(VMX_Mem.VMXON_Region, 'vmx');
		ExFreePoolWithTag(VMX_Mem.VMCS_Region, 'vmcs');
		ExFreePoolWithTag(VMX_Mem.VTStack, 'stak');
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
		__writecr0(0x0000000080050031);
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
	__vmx_vmclear(&VMX_Mem.VMCS_Region_PA);
	__vmx_vmptrld(&VMX_Mem.VMCS_Region_PA);
	VMCS_Setup();
	__vmx_vmlaunch();
	int Vxm_error_code = 0;
	__vmx_vmread(VM_INSTRUCTION_ERROR, &Vxm_error_code);
	DbgPrint("Error Code is %X\n", Vxm_error_code);
	return STATUS_SUCCESS;
}


NTSTATUS StopVt() {
	__vmx_vmclear(&VMX_Mem.VMXON_Region_PA);
	//__vmx_vmresume();
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