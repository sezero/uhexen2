# This file provides the "sanity" makefile target which may
# be used as a prerequisite for other targets

sanity:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)
ifdef __NOCMD
	@echo "Sanity   : Error," command\(s\) $(__NOCMD) not found
	@exit 2
endif
ifndef __HAVE_WHICH
	@echo Warning: Could not find the which utility program.
endif
ifdef __BAD_SHELL
	@echo Warning: You are NOT running on a Unix style shell,
	@echo Warning: certain compiler flag checks are disabled.
	@echo Warning: This might result in failures and even in
	@echo Warning: broken binaries. Using MSYS is highly
	@echo Warning: recommended.
else
	@echo "Sanity   :" OK
endif

