/*
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack; "usergroup.library" API
 * Copyright © 2001-2019 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Amiga specific TCP/IP 'C' header files;
 * Freely Distributable
 *
 * WARNING: The "usergroup.library" API must be considered obsolete and
 *          should not be used in new software. It is provided solely
 *          for backwards compatibility and legacy application software.
 */

/*
 * This file was created with fd2pragma V2.171 using the following options:
 *
 * fd2pragma usergroup_lib.sfd to RAM:inline special 47
 */

#ifndef _INLINE_USERGROUP_H
#define _INLINE_USERGROUP_H

#ifndef CLIB_USERGROUP_PROTOS_H
#define CLIB_USERGROUP_PROTOS_H
#endif

#ifndef  LIBRARIES_USERGROUP_H
#include <libraries/usergroup.h>
#endif
#ifndef  PWD_H
#include <pwd.h>
#endif
#ifndef  GRP_H
#include <grp.h>
#endif

#ifndef USERGROUP_BASE_NAME
#define USERGROUP_BASE_NAME UserGroupBase
#endif

#define ug_SetupContextTagList(name, tags) ({ \
  STRPTR _ug_SetupContextTagList_name = (name); \
  struct TagItem * _ug_SetupContextTagList_tags = (tags); \
  ({ \
  register char * _ug_SetupContextTagList__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), struct TagItem * __asm("a1"))) \
  (_ug_SetupContextTagList__bn - 30))(_ug_SetupContextTagList__bn, _ug_SetupContextTagList_name, _ug_SetupContextTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___ug_SetupContextTags(struct Library * UserGroupBase, STRPTR name, ...)
{
  return ug_SetupContextTagList(name, (struct TagItem *) ((ULONG) &name + sizeof(STRPTR)));
}

#define ug_SetupContextTags(name...) ___ug_SetupContextTags(USERGROUP_BASE_NAME, name)
#endif

#define ug_GetErr() ({ \
  register char * _ug_GetErr__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_ug_GetErr__bn - 36))(_ug_GetErr__bn); \
})

#define ug_StrError(err) ({ \
  LONG _ug_StrError_err = (err); \
  ({ \
  register char * _ug_StrError__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), LONG __asm("d1"))) \
  (_ug_StrError__bn - 42))(_ug_StrError__bn, _ug_StrError_err); \
});})

#define getuid() ({ \
  register char * _getuid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_getuid__bn - 48))(_getuid__bn); \
})

#define geteuid() ({ \
  register char * _geteuid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_geteuid__bn - 54))(_geteuid__bn); \
})

#define setreuid(real, effective) ({ \
  LONG _setreuid_real = (real); \
  LONG _setreuid_effective = (effective); \
  ({ \
  register char * _setreuid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_setreuid__bn - 60))(_setreuid__bn, _setreuid_real, _setreuid_effective); \
});})

#define setuid(uid) ({ \
  LONG _setuid_uid = (uid); \
  ({ \
  register char * _setuid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_setuid__bn - 66))(_setuid__bn, _setuid_uid); \
});})

#define getgid() ({ \
  register char * _getgid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_getgid__bn - 72))(_getgid__bn); \
})

#define getegid() ({ \
  register char * _getegid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_getegid__bn - 78))(_getegid__bn); \
})

#define setregid(real, effective) ({ \
  LONG _setregid_real = (real); \
  LONG _setregid_effective = (effective); \
  ({ \
  register char * _setregid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_setregid__bn - 84))(_setregid__bn, _setregid_real, _setregid_effective); \
});})

#define setgid(gid) ({ \
  LONG _setgid_gid = (gid); \
  ({ \
  register char * _setgid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_setgid__bn - 90))(_setgid__bn, _setgid_gid); \
});})

#define getgroups(gidsetlen, gidset) ({ \
  LONG _getgroups_gidsetlen = (gidsetlen); \
  LONG * _getgroups_gidset = (gidset); \
  ({ \
  register char * _getgroups__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG * __asm("a1"))) \
  (_getgroups__bn - 96))(_getgroups__bn, _getgroups_gidsetlen, _getgroups_gidset); \
});})

#define setgroups(gidsetlen, gidset) ({ \
  LONG _setgroups_gidsetlen = (gidsetlen); \
  LONG * _setgroups_gidset = (gidset); \
  ({ \
  register char * _setgroups__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG * __asm("a1"))) \
  (_setgroups__bn - 102))(_setgroups__bn, _setgroups_gidsetlen, _setgroups_gidset); \
});})

#define initgroups(name, basegid) ({ \
  STRPTR _initgroups_name = (name); \
  LONG _initgroups_basegid = (basegid); \
  ({ \
  register char * _initgroups__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a1"), LONG __asm("d0"))) \
  (_initgroups__bn - 108))(_initgroups__bn, _initgroups_name, _initgroups_basegid); \
});})

#define getpwnam(login) ({ \
  STRPTR _getpwnam_login = (login); \
  ({ \
  register char * _getpwnam__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct passwd * (*)(char * __asm("a6"), STRPTR __asm("a1"))) \
  (_getpwnam__bn - 114))(_getpwnam__bn, _getpwnam_login); \
});})

#define getpwuid(uid) ({ \
  LONG _getpwuid_uid = (uid); \
  ({ \
  register char * _getpwuid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct passwd * (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_getpwuid__bn - 120))(_getpwuid__bn, _getpwuid_uid); \
});})

