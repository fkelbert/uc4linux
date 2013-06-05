################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../trust_manager/trust_manager.c 

OBJS += \
./trust_manager/trust_manager.o 

C_DEPS += \
./trust_manager/trust_manager.d 


# Each subdirectory must supply rules for building sources it contributes
trust_manager/%.o: ../trust_manager/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


