################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../timer\ opgave.c 

OBJS += \
./timer\ opgave.o 

C_DEPS += \
./timer\ opgave.d 


# Each subdirectory must supply rules for building sources it contributes
timer\ opgave.o: ../timer\ opgave.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c99 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"timer opgave.d" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


