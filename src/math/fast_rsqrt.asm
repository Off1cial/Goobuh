; fast_rsqrt.asm
; nasm -f elf64 fast_rsqrt.asm -o fast_rsqrt.o - done my cmake now

section .text
    global _FastRSqrtSSE

_FastRSqrtSSE:
    ; Parameters: xmm0 = float a
    ; Returns: xmm0 = 1/sqrt(a)
    
    rsqrtss xmm1, xmm0
    
    ; Newton-Raphson refinement
    movss   xmm2, xmm0
    mulss   xmm2, xmm1
    mulss   xmm2, xmm1          ; xmm2 = a * xr^2
    
    movss   xmm3, [three_rel]   ; xmm3 = 3.0f
    subss   xmm3, xmm2          ; xmm3 = 3.0f - a * xr^2
    
    movss   xmm4, [half_rel]    ; xmm4 = 0.5f
    mulss   xmm3, xmm4          ; xmm3 = (3.0f - a * xr^2) * 0.5f
    
    mulss   xmm1, xmm3          ; xmm1 = xr * refinement
    
    movss   xmm0, xmm1
    ret

section .rodata
    align 8
three_rel:
    dd 3.0
half_rel:
    dd 0.5