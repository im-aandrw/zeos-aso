#include <asm.h>
#include <segment.h>
#include <errno.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	movl 8(%ebp), %edx // fd = edx
	movl 12(%ebp), %ecx // buffer = ecx
	movl 16(%ebp), %ebx // size = ebx
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
	
ENTRY(gettime)
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

ENTRY(getpid)
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

ENTRY(fork)
    pushl %ebp
    movl %esp, %ebp
    movl $2, %eax
    pushl $nextfork
    pushl %ebp
    movl %esp, %ebp
    sysenter
nextfork:
    popl %ebp
    subl $4, %esp
    cmpl $0, %eax
    jge endfork
    negl %eax
    movl %eax, errno
    movl $-1, %eax
endfork:
    movl %ebp, %esp
    pop %ebp
    ret









