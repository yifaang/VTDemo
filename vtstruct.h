#include <ntddk.h>
typedef union
{
	struct
	{
		unsigned SSE3 : 1;
		unsigned PCLMULQDQ : 1;
		unsigned DTES64 : 1;
		unsigned MONITOR : 1;
		unsigned DS_CPL : 1;
		unsigned VMX : 1;
		unsigned SMX : 1;
		unsigned EIST : 1;
		unsigned TM2 : 1;
		unsigned SSSE3 : 1;
		unsigned Reserved : 22;
	};

}_CPUID_ECX;

typedef union {
	struct 
	{
		unsigned PE : 1;
		unsigned MP : 1;
		unsigned EM : 1;
		unsigned TS : 1;
		unsigned ET : 1;
		unsigned NE : 1;
		unsigned Reverse1 : 10;
		unsigned WP : 1;
		unsigned Reverse2 : 1;
		unsigned AM : 1;
		unsigned Reverse3 : 10;
		unsigned NW : 1;
		unsigned CD : 1;
		unsigned PG : 1;
		//unsigned Reverse4 : 32;
	};

}_CR0;

typedef union
{
	struct {
		unsigned VME : 1;
		unsigned PVI : 1;
		unsigned TSD : 1;
		unsigned DE : 1;
		unsigned PSE : 1;
		unsigned PAE : 1;
		unsigned MCE : 1;
		unsigned PGE : 1;
		unsigned PCE : 1;
		unsigned OSFXSR : 1;
		unsigned PSXMMEXCPT : 1;
		unsigned UNKONOWN_1 : 1;		//These are zero
		unsigned UNKONOWN_2 : 1;		//These are zero
		unsigned VMXE : 1;			//It's zero in normal
		unsigned Reserved : 18;		//These are zero
		unsigned Reverse2 : 32;
	};
}_CR4;

typedef struct VMX_Struct{
	PVOID VMXON_Region;
	PVOID VMCS_Region;
	PHYSICAL_ADDRESS VMXON_Region_PA;
	PHYSICAL_ADDRESS VMCS_Region_PA;
}VMX_Struct,*PVMX_Struct;
