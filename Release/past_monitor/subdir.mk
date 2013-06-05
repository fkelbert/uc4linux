################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../past_monitor/lex.yy.o \
../past_monitor/past_monitor.o \
../past_monitor/past_parser.tab.o 

C_SRCS += \
../past_monitor/lex.yy.c \
../past_monitor/past_monitor.c \
../past_monitor/past_monitor_demo.c \
../past_monitor/past_parser.tab.c 

OBJS += \
./past_monitor/lex.yy.o \
./past_monitor/past_monitor.o \
./past_monitor/past_monitor_demo.o \
./past_monitor/past_parser.tab.o 

C_DEPS += \
./past_monitor/lex.yy.d \
./past_monitor/past_monitor.d \
./past_monitor/past_monitor_demo.d \
./past_monitor/past_parser.tab.d 


# Each subdirectory must supply rules for building sources it contributes
past_monitor/%.o: ../past_monitor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


