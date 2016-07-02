/* Automatically generated header (sfdc 1.10)! Do not edit! */

#ifndef PROTO_CAMD_H
#define PROTO_CAMD_H

#include <clib/camd_protos.h>

#ifndef _NO_INLINE
# if defined(__GNUC__)
#  ifdef __AROS__
#   include <defines/camd.h>
#  else
#   include <inline/camd.h>
#  endif
# else
#  include <pragmas/camd_pragmas.h>
# endif
#endif /* _NO_INLINE */

#ifdef __amigaos4__
# include <interfaces/camd.h>
# ifndef __NOGLOBALIFACE__
   extern struct CamdIFace *ICamd;
# endif /* __NOGLOBALIFACE__*/
#endif /* !__amigaos4__ */
#ifndef __NOLIBBASE__
  extern struct Library *
# ifdef __CONSTLIBBASEDECL__
   __CONSTLIBBASEDECL__
# endif /* __CONSTLIBBASEDECL__ */
  CamdBase;
#endif /* !__NOLIBBASE__ */

#endif /* !PROTO_CAMD_H */
