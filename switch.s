.align	4
.globl	__swtch
.globl	_swtch
__swtch:
_swtch:
	subl		$16,%esp	
	movl	%ebx,0(%esp)	
	movl	%esi,4(%esp)		
	movl	%edi,8(%esp)
	movl	%ebp,12(%esp)	
	movl	20(%esp),%eax	 
	movl	%esp,0(%eax)	 
	movl	24(%esp),%eax	
	movl	0(%eax),%esp	
	movl	0(%esp),%ebx	
	movl	4(%esp),%esi
	movl	8(%esp),%edi
	movl	12(%esp),%ebp	
	addl	$16, %esp		
	ret						
.align	4
.globl	__thrstart
.globl	_thrstart
__thrstart:
_thrstart:
	pushl %edi
	call *%esi
	pushl %eax
	call Thread_exit
.globl	__ENDMONITOR
.globl	_ENDMONITOR
__ENDMONITOR:
_ENDMONITOR:
