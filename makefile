#******************************************************************************
# Generated by VisualTeensy V1.4.0 on 2/8/2022 at 3:10 PM
#
# Board              Teensy 4.0 or 4.1 depending on BOARD_ID
# USB Type           Serial
# CPU Speed          600 MHz
# Optimize           Faster
# Keyboard Layout    US English
#
# https://github.com/luni64/VisualTeensy
#******************************************************************************
SHELL            := cmd.exe
export SHELL

TARGET_NAME      := SerialProtoWork
# BOARD_ID         := TEENSY41
BOARD_ID         := TEENSY40

MCU              := imxrt1062

LIBS_SHARED_BASE := $(ARDUINO_LIBS)
LIBS_SHARED      :=  

LIBS_LOCAL_BASE  := lib
LIBS_LOCAL       := FastCRC tinycbor

CORE_BASE        := $(ARDUINO_HARDWARE)\teensy\avr\cores\teensy4
GCC_BASE         := $(ARDUINO_HARDWARE)\tools\arm\bin
UPL_PJRC_B       := $(ARDUINO_HARDWARE)\tools
UPL_TYCMD_B      := $(TY_TOOLS)

#******************************************************************************
# Flags and Defines
#******************************************************************************

FLAGS_CPU   := -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16
FLAGS_OPT   := -O2
FLAGS_COM   := -g -Wall -ffunction-sections -fdata-sections -nostdlib -MMD
FLAGS_LSP   := 

FLAGS_CPP   := -std=gnu++14 -fno-exceptions -fpermissive -fno-rtti -fno-threadsafe-statics -felide-constructors -Wno-error=narrowing
FLAGS_C     := 
FLAGS_S     := -x assembler-with-cpp
ifeq ($(BOARD_ID),TEENSY41)
FLAGS_LD    := -Wl,--print-memory-usage,--gc-sections,--relax -T$(CORE_BASE)/imxrt1062_t41.ld
else
FLAGS_LD    := -Wl,--print-memory-usage,--gc-sections,--relax -T$(CORE_BASE)/imxrt1062.ld
endif

LIBS        := -larm_cortexM7lfsp_math -lm -lstdc++

DEFINES     := -D__IMXRT1062__ -DTEENSYDUINO=156 -DARDUINO_$(BOARD_ID) -DARDUINO=10813
DEFINES     += -DF_CPU=600000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH

CPP_FLAGS   := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_CPP)
C_FLAGS     := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_C)
S_FLAGS     := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_S)
LD_FLAGS    := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_LSP) $(FLAGS_LD)
AR_FLAGS    := rcs
NM_FLAGS    := --numeric-sort --defined-only --demangle --print-size

