################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../event/event.c \
../event/event_trace.c 

OBJS += \
./event/event.o \
./event/event_trace.o 

C_DEPS += \
./event/event.d \
./event/event_trace.d 


# Each subdirectory must supply rules for building sources it contributes
event/%.o: ../event/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


