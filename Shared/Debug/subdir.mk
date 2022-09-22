################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../client_utils.c \
../server_utils.c \
../shared_utils.c 

OBJS += \
./client_utils.o \
./server_utils.o \
./shared_utils.o 

C_DEPS += \
./client_utils.d \
./server_utils.d \
./shared_utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/matias/Desktop/utn/SO/tp-2022-1c-Grupo-999/Shared" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