#******************************************************************************
# Colors
#******************************************************************************
COL_CORE    := [38;2;187;206;251m
COL_LIB     := [38;2;206;244;253m
COL_SRC     := [38;2;100;149;237m
COL_LINK    := [38;2;255;255;202m
COL_ERR     := [38;2;255;159;159m
COL_OK      := [38;2;179;255;179m
COL_RESET   := [0m

#******************************************************************************
# Folders and Files
#******************************************************************************
USR_SRC         := firmware
LIB_SRC         := lib
CORE_SRC        := $(CORE_BASE)

BIN             := .vsteensy/build
USR_BIN         := $(BIN)/src
CORE_BIN        := $(BIN)/core
LIB_BIN         := $(BIN)/lib
CORE_LIB        := $(BIN)/core.a
TARGET_HEX      := $(BIN)/$(TARGET_NAME).hex
TARGET_ELF      := $(BIN)/$(TARGET_NAME).elf
TARGET_LST      := $(BIN)/$(TARGET_NAME).lst
TARGET_SYM      := $(BIN)/$(TARGET_NAME).sym

#******************************************************************************
# BINARIES
#******************************************************************************
CC              := $(GCC_BASE)/arm-none-eabi-gcc
CXX             := $(GCC_BASE)/arm-none-eabi-g++
AR              := $(GCC_BASE)/arm-none-eabi-gcc-ar
NM              := $(GCC_BASE)/arm-none-eabi-gcc-nm
SIZE            := $(GCC_BASE)/arm-none-eabi-size
OBJDUMP         := $(GCC_BASE)/arm-none-eabi-objdump
OBJCOPY         := $(GCC_BASE)/arm-none-eabi-objcopy
UPL_PJRC        := "$(UPL_PJRC_B)/teensy_post_compile" -test -file=$(TARGET_NAME) -path=$(BIN) -tools="$(UPL_PJRC_B)" -board=$(BOARD_ID) -reboot
UPL_TYCMD       := $(UPL_TYCMD_B)/tyCommanderC upload $(TARGET_HEX) --autostart --wait --multi
UPL_CLICMD      := $(UPL_CLICMD_B)/teensy_loader_cli -mmcu=$(MCU) -v $(TARGET_HEX)
UPL_JLINK       := $(UPL_JLINK_B)/jlink -commanderscript .vsteensy/flash.jlink

#******************************************************************************
# Source and Include Files
#******************************************************************************
# Recursively create list of source and object files in USR_SRC and CORE_SRC
# and corresponding subdirectories.
# The function rwildcard is taken from http://stackoverflow.com/a/12959694)

rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#User Sources -----------------------------------------------------------------
USR_C_FILES     := $(call rwildcard,$(USR_SRC)/,*.c)
USR_CPP_FILES   := $(call rwildcard,$(USR_SRC)/,*.cpp)
USR_S_FILES     := $(call rwildcard,$(USR_SRC)/,*.S)
USR_OBJ         := $(USR_S_FILES:$(USR_SRC)/%.S=$(USR_BIN)/%.s.o) $(USR_C_FILES:$(USR_SRC)/%.c=$(USR_BIN)/%.c.o) $(USR_CPP_FILES:$(USR_SRC)/%.cpp=$(USR_BIN)/%.cpp.o)

# Core library sources --------------------------------------------------------
CORE_CPP_FILES  := $(call rwildcard,$(CORE_SRC)/,*.cpp)
CORE_C_FILES    := $(call rwildcard,$(CORE_SRC)/,*.c)
CORE_S_FILES    := $(call rwildcard,$(CORE_SRC)/,*.S)
CORE_OBJ        := $(CORE_S_FILES:$(CORE_SRC)/%.S=$(CORE_BIN)/%.s.o) $(CORE_C_FILES:$(CORE_SRC)/%.c=$(CORE_BIN)/%.c.o) $(CORE_CPP_FILES:$(CORE_SRC)/%.cpp=$(CORE_BIN)/%.cpp.o)

# User library sources (see https://github.com/arduino/arduino/wiki/arduino-ide-1.5:-library-specification)
LIB_DIRS_SHARED := $(foreach d, $(LIBS_SHARED), $(LIBS_SHARED_BASE)/$d/ $(LIBS_SHARED_BASE)/$d/utility/)      # base and /utility
LIB_DIRS_SHARED += $(foreach d, $(LIBS_SHARED), $(LIBS_SHARED_BASE)/$d/src/ $(dir $(call rwildcard,$(LIBS_SHARED_BASE)/$d/src/,*/.)))        # src and all subdirs of base

LIB_DIRS_LOCAL  := $(foreach d, $(LIBS_LOCAL), $(LIBS_LOCAL_BASE)/$d/ $(LIBS_LOCAL_BASE)/$d/utility/ )                                       # base and /utility
LIB_DIRS_LOCAL  += $(foreach d, $(LIBS_LOCAL), $(LIBS_LOCAL_BASE)/$d/src/ $(dir $(call rwildcard,$(LIBS_LOCAL_BASE)/$d/src/,*/.)))           # src and all subdirs of base

LIB_CPP_SHARED  := $(foreach d, $(LIB_DIRS_SHARED),$(call wildcard,$d*.cpp))
LIB_C_SHARED    := $(foreach d, $(LIB_DIRS_SHARED),$(call wildcard,$d*.c))
LIB_S_SHARED    := $(foreach d, $(LIB_DIRS_SHARED),$(call wildcard,$d*.S))

LIB_CPP_LOCAL   := $(foreach d, $(LIB_DIRS_LOCAL),$(call wildcard,$d/*.cpp))
LIB_C_LOCAL     := $(foreach d, $(LIB_DIRS_LOCAL),$(call wildcard,$d/*.c))
LIB_S_LOCAL     := $(foreach d, $(LIB_DIRS_LOCAL),$(call wildcard,$d/*.S))

LIB_OBJ         := $(LIB_CPP_SHARED:$(LIBS_SHARED_BASE)/%.cpp=$(LIB_BIN)/%.cpp.o)  $(LIB_CPP_LOCAL:$(LIBS_LOCAL_BASE)/%.cpp=$(LIB_BIN)/%.cpp.o)
LIB_OBJ         += $(LIB_C_SHARED:$(LIBS_SHARED_BASE)/%.c=$(LIB_BIN)/%.c.o)  $(LIB_C_LOCAL:$(LIBS_LOCAL_BASE)/%.c=$(LIB_BIN)/%.c.o)
LIB_OBJ         += $(LIB_S_SHARED:$(LIBS_SHARED_BASE)/%.S=$(LIB_BIN)/%.s.o)  $(LIB_S_LOCAL:$(LIBS_LOCAL_BASE)/%.S=$(LIB_BIN)/%.s.o)

# Includes -------------------------------------------------------------
INCLUDE         := -I./$(USR_SRC) -I$(CORE_SRC)
INCLUDE         += $(foreach d, $(LIBS_LOCAL),-I$(LIBS_LOCAL_BASE)/$d/ -I$(LIBS_LOCAL_BASE)/$d/src -I$(LIBS_LOCAL_BASE)/$d/utility/)
INCLUDE         += $(foreach d, $(LIBS_SHARED), -I$(LIBS_SHARED_BASE)/$d/ -I$(LIBS_SHARED_BASE)/$d/src -I$(LIBS_SHARED_BASE)/$d/utility/)

# Generate directories --------------------------------------------------------
DIRECTORIES     :=  $(sort $(dir $(CORE_OBJ) $(USR_OBJ) $(LIB_OBJ)))
generateDirs    := $(foreach d, $(DIRECTORIES), $(shell if not exist "$d" mkdir "$d"))

#$(info dirs: $(DIRECTORIES))

#******************************************************************************
# Rules:
#******************************************************************************

.PHONY: directories all rebuild upload uploadTy uploadCLI clean cleanUser cleanCore

all:  $(TARGET_LST) $(TARGET_SYM) $(TARGET_HEX)

rebuild: cleanUser all

clean: cleanUser cleanCore cleanLib
	@echo $(COL_OK)cleaning done$(COL_RESET)

upload: all
	@$(UPL_PJRC)

uploadTy: all
	@$(UPL_TYCMD)

uploadCLI: all
	@$(UPL_CLICMD)

uploadJLink: all
	@$(UPL_JLINK)

# Core library ----------------------------------------------------------------
$(CORE_BIN)/%.s.o: $(CORE_SRC)/%.S
	@echo $(COL_CORE)CORE [ASM] $(notdir $<) $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.c.o: $(CORE_SRC)/%.c
	@echo $(COL_CORE)CORE [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.cpp.o: $(CORE_SRC)/%.cpp
	@echo $(COL_CORE)CORE [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_LIB) : $(CORE_OBJ)
	@echo $(COL_LINK)CORE [AR] $@ $(COL_ERR)
	@$(AR) $(AR_FLAGS) $@ $^
	@echo $(COL_OK)Teensy core built successfully &&echo.

# Shared Libraries ------------------------------------------------------------
$(LIB_BIN)/%.s.o: $(LIBS_SHARED_BASE)/%.S
	@echo $(COL_LIB)LIB [ASM] $(notdir $<) $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(LIB_BIN)/%.cpp.o: $(LIBS_SHARED_BASE)/%.cpp
	@echo $(COL_LIB)LIB [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(LIB_BIN)/%.c.o: $(LIBS_SHARED_BASE)/%.c
	@echo $(COL_LIB)LIB [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

# Local Libraries -------------------------------------------------------------
$(LIB_BIN)/%.s.o: $(LIBS_LOCAL_BASE)/%.S
	@echo $(COL_LIB)LIB [ASM] $(notdir $<) $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(LIB_BIN)/%.cpp.o: $(LIBS_LOCAL_BASE)/%.cpp
	@echo $(COL_LIB)LIB [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(LIB_BIN)/%.c.o: $(LIBS_LOCAL_BASE)/%.c
	@echo $(COL_LIB)LIB [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

# Handle user sources ---------------------------------------------------------
$(USR_BIN)/%.s.o: $(USR_SRC)/%.S
	@echo $(COL_SRC)USER [ASM] $< $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.c.o: $(USR_SRC)/%.c
	@echo $(COL_SRC)USER [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.cpp.o: $(USR_SRC)/%.cpp
	@echo $(COL_SRC)USER [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Linking ---------------------------------------------------------------------
$(TARGET_ELF): $(CORE_LIB) $(LIB_OBJ) $(USR_OBJ)
	@echo $(COL_LINK)
	@echo [LD]  $@ $(COL_ERR)
	@$(CC) $(LD_FLAGS) -o "$@" $(USR_OBJ) $(LIB_OBJ) $(CORE_LIB) $(LIBS)
	@echo $(COL_OK)User code built and linked to libraries &&echo.

%.lst: %.elf
	@echo [LST] $@
	@$(OBJDUMP) -d -S --demangle --no-show-raw-insn "$<" > "$@"
	@echo $(COL_OK)Sucessfully built project$(COL_RESET) &&echo.

%.sym: %.elf
	@echo [SYM] $@
	@$(NM) $(NM_FLAGS) "$<" > "$@"

%.hex: %.elf
	@echo $(COL_LINK)[HEX] $@
	@$(OBJCOPY) -O ihex -R.eeprom "$<" "$@"

# Cleaning --------------------------------------------------------------------
cleanUser:
	@echo $(COL_LINK)Cleaning user binaries...$(COL_RESET)
	@if exist $(USR_BIN) rd /s/q "$(USR_BIN)"
	@if exist "$(TARGET_LST)" del $(subst /,\,$(TARGET_LST))

cleanCore:
	@echo $(COL_LINK)Cleaning core binaries...$(COL_RESET)
	@if exist $(CORE_BIN) rd /s/q "$(CORE_BIN)"
	@if exist $(CORE_LIB) del  $(subst /,\,$(CORE_LIB))

cleanLib:
	@echo $(COL_LINK)Cleaning user library binaries...$(COL_RESET)
	@if exist $(LIB_BIN) rd /s/q "$(LIB_BIN)"

# compiler generated dependency info ------------------------------------------
-include $(CORE_OBJ:.o=.d)
-include $(USR_OBJ:.o=.d)
-include $(LIB_OBJ:.o=.d)