//
//  relu.S
//

#ifdef __aarch64__
@ #include "../AsmGlobal.h"

.global reluAssembleV2
.type reluAssembleV2, %function

.text
.align 5

@ asm_function reluAssembleV2
//void reluAssembleV2(const float* src, const int aligned_size, float* dst)

//Auto Load:
//x0:src, x1:aligned_size, x2:dst

movi v0.4s, #0
scvtf v0.4s, v0.4s
0:
    ld1 {v1.4s}, [x0], #16
    fmax v1.4s, v0.4s, v1.4s
    subs x1, x1, #4
    st1 {v1.4s}, [x2], #16
    bgt 0b
    ret

#endif
