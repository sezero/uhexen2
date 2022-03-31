/* Automatically generated header (sfdc 1.11)! Do not edit! */

#ifndef _INLINE_CDPLAYER_H
#define _INLINE_CDPLAYER_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef CDPLAYER_BASE_NAME
#define CDPLAYER_BASE_NAME CDPlayerBase
#endif /* !CDPLAYER_BASE_NAME */

#ifndef LP2A5
#define LP2A5(offs, rt, name, t1, v1, r1, t2, v2, r2, bt, bn)	\
({								\
   t1 _##name##_v1 = (v1);					\
   t2 _##name##_v2 = (v2);					\
   rt _##name##_re2 =						\
   ({								\
      register int _d1 __asm("d1");				\
      register int _a0 __asm("a0");				\
      register int _a1 __asm("a1");				\
      register rt _##name##_re __asm("d0");			\
      register void *const _##name##_bn __asm("a6") = (bn);	\
      register t1 _n1 __asm(#r1) = _##name##_v1;		\
      register t2 _n2 __asm(#r2) = _##name##_v2;		\
      __asm volatile ("exg d7,a5\n\tjsr a6@(-"#offs":W)\n\texg d7,a5"			\
      : "=r" (_##name##_re), "=r" (_d1), "=r" (_a0), "=r" (_a1)	\
      : "r" (_##name##_bn), "rf"(_n1), "rf"(_n2)		\
      : "fp0", "fp1", "cc", "memory");				\
      _##name##_re;						\
   });								\
   _##name##_re2;						\
})
#endif

#ifndef LP3A5
#define LP3A5(offs, rt, name, t1, v1, r1, t2, v2, r2, t3, v3, r3, bt, bn) \
({								\
   t1 _##name##_v1 = (v1);					\
   t2 _##name##_v2 = (v2);					\
   t3 _##name##_v3 = (v3);					\
   rt _##name##_re2 =						\
   ({								\
      register int _d1 __asm("d1");				\
      register int _a0 __asm("a0");				\
      register int _a1 __asm("a1");				\
      register rt _##name##_re __asm("d0");			\
      register void *const _##name##_bn __asm("a6") = (bn);	\
      register t1 _n1 __asm(#r1) = _##name##_v1;		\
      register t2 _n2 __asm(#r2) = _##name##_v2;		\
      register t3 _n3 __asm(#r3) = _##name##_v3;		\
      __asm volatile ("exg d7,a5\n\tjsr a6@(-"#offs":W)\n\texg d7,a5"			\
      : "=r" (_##name##_re), "=r" (_d1), "=r" (_a0), "=r" (_a1)	\
      : "r" (_##name##_bn), "rf"(_n1), "rf"(_n2), "rf"(_n3)	\
      : "fp0", "fp1", "cc", "memory");				\
      _##name##_re;						\
   });								\
   _##name##_re2;						\
})
#endif

#define CDEject(___io_ptr) \
      LP1A5(0x1e, BYTE, CDEject , struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

/*
#define CDPlay(___starttrack, ___endtrack, ___io_ptr) \
      LP3A5(0x24, BYTE, CDPlay , UBYTE, ___starttrack, a0, UBYTE, ___endtrack, a1, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)
*/
#define CDPlay(___starttrack, ___endtrack, ___io_ptr) \
      LP3A5(0x24, BYTE, CDPlay , ULONG, ___starttrack, a0, ULONG, ___endtrack, a1, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDResume(___Mode, ___io_ptr) \
      LP2A5(0x2a, BYTE, CDResume , BOOL, ___Mode, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDStop(___io_ptr) \
      LP1A5(0x30, BYTE, CDStop , struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDJump(___Blocks, ___io_ptr) \
      LP2A5(0x36, BYTE, CDJump , ULONG, ___Blocks, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDActive(___io_ptr) \
      LP1A5(0x3c, BOOL, CDActive , struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDCurrentTitle(___io_ptr) \
      LP1A5(0x42, ULONG, CDCurrentTitle , struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDTitleTime(___cd_time, ___io_ptr) \
      LP2A5(0x48, BYTE, CDTitleTime , struct CD_Time *, ___cd_time, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDGetVolume(___vol, ___io_ptr) \
      LP2A5(0x4e, BYTE, CDGetVolume , struct CD_Volume *, ___vol, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDSetVolume(___vol, ___io_ptr) \
      LP2A5(0x54, BYTE, CDSetVolume , struct CD_Volume *, ___vol, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDReadTOC(___toc, ___io_ptr) \
      LP2A5(0x5a, BYTE, CDReadTOC , struct CD_TOC *, ___toc, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#define CDInfo(___cdi, ___io_ptr) \
      LP2A5(0x60, BYTE, CDInfo , struct CD_Info *, ___cdi, a0, struct IOStdReq *, ___io_ptr, d7,\
      , CDPLAYER_BASE_NAME)

#endif /* !_INLINE_CDPLAYER_H */
