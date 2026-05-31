################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/digit_nn/test/digit_nn_self_test.c \
../Core/Src/digit_nn/test/mnist_samples.c 

OBJS += \
./Core/Src/digit_nn/test/digit_nn_self_test.o \
./Core/Src/digit_nn/test/mnist_samples.o 

C_DEPS += \
./Core/Src/digit_nn/test/digit_nn_self_test.d \
./Core/Src/digit_nn/test/mnist_samples.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/digit_nn/test/digit_nn_self_test.o: ../Core/Src/digit_nn/test/digit_nn_self_test.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32F103xE -DUSE_HAL_DRIVER -DDEBUG -c -I../Core/Inc -I../Core/Inc/digit_nn/core -I../Core/Inc/digit_nn/generated -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/digit_nn/test/digit_nn_self_test.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/digit_nn/test/mnist_samples.o: ../Core/Src/digit_nn/test/mnist_samples.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32F103xE -DUSE_HAL_DRIVER -DDEBUG -c -I../Core/Inc -I../Core/Inc/digit_nn/core -I../Core/Inc/digit_nn/generated -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/digit_nn/test/mnist_samples.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

