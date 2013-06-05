################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../attestation_service/attestation_client.c \
../attestation_service/attestation_service.c 

OBJS += \
./attestation_service/attestation_client.o \
./attestation_service/attestation_service.o 

C_DEPS += \
./attestation_service/attestation_client.d \
./attestation_service/attestation_service.d 


# Each subdirectory must supply rules for building sources it contributes
attestation_service/%.o: ../attestation_service/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


