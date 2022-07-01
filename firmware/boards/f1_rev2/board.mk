USE_BOOTLOADER = no

MCU = cortex-m3

include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f1xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/platform.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk
