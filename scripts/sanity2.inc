# This file provides the "sanity" makefile target which may
# be used as a prerequisite for other targets

sanity:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)
ifneq ($(__NOCMD),)
	@echo "Sanity   : Error," command\(s\) $(__NOCMD) not found
	@exit 2
endif
	@echo "Sanity   :" OK

