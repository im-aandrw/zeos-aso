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
	
ENTRY(gettime)
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
	cmpl $0, %eax
	jge endpid
	negl %eax
	movl %eax, errno
	movl $-1, %eax
endpid:
	movl %ebp, %esp
	popl %ebp
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
	cmpl $0, %eax
	jge endfork
	negl %eax
	movl %eax, errno
	movl $-1, %eax
endfork:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(exit)
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

ENTRY(block)
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

ENTRY(unblock)
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
	
	
ENTRY(getchar)
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
