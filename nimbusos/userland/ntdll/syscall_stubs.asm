; NimbusOS syscall stubs (x64) — bare-metal; host usa libc.
; NtCreateFile PROC
;   mov r10, rcx
;   mov eax, 55h
;   syscall
;   ret
; NtCreateFile ENDP
