# This file is responsible for build environment "sanity checking",
# namely, finding out the availability of the required build commands


ifndef __BAD_SHELL
# these facilities are available only on Unix style shells

# See if we have access to the which utility
__HAVE_WHICH:=$(shell if which which > __tmp.tmp 2>&1; then echo "1";fi) 

ifdef __HAVE_WHICH
check_cmd=$(shell if which $(1) > __tmp.tmp 2>&1; then echo "";else echo $(1);fi)

# always check for gcc command existance
__NOCMD:=$(call check_cmd,$(CC))

ifeq "$(CHECK_NASM)" "1"
__NOCMD_TMP:=$(call check_cmd,$(NASM))
ifdef __NOCMD_TMP
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
endif

ifeq "$(CHECK_WINDRES)" "1"
ifeq "$(TARGET_OS)" "WIN32"
__NOCMD_TMP:=$(call check_cmd,$(WINDRES))
ifdef __NOCMD_TMP
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
endif
endif

ifeq "$(CHECK_SDL)" "1"
ifeq "$(TARGET_OS)" "UNIX"
__NOCMD_TMP:=$(call check_cmd,$(CMD_SDLCFG))
ifdef __NOCMD_TMP
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
# FIXME what about win32
endif
endif

# end of __HAVE_WHICH condition
endif

# end of __BAD_SHELL condition
endif

