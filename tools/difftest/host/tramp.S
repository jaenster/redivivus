/*
 * The call trampoline: run one function under a register and stack state the
 * harness chose, on a private stack, and come back whatever the function does.
 *
 *   int tramp_call(CallCtx *ctx)
 *
 * The layout of CallCtx is fixed (see difftest.c, the CTX_* offsets must agree).
 * The function runs on ctx->stack_top, a stack the harness filled with a known
 * pattern, so an uninitialised local reads the same bytes in both images and a
 * difference it causes is the function's, not the harness's. The TEB stack
 * bounds are switched with it, because ___chkstk_ms and the exception
 * dispatcher both look at them.
 *
 * A fault or a timeout redirects EIP to tramp_escape, which unwinds to the
 * saved host ESP. Whatever the function left on its stack is abandoned.
 */
    .intel_syntax noprefix
    .data
    .globl _g_tramp_ctx
_g_tramp_ctx:       .long 0
_g_saved_esp:       .long 0
_g_saved_sbase:     .long 0
_g_saved_slimit:    .long 0
_g_pre_esp:         .long 0
_g_fn:              .long 0
_g_r_eax:           .long 0

    .text
    .globl _tramp_call
    .globl _tramp_escape
_tramp_call:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov eax, [ebp + 8]
    mov [_g_tramp_ctx], eax
    mov [_g_saved_esp], esp
    mov ecx, fs:[4]
    mov [_g_saved_sbase], ecx
    mov ecx, fs:[8]
    mov [_g_saved_slimit], ecx
    mov ecx, [eax + 96]            /* stack_top */
    mov fs:[4], ecx
    mov edx, [eax + 152]           /* stack_limit */
    mov fs:[8], edx
    mov esp, ecx
    mov ecx, [eax + 28]            /* nstack */
    test ecx, ecx
    jz 2f
1:  push dword ptr [eax + 32 + ecx * 4 - 4]
    dec ecx
    jnz 1b
2:  mov [_g_pre_esp], esp
    mov ecx, [eax]
    mov [_g_fn], ecx
    fninit
    push 0x27f                     /* the MSVC CRT default: 53-bit precision */
    fldcw [esp]
    add esp, 4
    cld
    mov ecx, [eax + 8]
    mov edx, [eax + 12]
    mov ebx, [eax + 16]
    mov esi, [eax + 20]
    mov edi, [eax + 24]
    mov ebp, [eax + 156]           /* ebp sentinel */
    mov eax, [eax + 4]
    call dword ptr [_g_fn]
    mov [_g_r_eax], eax
    mov eax, [_g_tramp_ctx]
    mov ecx, [_g_r_eax]
    mov [eax + 104], ecx
    mov [eax + 108], edx
    mov [eax + 112], ebx
    mov [eax + 116], esi
    mov [eax + 120], edi
    mov [eax + 124], ebp
    mov ecx, esp
    sub ecx, [_g_pre_esp]
    mov [eax + 128], ecx
    cmp dword ptr [eax + 100], 0   /* want_fpu */
    je 3f
    fstp qword ptr [eax + 136]
3:  mov dword ptr [eax + 132], 0   /* status: returned */
    jmp 4f
_tramp_escape:
4:  mov ecx, [_g_saved_sbase]
    mov fs:[4], ecx
    mov ecx, [_g_saved_slimit]
    mov fs:[8], ecx
    mov esp, [_g_saved_esp]
    fninit
    cld
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
