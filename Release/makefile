################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include utilities/subdir.mk
-include trust_manager/subdir.mk
-include tests/subdir.mk
-include syscall_handler/subdir.mk
-include shell_wrapper/subdir.mk
-include past_monitor/subdir.mk
-include logger/subdir.mk
-include event_signaler/subdir.mk
-include event/subdir.mk
-include control_monitor/subdir.mk
-include attestation_service/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: OSLFramework.exe

# Tool invocations
OSLFramework.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cygwin C Linker'
	gcc  -o"OSLFramework.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) OSLFramework.exe
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
