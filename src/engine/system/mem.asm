global v_memset_sse
global v_memset_avx8
global v_memset_avx16


; void memset(rdi = void *dst, rsi = char c, rdx = long n )

v_memset_sse:
    ; rdi, rsi, rdx, rcx

    ; char c, will be stored in lower 8 of rsi, = sil

    movzx   esi,    sil ; clear garbage in rsi
    ; Now we need rsi to be 'c' repeating
    mov     rcx,    0x0101010101010101
    imul    rsi,    rcx
    
    movq    xmm0,   rsi ; mov quadword into our xmm0 (lower half)
    punpcklqdq xmm0, xmm0; Fill the rest of xmm0 with the lower half of xmm0


    