#define setpwent() ({ \
  register char * _setpwent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_setpwent__bn - 126))(_setpwent__bn); \
})

#define getpwent() ({ \
  register char * _getpwent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct passwd * (*)(char * __asm("a6"))) \
  (_getpwent__bn - 132))(_getpwent__bn); \
})

#define endpwent() ({ \
  register char * _endpwent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endpwent__bn - 138))(_endpwent__bn); \
})

#define getgrnam(name) ({ \
  STRPTR _getgrnam_name = (name); \
  ({ \
  register char * _getgrnam__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct group * (*)(char * __asm("a6"), STRPTR __asm("a1"))) \
  (_getgrnam__bn - 144))(_getgrnam__bn, _getgrnam_name); \
});})

#define getgrgid(gid) ({ \
  LONG _getgrgid_gid = (gid); \
  ({ \
  register char * _getgrgid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct group * (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_getgrgid__bn - 150))(_getgrgid__bn, _getgrgid_gid); \
});})

#define setgrent() ({ \
  register char * _setgrent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_setgrent__bn - 156))(_setgrent__bn); \
})

#define getgrent() ({ \
  register char * _getgrent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct group * (*)(char * __asm("a6"))) \
  (_getgrent__bn - 162))(_getgrent__bn); \
})

#define endgrent() ({ \
  register char * _endgrent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endgrent__bn - 168))(_endgrent__bn); \
})

#define crypt(key, set) ({ \
  UBYTE * _crypt_key = (key); \
  UBYTE * _crypt_set = (set); \
  ({ \
  register char * _crypt__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), UBYTE * __asm("a0"), UBYTE * __asm("a1"))) \
  (_crypt__bn - 174))(_crypt__bn, _crypt_key, _crypt_set); \
});})

#define ug_GetSalt(user, buf, size) ({ \
  struct passwd * _ug_GetSalt_user = (user); \
  UBYTE * _ug_GetSalt_buf = (buf); \
  ULONG _ug_GetSalt_size = (size); \
  ({ \
  register char * _ug_GetSalt__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), struct passwd * __asm("a0"), UBYTE * __asm("a1"), ULONG __asm("d0"))) \
  (_ug_GetSalt__bn - 180))(_ug_GetSalt__bn, _ug_GetSalt_user, _ug_GetSalt_buf, _ug_GetSalt_size); \
});})

#define getpass(prompt) ({ \
  STRPTR _getpass_prompt = (prompt); \
  ({ \
  register char * _getpass__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), STRPTR __asm("a1"))) \
  (_getpass__bn - 186))(_getpass__bn, _getpass_prompt); \
});})

#define umask(mask) ({ \
  ULONG _umask_mask = (mask); \
  ({ \
  register char * _umask__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), ULONG __asm("d0"))) \
  (_umask__bn - 192))(_umask__bn, _umask_mask); \
});})

#define getumask() ({ \
  register char * _getumask__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"))) \
  (_getumask__bn - 198))(_getumask__bn); \
})

#define setsid() ({ \
  register char * _setsid__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_setsid__bn - 204))(_setsid__bn); \
})

#define getpgrp() ({ \
  register char * _getpgrp__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_getpgrp__bn - 210))(_getpgrp__bn); \
})

#define getlogin() ({ \
  register char * _getlogin__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"))) \
  (_getlogin__bn - 216))(_getlogin__bn); \
})

#define setlogin(name) ({ \
  STRPTR _setlogin_name = (name); \
  ({ \
  register char * _setlogin__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a1"))) \
  (_setlogin__bn - 222))(_setlogin__bn, _setlogin_name); \
});})

#define setutent() ({ \
  register char * _setutent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_setutent__bn - 228))(_setutent__bn); \
})

#define getutent() ({ \
  register char * _getutent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct utmp * (*)(char * __asm("a6"))) \
  (_getutent__bn - 234))(_getutent__bn); \
})

#define endutent() ({ \
  register char * _endutent__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endutent__bn - 240))(_endutent__bn); \
})

#define getlastlog(uid) ({ \
  LONG _getlastlog_uid = (uid); \
  ({ \
  register char * _getlastlog__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct lastlog * (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_getlastlog__bn - 246))(_getlastlog__bn, _getlastlog_uid); \
});})

#define setlastlog(uid, name, host) ({ \
  LONG _setlastlog_uid = (uid); \
  STRPTR _setlastlog_name = (name); \
  STRPTR _setlastlog_host = (host); \
  ({ \
  register char * _setlastlog__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), STRPTR __asm("a0"), STRPTR __asm("a1"))) \
  (_setlastlog__bn - 252))(_setlastlog__bn, _setlastlog_uid, _setlastlog_name, _setlastlog_host); \
});})

#define getcredentials(task) ({ \
  struct Task * _getcredentials_task = (task); \
  ({ \
  register char * _getcredentials__bn __asm("a6") = (char *) (USERGROUP_BASE_NAME);\
  ((struct UserGroupCredentials * (*)(char * __asm("a6"), struct Task * __asm("a0"))) \
  (_getcredentials__bn - 258))(_getcredentials__bn, _getcredentials_task); \
});})

#endif /*  _INLINE_USERGROUP_H  */
