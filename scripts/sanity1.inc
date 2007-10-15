# This file is responsible for build environment "sanity checking",
# namely, finding out the availability of the required build commands


check_cmd=$(shell if which $(1) > __tmp.tmp 2>&1; then echo "";else echo $(1);fi)

# we need access to the which utility
_DONT_HAVE_WHICH:=$(call check_cmd,which)

ifneq ($(_DONT_HAVE_WHICH),)
$(error which utility not found.)
endif

# always check for gcc command existance
__NOCMD:=$(call check_cmd,$(CC))

ifeq ($(CHECK_NASM),1)
__NOCMD_TMP:=$(call check_cmd,$(NASM))
ifneq ($(__NOCMD_TMP),)
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
endif

ifeq ($(CHECK_WINDRES),1)
ifeq ($(TARGET_OS),win32)
__NOCMD_TMP:=$(call check_cmd,$(WINDRES))
ifneq ($(__NOCMD_TMP),)
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
endif
endif

ifeq ($(CHECK_SDL),1)
ifeq ($(TARGET_OS),unix)
__NOCMD_TMP:=$(call check_cmd,$(CMD_SDLCFG))
ifneq ($(__NOCMD_TMP),)
__NOCMD:=$(__NOCMD) $(__NOCMD_TMP)
endif
# FIXME what about win32
endif
endif

