################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../syscall_handler/syscall_handler.c \
../syscall_handler/syscalls.c 

OBJS += \
./syscall_handler/syscall_handler.o \
./syscall_handler/syscalls.o 

C_DEPS += \
./syscall_handler/syscall_handler.d \
./syscall_handler/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
syscall_handler/%.o: ../syscall_handler/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


