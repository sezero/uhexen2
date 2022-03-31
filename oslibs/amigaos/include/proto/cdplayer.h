/* Automatically generated header (sfdc 1.11)! Do not edit! */

#ifndef PROTO_CDPLAYER_H
#define PROTO_CDPLAYER_H

#include <clib/cdplayer_protos.h>

#ifndef _NO_INLINE
# if defined(__GNUC__)
#  ifdef __AROS__
#   include <defines/cdplayer.h>
#  else
#   include <inline/cdplayer.h>
#  endif
# else
#  include <pragmas/cdplayer_pragmas.h>
# endif
#endif /* _NO_INLINE */

#ifdef __amigaos4__
# include <interfaces/cdplayer.h>
# ifndef __NOGLOBALIFACE__
   extern struct CDPlayerIFace *ICDPlayer;
# endif /* __NOGLOBALIFACE__*/
#endif /* !__amigaos4__ */
#ifndef __NOLIBBASE__
  extern struct Library *
# ifdef __CONSTLIBBASEDECL__
   __CONSTLIBBASEDECL__
# endif /* __CONSTLIBBASEDECL__ */
  CDPlayerBase;
#endif /* !__NOLIBBASE__ */

#endif /* !PROTO_CDPLAYER_H */
