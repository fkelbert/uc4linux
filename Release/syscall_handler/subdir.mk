################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../syscall_handler/syscall_handler.c 

OBJS += \
./syscall_handler/syscall_handler.o 

C_DEPS += \
./syscall_handler/syscall_handler.d 


# Each subdirectory must supply rules for building sources it contributes
syscall_handler/%.o: ../syscall_handler/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


