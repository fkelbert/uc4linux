################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../control_monitor/control_monitor.c 

OBJS += \
./control_monitor/control_monitor.o 

C_DEPS += \
./control_monitor/control_monitor.d 


# Each subdirectory must supply rules for building sources it contributes
control_monitor/%.o: ../control_monitor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


