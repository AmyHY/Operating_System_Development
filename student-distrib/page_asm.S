# setup_cr - setup values for cr0, cr3, cr4 to enabel paging.

.global setup_cr
setup_cr:
  pushl %ebp
  movl  %esp, %ebp

  movl  8(%ebp), %ecx
  movl  %ecx, %cr3 # Set cr3 to be the base address of page directory

  movl  %cr4, %ecx
  orl   $0x10, %ecx
  movl  %ecx, %cr4    # Set the PSE bit at cr4 since PDEs point to both page tables and pages.

  movl %cr0, %ecx
  orl  $0x80000001, %ecx
  movl %ecx, %cr0    # Set the PG and PE bit at cr0.

  movl	%cr3,%eax  # flush tlb
	movl	%eax,%cr3

  leave
  ret


# flush_tlb
.global flush_tlb
flush_tlb:
  movl	%cr3,%eax
	movl	%eax,%cr3

