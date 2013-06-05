################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shell_wrapper/shell_wrapper.c 

OBJS += \
./shell_wrapper/shell_wrapper.o 

C_DEPS += \
./shell_wrapper/shell_wrapper.d 


# Each subdirectory must supply rules for building sources it contributes
shell_wrapper/%.o: ../shell_wrapper/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


