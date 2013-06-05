################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../event_signaler/event_handler.c \
../event_signaler/event_manager.c \
../event_signaler/event_signaler.c 

OBJS += \
./event_signaler/event_handler.o \
./event_signaler/event_manager.o \
./event_signaler/event_signaler.o 

C_DEPS += \
./event_signaler/event_handler.d \
./event_signaler/event_manager.d \
./event_signaler/event_signaler.d 


# Each subdirectory must supply rules for building sources it contributes
event_signaler/%.o: ../event_signaler/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


