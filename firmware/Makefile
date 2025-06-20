##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

ifeq ($(BOARD),)
  BOARD = f0_module
endif

BOARDDIR = boards/$(BOARD)

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant
endif

USE_OPT += -DWB_PROD=1

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti -fno-exceptions -ffast-math -funsafe-math-optimizations -fno-threadsafe-statics -fno-use-cxa-atexit -std=c++20
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

ifeq ($(USE_BOOTLOADER),)
  USE_BOOTLOADER = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = wideband

# Imported source files and paths.
CHIBIOS  := ChibiOS
CONFDIR  := $(BOARDDIR)
BUILDDIR := ./build
DEPDIR   := ./.dep
FIRMWARE_DIR = .

ifeq ("$(wildcard $(CHIBIOS)/os/license/license.mk)","")
# submodules probably aren't checked out, complain to the user about it!
# make is not happy about newly checked out module for some reason but next invocation would work
$(error Please run 'git submodule update --init --recursive' before trying to build!)
endif

# Configure libfirmware Paths/Includes
RUSEFI_LIB = libfirmware
include $(RUSEFI_LIB)/util/util.mk

# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
# Other files (optional).
include $(CHIBIOS)/os/hal/lib/streams/streams.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk
include $(CHIBIOS)/os/hal/lib/complex/mfs/hal_mfs.mk

# include board.mk that sets per-board options
include $(BOARDDIR)/board.mk

include wideband.mk

# Define linker script file here
LDSCRIPT=app.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) $(BOARDDIR)/board.c

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC) \
          $(BOARDDIR)/port.cpp \
          $(BOARDDIR)/port_shared.cpp \
          $(RUSEFI_LIB_CPP) \
          $(WIDEBANDSRC) \
          shared/flash.cpp \
          can.cpp \
          can_helper.cpp \
          fault.cpp \
          lambda_conversion.cpp \
          pwm.cpp \
          dac.cpp \
          pump_dac.cpp \
          pump_control.cpp \
          max3185x.cpp \
          uart.cpp \
          auxout.cpp \
          indication.cpp \
          sampling_thread.cpp \
          heater_thread.cpp \
          main.cpp

ifneq ($(ENABLE_TS),)
CPPSRC += console/binary/tunerstudio.cpp \
          console/binary/tunerstudio_io.cpp \
          console/binary/tunerstudio_io_serial.cpp \
          console/binary/tunerstudio_commands.cpp \
          livedata.cpp \

DDEFS += -DTS_ENABLED=TRUE
endif

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC) main_hardfault_asm.S

# Inclusion directories.
INCDIR = $(CONFDIR) \
         $(ALLINC) \
         $(RUSEFI_LIB_INC) \
         util/ \
         util/math/ \
         console/binary/ \
         boards/ \
         shared/ \
         $(BOARDDIR)/io/

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef -Werror=switch

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

# OpenBLT flasher expects srec files
SREC = $(CP) -O srec

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
#UDEFS = -DCHPRINTF_USE_FLOAT=1


# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR = $(BOARDDIR)

ifeq ($(USE_BOOTLOADER),yes)
  ULIBS = boards/f0_module/bootloader/build/wideband_bootloader.o
  USE_OPT += -Wl,--defsym=USE_BOOTLOADER=1
endif

ifeq ($(USE_OPENBLT),yes)
  # Reserve start of flash for OpenBLT
  USE_OPT += -Wl,--defsym=USE_BOOTLOADER=1
  DDEFS += -DUSE_OPENBLT=TRUE
  # Shared params
  INCDIR += boards/f1_common/openblt
  CSRC += boards/f1_common/openblt/shared_params.c
endif

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk

#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#

#
# Custom rules
##############################################################################
