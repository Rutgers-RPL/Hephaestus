################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/littlefs/lfs.c \
../Core/Src/littlefs/lfs_util.c 

OBJS += \
./Core/Src/littlefs/lfs.o \
./Core/Src/littlefs/lfs_util.o 

C_DEPS += \
./Core/Src/littlefs/lfs.d \
./Core/Src/littlefs/lfs_util.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/littlefs/%.o Core/Src/littlefs/%.su Core/Src/littlefs/%.cyclo: ../Core/Src/littlefs/%.c Core/Src/littlefs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Core/Inc/littlefs -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-littlefs

clean-Core-2f-Src-2f-littlefs:
	-$(RM) ./Core/Src/littlefs/lfs.cyclo ./Core/Src/littlefs/lfs.d ./Core/Src/littlefs/lfs.o ./Core/Src/littlefs/lfs.su ./Core/Src/littlefs/lfs_util.cyclo ./Core/Src/littlefs/lfs_util.d ./Core/Src/littlefs/lfs_util.o ./Core/Src/littlefs/lfs_util.su

.PHONY: clean-Core-2f-Src-2f-littlefs

