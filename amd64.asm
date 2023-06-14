
.DATA

.CODE
;
;
;Get Selector
RegGetCs PROC
	mov rax,cs
	ret
RegGetCs ENDP
	
RegGetDs PROC
	mov rax,cs
	ret
RegGetDs ENDP

RegGetEs PROC
	mov rax,es
	ret
RegGetEs ENDP

RegGetSs PROC
	mov rax,ss
	ret
RegGetSs ENDP

GetTrSelector PROC
	str	rax
	ret
GetTrSelector ENDP

RegGetFs PROC
	mov		rax, fs
	ret
RegGetFs ENDP

RegGetGs PROC
	mov rax,gs
	ret
RegGetGS ENDP


; Get IDT/GDT 
GetIdtBase PROC
	LOCAL	idtr[10]:BYTE
	sidt	idtr
	mov		rax, QWORD PTR idtr[2]
	ret
GetIdtBase ENDP

GetGdtBase PROC
	LOCAL	gdtr[10]:BYTE
	sgdt	gdtr
	mov		rax, QWORD PTR gdtr[2]
	ret
GetGdtBase ENDP


GetGdtLimit PROC
	LOCAL	gdtr[10]:BYTE

	sgdt	gdtr
	mov		ax, WORD PTR gdtr[0]
	ret
GetGdtLimit ENDP

GetIdtLimit PROC
	LOCAL	idtr[10]:BYTE
	sidt	idtr
	mov		ax, WORD PTR idtr[0]
	ret
GetIdtLimit ENDP



;VmLauch Handler
;ExithandlerPoint PROC
;	xor rax,rax
;	int 3
;ExithandlerPoint ENDP

Asm_vmcall PROC
;mov ax,es
;mov es,ax
;mov ax,ds
;mov ds,ax
;mov ax,fs
;mov fs,ax
;mov ax,gs
;mov gs,ax
;mov ax,ss
;mov ss,ax
	vmcall
	ret
Asm_vmcall ENDP

RegGetRflags PROC
	pushfq
	pop		rax
	ret
RegGetRflags ENDP

END