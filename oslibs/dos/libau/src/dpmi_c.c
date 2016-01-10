#include "libaudef.h"

static struct dosmem_t au_dosmem;
#ifdef ZDM
unsigned short au_map_selector;
#endif

void pds_dpmi_dos_freemem(void)
{
	struct dosmem_t *dm = &au_dosmem;
	unsigned short sel = dm->selector;

#ifdef __DJGPP__
	__asm__("movw %0,%%dx"::"m"(sel));
	__asm__("movw $0x101,%ax");
	__asm__("int $0x31");

#else /*WATCOM*/
	_asm{
		mov ax,101h
		mov dx,sel
		int 31h
	}
#endif
}

struct dosmem_t *pds_dpmi_dos_allocmem(unsigned int size)
{
	struct dosmem_t *dm = &au_dosmem;
	unsigned short sel;
	char *lin;

#ifdef __DJGPP__
  #ifndef ZDM
	if(!__djgpp_nearptr_enable())
		return NULL;
  #endif

	__asm__("movl %0,%%ebx"::"m"(size));
	__asm__("movw $0x100,%ax	\n\
		addl $16,%ebx		\n\
		shrl $4,%ebx		\n\
		int $0x31		\n\
		jnc go			\n\
		xorl %edx,%edx		\n\
	go:				\n\
		movzx %ax,%eax		\n\
		shll $4,%eax");
	__asm__("movl %%eax,%0":"=m"(lin));
	__asm__("movw %%dx,%0":"=m"(sel));

#else /*WATCOM*/
	_asm{
		mov ax,100h
		mov ebx,size
		add ebx,16
		shr ebx,4
		int 31h
		jnc go
		xor edx,edx
	go:
		movzx eax,ax
		shl eax,4
		mov lin,eax;
		mov sel,dx
	}
#endif

	dm->linearptr = lin;
	dm->selector = sel;
	return (sel)? dm : NULL;
}

unsigned long pds_dpmi_map_physical_memory(unsigned long phys_addr,unsigned long memsize)
{
#ifdef ZDM
	__dpmi_meminfo mi;
	mi.address = phys_addr;
	mi.size = memsize;

	if(__dpmi_physical_address_mapping(&mi) != 0)
		return 0;

	au_map_selector = __dpmi_allocate_ldt_descriptors(1);
	__dpmi_set_segment_base_address (au_map_selector,mi.address);
	__dpmi_set_segment_limit (au_map_selector, mi.size - 1);

	return mi.address;

#else
	unsigned long ret;
	union REGS regs;

	memset(&regs,0,sizeof(union REGS));

	regs.w.ax = 0x0800;
	regs.w.bx = (phys_addr>>16);
	regs.w.cx = (phys_addr&0xffff);
	regs.w.di = (memsize&0xffff);
	regs.w.si = (memsize>>16);
	int386(0x31,&regs,&regs);

	if(regs.w.cflag)
		return 0;

	ret = ((unsigned long)regs.w.bx<<16) | (regs.w.cx&0xffff);

	#ifdef __DJGPP__
		ret += __djgpp_conventional_base;
	#endif

	return ret;
#endif
}

void pds_dpmi_unmap_physycal_memory(unsigned long linear_addr)
{
#ifdef ZDM
	__dpmi_meminfo mi;
	mi.address = linear_addr;
	__dpmi_free_physical_address_mapping(&mi);

#else
	union REGS regs;

	memset(&regs,0,sizeof(union REGS));

	regs.w.ax = 0x0801;
	regs.w.bx = (linear_addr>>16);
	regs.w.cx = (linear_addr&0xffff);
	int386(0x31,&regs,&regs);
#endif
}
