################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../handler/utilities/encryption.c \
../handler/utilities/functions.c \
../handler/utilities/linked_list.c 

OBJS += \
./handler/utilities/encryption.o \
./handler/utilities/functions.o \
./handler/utilities/linked_list.o 

C_DEPS += \
./handler/utilities/encryption.d \
./handler/utilities/functions.d \
./handler/utilities/linked_list.d 


# Each subdirectory must supply rules for building sources it contributes
handler/utilities/%.o: ../handler/utilities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


