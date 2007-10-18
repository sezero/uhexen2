# This file is responsible for build environment "sanity checking",
# namely, finding out the availability of the required build commands


check_cmd=$(shell if which $(1) > /dev/null 2>&1; then echo "";else echo $(1);fi)

# we need access to the which utility
_DONT_HAVE_WHICH:=$(call check_cmd,which)

ifneq ($(_DONT_HAVE_WHICH),)
$(error which utility not found.)
endif

# always check for gcc command existance
__NOCMD:=$(call check_cmd,$(CC))

ifeq ($(CHECK_NASM),1)
__NOCMD:=$(__NOCMD) $(call check_cmd,$(NASM))
endif

ifeq ($(CHECK_WINDRES),1)
ifeq ($(TARGET_OS),win32)
__NOCMD:=$(__NOCMD) $(call check_cmd,$(WINDRES))
endif
endif

ifeq ($(CHECK_SDL),1)
ifeq ($(TARGET_OS),unix)
__NOCMD:=$(__NOCMD) $(call check_cmd,$(SDL_CONFIG))
# FIXME what about win32
endif
endif

