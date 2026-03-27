##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

PROJECT_DIR = .

FIRMWARE_DIR = ./../firmware

# Imported source files and paths
RUSEFI_LIB = $(FIRMWARE_DIR)/libfirmware
include $(RUSEFI_LIB)/util/util.mk

include $(FIRMWARE_DIR)/wideband.mk

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = \
	$(RUSEFI_LIB_C) \

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = \
	$(RUSEFI_LIB_CPP) \
	$(RUSEFI_LIB_CPP_TEST) \
	$(WIDEBANDSRC) \
	gtest-all.cpp \
	gmock-all.cpp \
	gtest_main.cpp \
	test_stubs.cpp \
	tests/test_sampler.cpp \
	tests/test_heater.cpp \
	$(RUSEFI_LIB)/util/src/timer.cpp \
	$(RUSEFI_LIB)/mock/lib-time-mocks.cpp \

INCDIR = \
	$(PROJECT_DIR)/googletest/googlemock/ \
	$(PROJECT_DIR)/googletest/googlemock/include \
	$(PROJECT_DIR)/googletest/googletest \
	$(PROJECT_DIR)/googletest/googletest/include \
	$(RUSEFI_LIB_INC) \
	$(RUSEFI_LIB)/mock \
	$(FIRMWARE_DIR) \
	$(FIRMWARE_DIR)/boards \
	$(FIRMWARE_DIR)/util \

# User may want to pass in a forced value for SANITIZE
ifeq ($(SANITIZE),)
	ifneq ($(OS),Windows_NT)
		SANITIZE = yes
	else
		SANITIZE = no
	endif
endif

IS_MAC = no
ifneq ($(OS),Windows_NT)
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        IS_MAC = yes
    endif
endif

# Compiler options here.
ifeq ($(USE_OPT),)
  #USE_OPT = $(RFLAGS) -O2 -fgnu89-inline -ggdb -fomit-frame-pointer -falign-functions=16 -std=gnu99 -Werror-implicit-function-declaration -Werror -Wno-error=pointer-sign -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=sign-compare -Wno-error=unused-parameter -Wno-error=missing-field-initializers
  USE_OPT = -c -Wall -O0 -ggdb -g
  USE_OPT += -Werror=missing-field-initializers
endif

USE_OPT += -DWB_PROD=0

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -std=gnu99 -fgnu89-inline
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -std=c++20 -fno-rtti -fno-use-cxa-atexit
endif

USE_CPPOPT += -DMOCK_TIMER

# Enable address sanitizer for C++ files, but not on Windows since x86_64-w64-mingw32-g++ doesn't support it.
# only c++ because lua does some things asan doesn't like, but don't actually cause overruns.
ifeq ($(SANITIZE),yes)
	ifeq ($(IS_MAC),yes)
		USE_CPPOPT += -fsanitize=address
	else
		USE_CPPOPT += -fsanitize=address -fsanitize=bounds-strict -fno-sanitize-recover=all
	endif
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# C sources to be compiled in ARM mode regardless of the global setting.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
ACPPSRC =

# List ASM source files here
ASMSRC =

##############################################################################
# Compiler settings
#

# It looks like cygwin build of mingwg-w64 has issues with gcov runtime :(
# mingw-w64 is a project which forked from mingw in 2007 - be careful not to confuse these two.
# In order to have coverage generated please download from https://mingw-w64.org/doku.php/download/mingw-builds
# Install using mingw-w64-install.exe instead of similar thing packaged with cygwin
# Both 32 bit and 64 bit versions of mingw-w64 are generating coverage data.

ifeq ($(OS),Windows_NT)
ifeq ($(USE_MINGW32_I686),)
#this one is 64 bit
  TRGT = x86_64-w64-mingw32-
else
#this one was 32 bit
  TRGT = i686-w64-mingw32-
endif
else
  TRGT =
endif

CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes -pedantic -Wmissing-prototypes -Wold-style-definition

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Werror -Wno-error=sign-compare

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
ifeq ($(OS),Windows_NT)
  # Windows
  DLIBS = -static-libgcc -static -static-libstdc++
else
  # Linux
  DLIBS = -pthread
endif

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lm

ifeq ($(SANITIZE),yes)
	ULIBS += -fsanitize=address -fsanitize=undefined
endif

#
# End of user defines
##############################################################################

# Define project name here
PROJECT = wideband_test

include rules.mk
