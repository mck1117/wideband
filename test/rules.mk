# ARM Cortex-Mx common makefile scripts and rules.

ifeq ($(BUILDDIR),)
  # Define if not specified
  BUILDDIR = build
endif
ifeq ($(BUILDDIR),.)
  # Redefine if pointing at current folder
  BUILDDIR = build
endif
BINARY_OUTPUT = $(BUILDDIR)/$(PROJECT)

# Automatic compiler options
OPT = $(USE_OPT)
COPT = $(USE_COPT)
CPPOPT = $(USE_CPPOPT)
ifeq ($(USE_LINK_GC),yes)
  OPT += -ffunction-sections -fdata-sections -fno-common
endif

ACSRC += $(CSRC)
ACPPSRC += $(CPPSRC)

ASRC	  = $(ACSRC)$(ACPPSRC)
SRCPATHS  = $(sort $(dir $(ASMXSRC)) $(dir $(ASMSRC)) $(dir $(ASRC)))

# Various directories
OBJDIR    = $(BUILDDIR)/obj
LSTDIR    = $(BUILDDIR)/lst

# Object files groups
ACOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ACSRC:.c=.o)))
ACPPOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ACPPSRC:.cpp=.o)))
ASMOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
ASMXOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
OBJS	  = $(ASMXOBJS) $(ASMOBJS) $(ACOBJS) $(ACPPOBJS)

# Paths
IINCDIR   = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
LLIBDIR   = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

# Macros
DEFS      = $(DDEFS) $(UDEFS)
ADEFS 	  = $(DADEFS) $(UADEFS)

# Libs
LIBS      = $(DLIBS) $(ULIBS)

# Various settings
ifeq ($(IS_MAC),yes)
	ODFLAGS	  = -x --syms
	ASFLAGS   = $(MCFLAGS) -Wa $(ADEFS)
	ASXFLAGS  = $(MCFLAGS) -Wa $(ADEFS)
	CFLAGS    = $(MCFLAGS) $(OPT) $(COPT)   $(CWARN)   $(DEFS)
	CPPFLAGS  = $(MCFLAGS) $(OPT) $(CPPOPT) $(CPPWARN) $(DEFS)
	LDFLAGS = $(MCFLAGS) $(LLIBDIR)
else
	# not mac
	ODFLAGS	  = -x --syms
	ASFLAGS   = $(MCFLAGS) $(ADEFS)
	ASXFLAGS  = $(MCFLAGS) $(ADEFS)
	CFLAGS    = $(MCFLAGS) $(OPT) $(COPT) $(CWARN) $(DEFS)
	CPPFLAGS  = $(MCFLAGS) $(OPT) $(CPPOPT) $(CPPWARN) $(DEFS)
	ifeq ($(USE_LINK_GC),yes)
	  LDFLAGS = $(MCFLAGS) -Wl,-Map=$(BINARY_OUTPUT).map,--cref,--no-warn-mismatch,--gc-sections $(LLIBDIR)
	else
	  LDFLAGS = $(MCFLAGS) -Wl,-Map=$(BINARY_OUTPUT).map,--cref,--no-warn-mismatch $(LLIBDIR)
	endif
endif

# Generate dependency information
CFLAGS   += -MD -MP -MF .dep/$(@F).d
CPPFLAGS += -MD -MP -MF .dep/$(@F).d

# Paths where to search for sources
VPATH     = $(SRCPATHS)

#
# Makefile rules
#

all: $(OBJS) $(BINARY_OUTPUT) MAKE_ALL_RULE_HOOK

MAKE_ALL_RULE_HOOK:

$(OBJS): | $(BUILDDIR)

$(BUILDDIR) $(OBJDIR) $(LSTDIR):
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo Compiler Options
	@echo $(CPPC) -c $(CPPFLAGS) -I. $(IINCDIR) main.cpp -o main.o
	@echo
endif
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)

$(ACPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(ACOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(ASMOBJS) : $(OBJDIR)/%.o : %.s Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
endif

$(ASMXOBJS) : $(OBJDIR)/%.o : %.S Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo 
	$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

$(BINARY_OUTPUT): $(OBJS)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
else
	@echo Linking $@
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif

clean: CLEAN_RULE_HOOK
	@echo Cleaning
	-rm -fR .dep $(BUILDDIR)
	@echo Done

CLEAN_RULE_HOOK:

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
