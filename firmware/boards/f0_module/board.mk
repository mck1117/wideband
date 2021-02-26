USE_BOOTLOADER = yes

MCU = cortex-m0

include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f0xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F0xx/platform.mk
include $(CHIBIOS)/os/common/ports/ARMv6-M/compilers/GCC/mk/port.mk
