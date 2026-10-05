global AVX_Add
global AVX_Integrate
global AVX_IntegrateFMA3

section .text
  
; rdi = dst
; rsi = a
; rdx = b
; rcx = count


AVX_Add:
  
  .avxloop:
    cmp rcx, 0x8
    jl .sseloop



    vmovups ymm0, [rsi]
    vmovups ymm1, [rdx]

    vaddps ymm0, ymm0, ymm1

    vmovups [rdi], ymm0

    ; advance
    add rsi, 0x20
    add rdx, 0x20
    add rdi, 0x20
    
    sub rcx, 0x8
    jmp .avxloop

  .sseloop:
    vzeroupper ; Maybe not call this every iteration, but this should be called just once anyway
    cmp rcx, 0x4
    jl .done
    
    movups xmm0, [rsi]
    movups xmm1, [rdx]

    addps xmm0, xmm1
    movups [rdi], xmm0

    ; advance
    add rsi, 0x10
    add rdx, 0x10
    add rdi, 0x10
  
    sub rcx, 0x4

    jmp .sseloop



  .done:
    vzeroupper
    mov rax, rcx
    ret

AVX_Integrate:
; rdi = dst
; rsi = a;
; rdx = b;
; rcx = count
; xmm0 = m
  vbroadcastss ymm0, xmm0
  .avxloop: ; Integrate 8 floats
    cmp rcx,0x8 
    jl .sseloop
    ; No FMA3 Support
    vmovups ymm1, [rsi]
    vmovups ymm2, [rdx]

    vmulps ymm2, ymm2, ymm0 ; b = b * m
    vaddps ymm1, ymm1, ymm2

    vmovups [rdi], ymm1

    ; advance
    add rsi, 0x20
    add rdx, 0x20
    add rdi, 0x20

    sub rcx, 0x8
    jmp .avxloop

  .sseloop: ; Integrate 4 floats
    vzeroupper
    cmp rcx, 0x4
    jl .done

    movups xmm1, [rsi]
    movups xmm2, [rdx]

    mulps xmm2, xmm0
    addps xmm1, xmm2

    movups [rdi], xmm1

    ;adavnce
    add rsi, 0x10
    add rdx, 0x10
    add rdi, 0x10
    
    sub rcx, 0x4
    jmp .sseloop
  
  .done:
    vzeroupper
    mov rax, rcx
    ret

AVX_IntegrateFMA3:
; rdi = dst
; rsi = a;
; rdx = b;
; rcx = count
; xmm0 = m
  vbroadcastss ymm0, xmm0 ; ymm0 is just m
  .avxloop:
    ; dst = a + (m * b)
    ; Assume FMA3 support?
    cmp rcx, 0x8
    jl .sseloop 
    ; Move a and b
    vmovups ymm1, [rsi]
    vmovups ymm2, [rdx]

    ; FMA3 instruction
    vfmadd231ps ymm1, ymm2, ymm0
    ; Store result
    vmovups [rdi], ymm1
  
    ;advance
    add rsi, 0x20
    add rdi, 0x20
    add rdx, 0x20

    sub rcx, 0x8
    jmp .avxloop

  .sseloop:
    vzeroupper
    cmp rcx, 0x4
    jl .done

    movups xmm1, [rsi]
    movups xmm2, [rdx]

    ; 4 Floats can use the same instruction
    vfmadd231ps xmm1, xmm2, xmm0
    vmovups [rdi], xmm1

    ;advance
    add rsi, 0x10
    add rdx, 0x10
    add rdi, 0x10

  .done:
    vzeroupper
    mov rax, rcx
    ret
    
