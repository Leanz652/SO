################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/client_utils.c \
../src/server_utils.c \
../src/shared_utils.c 

OBJS += \
./src/client_utils.o \
./src/server_utils.o \
./src/shared_utils.o 

C_DEPS += \
./src/client_utils.d \
./src/server_utils.d \
./src/shared_utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/matias/Desktop/utn/SO/tp-2022-1c-Grupo-999/Shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


