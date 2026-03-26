# 0 "wrappers.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrappers.S"
# 1 "include/asm.h" 1
# 2 "wrappers.S" 2
# 1 "include/segment.h" 1
# 3 "wrappers.S" 2
# 1 "include/errno.h" 1
# 4 "wrappers.S" 2

.globl write; .type write, @function; .align 0; write:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx
 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx
 movl $4, %eax
 pushl %edx
 pushl %ecx
 pushl $return_address
 pushl %ebp
    sysenter
return_address:
 popl %ebp
 subl $4, %esp
 popl %ecx
 popl %edx
 cmpl $0, %eax
 jge end
 negl %eax
 movl %eax, errno
 movl $-1, %eax
end:
 popl %ebx
 popl %ebp
 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
    pushl %ebp
    movl %esp, %ebp
    movl $10, %eax
    pushl %edx
    pushl %ecx
    pushl $nextime
    pushl %ebp
    movl %esp, %ebp
    sysenter
nextime:
    popl %ebp
    subl $4, %esp
    popl %ecx
    popl %edx
    cmpl $0, %eax
    jge endtime
    negl %eax
    movl %eax, errno
    movl $-1, %eax
endtime:
    movl %ebp, %esp
    pop %ebp
    ret

.globl getpid; .type getpid, @function; .align 0; getpid:
    pushl %ebp
    movl %esp, %ebp
 movl $20, %eax
 pushl $nextpid
 pushl %ebp
    movl %esp, %ebp
 sysenter
nextpid:
 popl %ebp
    subl $4, %esp
    cmpl $0, %eax
    jge endpid
    negl %eax
    movl %eax, errno
    movl $-1, %eax
endpid:
    movl %ebp, %esp
    pop %ebp
    ret
