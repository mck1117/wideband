USE_BOOTLOADER = no
ENABLE_TS = yes

MCU = cortex-m3

ALLCPPSRC += $(BOARDDIR)/../f1_common/f1_port.cpp

include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f1xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/platform.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk
