###############################################################################
## Simulator Makefile
###############################################################################

# TARGETS
TARGETS	   ?= exactstep exactstep-riscv-linux

HAS_SCREEN ?= False
HAS_NETWORK ?= False

# Source Files
SRC_DIR    = core peripherals cpu-rv32 cpu-rv64 cpu-armv6m cpu-mips-i cli platforms device-tree display net virtio sbi

CFLAGS	    = -O2 -fPIC -std=gnu++11
CFLAGS     += -Wno-format
CFLAGS     += -D__USE_RV32__
CFLAGS     += -D__USE_RV64__
CFLAGS     += -D__USE_ARMV6M__
CFLAGS     += -D__USE_MIPS1__
ifneq ($(HAS_NETWORK),False)
  CFLAGS   += -DINCLUDE_NET_DEVICE
endif
ifneq ($(HAS_SCREEN),False)
  CFLAGS   += -DINCLUDE_SCREEN
endif

INCLUDE_PATH += $(SRC_DIR)
CFLAGS       += $(patsubst %,-I%,$(INCLUDE_PATH))

LDFLAGS     = 
LIBS        = -lelf -lbfd -lfdt

ifneq ($(HAS_SCREEN),False)
  LIBS     += -lSDL
endif

###############################################################################
# Variables
###############################################################################
OBJ_DIR      ?= obj/

###############################################################################
# Variables: Lists of objects, source and deps
###############################################################################
# SRC / Object list
src2obj       = $(OBJ_DIR)$(patsubst %$(suffix $(1)),%.o,$(notdir $(1)))

SRC          ?= $(foreach src,$(SRC_DIR),$(wildcard $(src)/*.cpp))
SRC_FILT     := $(filter-out cli/main.cpp,$(SRC))
SRC_FILT     := $(filter-out cli/main_riscv_linux.cpp,$(SRC_FILT))

OBJ          ?= $(foreach src,$(SRC_FILT),$(call src2obj,$(src)))

###############################################################################
# Rules: Compilation macro
###############################################################################
define template_cpp
$(call src2obj,$(1)): $(1) | $(OBJ_DIR)
	@echo "# Compiling $(notdir $(1))"
	@g++ $(CFLAGS) -c $$< -o $$@
endef

###############################################################################
# Rules
###############################################################################
all: $(TARGETS) 
	
$(OBJ_DIR):
	@mkdir -p $@

$(foreach src,$(SRC),$(eval $(call template_cpp,$(src))))	

exactstep: $(OBJ) $(OBJ_DIR)main.o makefile
	@echo "# Linking $(notdir $@)"
	@g++ $(LDFLAGS) $(OBJ_DIR)main.o $(OBJ) $(LIBS) -o $@

exactstep-riscv-linux: $(OBJ) $(OBJ_DIR)main_riscv_linux.o makefile
	@echo "# Linking $(notdir $@)"
	@g++ $(LDFLAGS) $(OBJ_DIR)main_riscv_linux.o $(OBJ) $(LIBS) -o $@

clean:
	-rm -rf $(OBJ_DIR) $(TARGETS)

