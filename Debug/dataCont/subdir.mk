################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../dataCont/data_flow_monitor_service.o 

C_SRCS += \
../dataCont/dataCont.c \
../dataCont/data_flow_monitor.c \
../dataCont/data_flow_monitor_service.c \
../dataCont/list.c \
../dataCont/list_of_events.c 

OBJS += \
./dataCont/dataCont.o \
./dataCont/data_flow_monitor.o \
./dataCont/data_flow_monitor_service.o \
./dataCont/list.o \
./dataCont/list_of_events.o 

C_DEPS += \
./dataCont/dataCont.d \
./dataCont/data_flow_monitor.d \
./dataCont/data_flow_monitor_service.d \
./dataCont/list.d \
./dataCont/list_of_events.d 


# Each subdirectory must supply rules for building sources it contributes
dataCont/%.o: ../dataCont/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


