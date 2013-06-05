################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/crypto_utils.c \
../utilities/filesystem_utils.c \
../utilities/hash_table.c \
../utilities/network_utils.c \
../utilities/openbsd_utils.c \
../utilities/soup_utils.c \
../utilities/str_utils.c \
../utilities/systrace_utils.c \
../utilities/threads.c \
../utilities/time_utils.c \
../utilities/tss_utils.c \
../utilities/xml_utils.c 

OBJS += \
./utilities/crypto_utils.o \
./utilities/filesystem_utils.o \
./utilities/hash_table.o \
./utilities/network_utils.o \
./utilities/openbsd_utils.o \
./utilities/soup_utils.o \
./utilities/str_utils.o \
./utilities/systrace_utils.o \
./utilities/threads.o \
./utilities/time_utils.o \
./utilities/tss_utils.o \
./utilities/xml_utils.o 

C_DEPS += \
./utilities/crypto_utils.d \
./utilities/filesystem_utils.d \
./utilities/hash_table.d \
./utilities/network_utils.d \
./utilities/openbsd_utils.d \
./utilities/soup_utils.d \
./utilities/str_utils.d \
./utilities/systrace_utils.d \
./utilities/threads.d \
./utilities/time_utils.d \
./utilities/tss_utils.d \
./utilities/xml_utils.d 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


