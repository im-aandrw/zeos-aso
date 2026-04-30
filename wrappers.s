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
 pushl $return_address
 pushl %ebp
 movl %esp, %ebp
 sysenter
return_address:
 popl %ebp
 cmpl $0, %eax
 jge end
 negl %eax
 movl %eax, errno
 movl $-1, %eax
end:
 movl -4(%ebp), %ebx
 movl %ebp, %esp
 popl %ebp
 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp, %ebp
 movl $10, %eax
 pushl $nextime
 pushl %ebp
 movl %esp, %ebp
 sysenter
nextime:
 popl %ebp
 cmpl $0, %eax
 jge endtime
 negl %eax
 movl %eax, errno
 movl $-1, %eax
endtime:
 movl %ebp, %esp
 popl %ebp
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
 cmpl $0, %eax
 jge endpid
 negl %eax
 movl %eax, errno
 movl $-1, %eax
endpid:
 movl %ebp, %esp
 popl %ebp
 ret

.globl fork; .type fork, @function; .align 0; fork:
 pushl %ebp
 movl %esp, %ebp
 movl $2, %eax
 pushl $nextfork
 pushl %ebp
 movl %esp, %ebp
 sysenter
nextfork:
 popl %ebp
 cmpl $0, %eax
 jge endfork
 negl %eax
 movl %eax, errno
 movl $-1, %eax
endfork:
 movl %ebp, %esp
 popl %ebp
 ret

.globl exit; .type exit, @function; .align 0; exit:
 pushl %ebp
 movl %esp, %ebp
 movl $1, %eax
 pushl $endexit
 pushl %ebp
 movl %esp, %ebp
 sysenter
endexit:
 movl %ebp, %esp
 popl %ebp
 ret

.globl block; .type block, @function; .align 0; block:
 pushl %ebp
 movl %esp, %ebp
 movl $5, %eax
 pushl $nextblock
 pushl %ebp
 movl %esp, %ebp
 sysenter
nextblock:
 popl %ebp
 cmpl $0, %eax
 jge endblock
 negl %eax
 movl %eax, errno
 movl $-1, %eax
endblock:
 movl %ebp, %esp
 popl %ebp
 ret

.globl unblock; .type unblock, @function; .align 0; unblock:
 pushl %ebp
 movl %esp, %ebp
 movl $6, %eax
 movl 8(%ebp), %edx
 pushl $nextunblock
 pushl %ebp
 movl %esp, %ebp
 sysenter
nextunblock:
 popl %ebp
 cmpl $0, %eax
 jge endunblock
 negl %eax
 movl %eax, errno
 movl $-1, %eax
endunblock:
 movl %ebp, %esp
 popl %ebp
 ret


.globl getchar; .type getchar, @function; .align 0; getchar:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx
 movl 8(%ebp), %edx
 movl $3, %eax
 pushl $return_getchar
 pushl %ebp
 movl %esp, %ebp
 sysenter
return_getchar:
 popl %ebp
 cmpl $0, %eax
 jge end_getchar
 negl %eax
 movl %eax, errno
 movl $-1, %eax
end_getchar:
 movl -4(%ebp), %ebx
 movl %ebp, %esp
 popl %ebp
 ret
