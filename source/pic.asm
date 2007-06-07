; ******************************  pic.asm   **********************************
; Author:        Agner Fog
; Date created:  2007-01-27
; 
; Description:
; This example file shows how to make position-independent 32-bit code with
; the MASM/ML assembler.
;
; (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
; ****************************************************************************
;
; Explanation:
; Position-independent code is needed in Linux, BSD and Mac OS X systems when
; making dynamically linked libraries, also called shared objects (*.so files).
; The 32-bit x86 instruction set has no instruction for self-relative access
; to data. There are two problems we need to solve in order to make relative
; references to data in a data segment in 32-bit x86 code:
;
; (1) There is no way of addressing data relative to EIP and no simple
;     instruction for copying EIP into a pointer register.
;
; (2) It is not possible for the assembler to calculate the distance between
;     two addresses in different segments because the COFF object file format
;     has no relocation type to support this.
;
; Problem (1) is solved by using a CALL instruction to copy the instruction
; pointer EIP to the stack and then read it into a register (usually EBX).
; The CALL instruction must be followed by a RET in order to avoid
; mispredictions in the CPU.
;
; Problem (2) is solved by using a bogus CALL instruction to make a self-
; relative reference to a reference point in the data segment, and then
; back-patching the CALL instruction to change it to an ADD instruction.
; The relative address of the bogus CALL instruction then becomes the
; immediate operand of an ADD instruction. This makes it possible to
; calculate the address of the reference point in the data segment relative
; to the return point for the CALL instruction used under (1).
;
; Instructions:
; Include the "Macros and other necessary definitions" in your assembly code.
; Call the macro MAKEDATAPOINTER in the beginning of any function that needs
; to access data in the data segment. This will make a register, for example
; EBX, point to an arbitrary reference point in the data segment. It can be
; advantageous to place the reference point so that the most often used data
; are within +/- 128 bytes of the reference point. This will make the 
; instructions shorter.
; 
; The MAKEDATAPOINTER macro needs an extra function that reads the instruction
; pointer into a register (e.g. EBX). This function is created by calling
; the macro MAKETHUNK. It is necessary that MAKETHUNK is called outside of
; the first function.
; 
; If different functions use different pointer registers in MAKEDATAPOINTER
; then you must make one call to MAKETHUNK for each pointer register used.
;
; Example:
; The example below shows a function named testpic that returns the sum
; of the variables DAT1 and DAT2 in the data segment. You can call testpic
; from a C++ code by defining it as
; extern "C" int testpic();
; This function should return the value 300.
;
; Note about 64-bit code:
; It is much easier to make position-independent code in 64-bit mode because
; the x86-64 instruction set supports RIP-relative addressing. The data
; accesses in the example below would simply be:
;
;    mov   eax, DAT1                ; position independent only in 64 bit mode
;    add   eax, DAT2
;
; The ML64 assembler uses RIP-relative addressing by default when there is no
; pointer register. An array must be accessed with the LEA instruction. 
; For example:
;
;    mov   eax, array[rcx*4]        ; position dependent in 64 bit mode
;
; This code can be made position-independent by using
;
;   lea    rdx, array               ; get rip-relative address of array
;   mov    eax, [rdx + rcx*4]       ; access array through rdx
;
; ****************************************************************************


; ******************* Macros and other necessary definitions *****************

; Macro for making pointer to a reference point in data segment in 32 bit code.
; Call this macro in the beginning of a function that needs to address data
; in the data segment. 
;
; Parameters:
; REGISTERNAME:   A 32-bit register that you want to use as pointer
; REFERENCEPOINT: A label in the data segment
; 
; The macro will make the register point to the reference point

MAKEDATAPOINTER MACRO REGISTERNAME, REFERENCEPOINT
local p0, p1, p2
   call Read_EIP_into_&REGISTERNAME       ; make register point to p0
p0:                                       ; register REGISTERNAME now points here
   db  81h                                ; beginning of add REGISTERNAME, xxx instruction
p1:   
   call near ptr REFERENCEPOINT + (p2-p0) ; Bogus CALL instruction for making self-relative reference
p2:
; back patch CALL instruction to change it to  ADD REGISTERNAME, (REFERENCEPOINT - p0)
   org p1                    
   db  0C0H + ?&REGISTERNAME              ; second byte of instruction  ADD REGISTERNAME, (REFERENCEPOINT - p0)
   org p2                                 ; now REGISTERNAME points to REFERENCEPOINT
ENDM   


; Macro for making local function that reads EIP into a register.
; The function that this needed by MAKEDATAPOINTER is created with this macro.
; The register name must be the same as used by MAKEDATAPOINTER.
; This macro must be called outside of any function. 

MAKETHUNK MACRO REGISTERNAME
Read_EIP_into_&REGISTERNAME:              ; use local label to avoid clash with other modules
   mov REGISTERNAME, [ESP]                ; read instruction pointer from stack
   ret                                    ; return
ENDM


; Define instruction codes for register names
?EAX  equ  000b
?EBX  equ  011b
?ECX  equ  001b
?EDX  equ  010b
?ESI  equ  110b
?EDI  equ  111b
?EBP  equ  101b
?ESP  equ  100b

; ************************ End of necessary definitions **********************


; **************************** Example begins here ***************************

.386
.model flat

.data                                     ; data segment

REFPOINT LABEL DWORD                      ; ebx will point to this reference point
DAT1    DD  100                           ; variables in data segment
DAT2    DD  200

.code                                     ; code segment

; extern "C" int test ();                 ; C++ declaration for testpic()
_testpic  proc near                       ; procedure testpic() begins here
public _testpic
   push  ebx                              ; ebx must be saved
   MAKEDATAPOINTER ebx, REFPOINT          ; make ebx point to REFPOINT
   
   mov   eax, [ebx](DAT1 - REFPOINT)      ; read DAT1 using address relative to REFPOINT
   add   eax, [ebx](DAT2 - REFPOINT)      ; add  DAT2 using address relative to REFPOINT
   pop   ebx                              ; restore ebx
   ret                                    ; return
_testpic  endp   
   
MAKETHUNK ebx                             ; make function Read_EIP_into_ebx needed by MAKEDATAPOINTER

end
; ****************************** End of example ******************************
