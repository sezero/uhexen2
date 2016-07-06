/* workaround for ixemul headers >= 48.3:
 * AmiTCP SDK expect socklen_t as LONG,
 * however ixemul defines it as int... */
#ifndef sys_types_hackity_h
#define sys_types_hackity_h
#define socklen_t ixemul_socklen_t
#include_next <sys/types.h>
#undef socklen_t
#endif
