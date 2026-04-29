################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/flash/flash.c \
../Core/Src/flash/gd5f1gq5xe.c \
../Core/Src/flash/w25n01kv.c 

OBJS += \
./Core/Src/flash/flash.o \
./Core/Src/flash/gd5f1gq5xe.o \
./Core/Src/flash/w25n01kv.o 

C_DEPS += \
./Core/Src/flash/flash.d \
./Core/Src/flash/gd5f1gq5xe.d \
./Core/Src/flash/w25n01kv.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/flash/%.o Core/Src/flash/%.su Core/Src/flash/%.cyclo: ../Core/Src/flash/%.c Core/Src/flash/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Core/Inc/littlefs -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-flash

clean-Core-2f-Src-2f-flash:
	-$(RM) ./Core/Src/flash/flash.cyclo ./Core/Src/flash/flash.d ./Core/Src/flash/flash.o ./Core/Src/flash/flash.su ./Core/Src/flash/gd5f1gq5xe.cyclo ./Core/Src/flash/gd5f1gq5xe.d ./Core/Src/flash/gd5f1gq5xe.o ./Core/Src/flash/gd5f1gq5xe.su ./Core/Src/flash/w25n01kv.cyclo ./Core/Src/flash/w25n01kv.d ./Core/Src/flash/w25n01kv.o ./Core/Src/flash/w25n01kv.su

.PHONY: clean-Core-2f-Src-2f-flash

