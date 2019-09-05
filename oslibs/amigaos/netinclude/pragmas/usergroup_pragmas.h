/*
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack; "usergroup.library" API
 * Copyright © 2001-2016 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Amiga specific TCP/IP 'C' header files;
 * Freely Distributable
 *
 * WARNING: The "usergroup.library" API must be considered obsolete and
 *          should not be used in new software. It is provided solely
 *          for backwards compatibility and legacy application software.
 */

#ifndef PRAGMAS_USERGROUP_H
#define PRAGMAS_USERGROUP_H

#ifndef CLIB_USERGROUP_PROTOS_H
#include <clib/usergroup_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(UserGroupBase,0x01e,ug_SetupContextTagList(a0,a1))
#pragma amicall(UserGroupBase,0x024,ug_GetErr())
#pragma amicall(UserGroupBase,0x02a,ug_StrError(d1))
#pragma amicall(UserGroupBase,0x030,getuid())
#pragma amicall(UserGroupBase,0x036,geteuid())
#pragma amicall(UserGroupBase,0x03c,setreuid(d0,d1))
#pragma amicall(UserGroupBase,0x042,setuid(d0))
#pragma amicall(UserGroupBase,0x048,getgid())
#pragma amicall(UserGroupBase,0x04e,getegid())
#pragma amicall(UserGroupBase,0x054,setregid(d0,d1))
#pragma amicall(UserGroupBase,0x05a,setgid(d0))
#pragma amicall(UserGroupBase,0x060,getgroups(d0,a1))
#pragma amicall(UserGroupBase,0x066,setgroups(d0,a1))
#pragma amicall(UserGroupBase,0x06c,initgroups(a1,d0))
#pragma amicall(UserGroupBase,0x072,getpwnam(a1))
#pragma amicall(UserGroupBase,0x078,getpwuid(d0))
#pragma amicall(UserGroupBase,0x07e,setpwent())
#pragma amicall(UserGroupBase,0x084,getpwent())
#pragma amicall(UserGroupBase,0x08a,endpwent())
#pragma amicall(UserGroupBase,0x090,getgrnam(a1))
#pragma amicall(UserGroupBase,0x096,getgrgid(d0))
#pragma amicall(UserGroupBase,0x09c,setgrent())
#pragma amicall(UserGroupBase,0x0a2,getgrent())
#pragma amicall(UserGroupBase,0x0a8,endgrent())
#pragma amicall(UserGroupBase,0x0ae,crypt(a0,a1))
#pragma amicall(UserGroupBase,0x0b4,ug_GetSalt(a0,a1,d0))
#pragma amicall(UserGroupBase,0x0ba,getpass(a1))
#pragma amicall(UserGroupBase,0x0c0,umask(d0))
#pragma amicall(UserGroupBase,0x0c6,getumask())
#pragma amicall(UserGroupBase,0x0cc,setsid())
#pragma amicall(UserGroupBase,0x0d2,getpgrp())
#pragma amicall(UserGroupBase,0x0d8,getlogin())
#pragma amicall(UserGroupBase,0x0de,setlogin(a1))
#pragma amicall(UserGroupBase,0x0e4,setutent())
#pragma amicall(UserGroupBase,0x0ea,getutent())
#pragma amicall(UserGroupBase,0x0f0,endutent())
#pragma amicall(UserGroupBase,0x0f6,getlastlog(d0))
#pragma amicall(UserGroupBase,0x0fc,setlastlog(d0,a0,a1))
#pragma amicall(UserGroupBase,0x102,getcredentials(a0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall UserGroupBase ug_SetupContextTagList 01e 9802
#pragma  libcall UserGroupBase ug_GetErr              024 00
#pragma  libcall UserGroupBase ug_StrError            02a 101
#pragma  libcall UserGroupBase getuid                 030 00
#pragma  libcall UserGroupBase geteuid                036 00
#pragma  libcall UserGroupBase setreuid               03c 1002
#pragma  libcall UserGroupBase setuid                 042 001
#pragma  libcall UserGroupBase getgid                 048 00
#pragma  libcall UserGroupBase getegid                04e 00
#pragma  libcall UserGroupBase setregid               054 1002
#pragma  libcall UserGroupBase setgid                 05a 001
#pragma  libcall UserGroupBase getgroups              060 9002
#pragma  libcall UserGroupBase setgroups              066 9002
#pragma  libcall UserGroupBase initgroups             06c 0902
#pragma  libcall UserGroupBase getpwnam               072 901
#pragma  libcall UserGroupBase getpwuid               078 001
#pragma  libcall UserGroupBase setpwent               07e 00
#pragma  libcall UserGroupBase getpwent               084 00
#pragma  libcall UserGroupBase endpwent               08a 00
#pragma  libcall UserGroupBase getgrnam               090 901
#pragma  libcall UserGroupBase getgrgid               096 001
#pragma  libcall UserGroupBase setgrent               09c 00
#pragma  libcall UserGroupBase getgrent               0a2 00
#pragma  libcall UserGroupBase endgrent               0a8 00
#pragma  libcall UserGroupBase crypt                  0ae 9802
#pragma  libcall UserGroupBase ug_GetSalt             0b4 09803
#pragma  libcall UserGroupBase getpass                0ba 901
#pragma  libcall UserGroupBase umask                  0c0 001
#pragma  libcall UserGroupBase getumask               0c6 00
#pragma  libcall UserGroupBase setsid                 0cc 00
#pragma  libcall UserGroupBase getpgrp                0d2 00
#pragma  libcall UserGroupBase getlogin               0d8 00
#pragma  libcall UserGroupBase setlogin               0de 901
#pragma  libcall UserGroupBase setutent               0e4 00
#pragma  libcall UserGroupBase getutent               0ea 00
#pragma  libcall UserGroupBase endutent               0f0 00
#pragma  libcall UserGroupBase getlastlog             0f6 001
#pragma  libcall UserGroupBase setlastlog             0fc 98003
#pragma  libcall UserGroupBase getcredentials         102 801
#endif
#ifdef __STORM__
#pragma tagcall(UserGroupBase,0x01e,ug_SetupContextTags(a0,a1))
#endif
#ifdef __SASC_60
#pragma  tagcall UserGroupBase ug_SetupContextTags    01e 9802
#endif

#endif	/*  PRAGMAS_USERGROUP_H  */
