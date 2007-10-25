# This file is responsible for finding out the availability
# of the required build commands
#

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

