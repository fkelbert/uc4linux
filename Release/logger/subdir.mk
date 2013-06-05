################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../logger/logger.o 

C_SRCS += \
../logger/logger.c 

OBJS += \
./logger/logger.o 

C_DEPS += \
./logger/logger.d 


# Each subdirectory must supply rules for building sources it contributes
logger/%.o: ../logger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


