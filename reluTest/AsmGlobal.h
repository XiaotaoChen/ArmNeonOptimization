//
//  AsmGlobal.h
//  

#ifndef AsmGlobal_h
#define AsmGlobal_h
.macro asm_function fname
#ifdef __APPLE__
.globl _\fname
_\fname:
#else
.global \fname
#ifdef __ELF__
.hidden \fname
.type \fname, %function
#endif
\fname:
#endif
.endm


#endif /* AsmGlobal_h */
