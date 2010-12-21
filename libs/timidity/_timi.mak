# makefile fragment to be included by libtimidity users

ifdef DEBUG
TIMIDITY_BUILD = DEBUG=yes
endif
$(LIBS_DIR)/timidity/libtimidity.a:
	$(MAKE) -C $(LIBS_DIR)/timidity $(TIMIDITY_BUILD) CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)"

