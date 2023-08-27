//
//  relu.S
//

#ifdef __arm__
#ifndef __aarch64__
@ #include "AsmGlobal.h"

.text
.align 5

asm_function reluAssembleV2
//void reluAssembleV2(const float* src, const int aligned_size, float* dst)

//Auto Load:
//r0:src, r1:aligned_size, r2:dst

push {r4-r11, lr}


vmov.f32 q0, #0.0
0:
    vld1.32 {d2-d3}, [r0]!
    vmax.f32 q1, q0, q1
    subs r1, r1, #4
    vst1.32 {d2-d3}, [r2]!
    bgt 0b
    @ bx lr

pop {r4-r11, pc}

#endif
#endif
