# makefile fragment to be included by libtimidity.a users
# $Id$

ifdef DEBUG
TIMIDITY_BUILD = DEBUG=yes
endif
ifeq ($(TIMIDITY_USE_DLS),)
TIMIDITY_USE_DLS=no
endif
$(LIBS_DIR)/timidity/libtimidity.a:
	$(MAKE) -C $(LIBS_DIR)/timidity $(TIMIDITY_BUILD) USE_DLS="$(TIMIDITY_USE_DLS)" CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)"

timi_clean:
	$(MAKE) -C $(LIBS_DIR)/timidity clean

