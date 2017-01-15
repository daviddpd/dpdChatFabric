# Makefile for ESP8266 projects using rBoot and FOTA updates
# - 2016 - using for my own projects -dpd
#
# Based on a Makefile by 
# - zarya
# - Jeroen Domburg (Sprite_tm)
# - Christian Klippel (mamalala)
# - Tommie Gannert (tommie)
#
# And merged with Makefile from rBoot sample project 
# by raburton


# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware
FIRMW_DIR	= firmware

FW_SECTS      = .text .data .rodata
FW_USER_ARGS  = -quiet -bin -boot2


# base directory for the compiler
#XTENSA_TOOLS_ROOT ?= /home/vagrant/esp-open-sdk/xtensa-lx106-elf/bin
XTENSA_TOOLS_ROOT ?= /Volumes/esp8266/esp-open-sdk/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= /Volumes/esp8266/esp-open-sdk/sdk

# esptool.py path and port
ESPTOOL		?= /usr/local/bin/esptool.py
ESPTOOL2_DIR	?= /Users/dpd/Documents/github/esptool2
ESPTOOL2	?= /Users/dpd/Documents/github/esptool2/esptool2
ESPPORT		?= /dev/ttyUSB0

# rboot

RBOOT_DIR	?= /Users/dpd/Documents/github/rboot
RBOOT_BIN	?= /Users/dpd/Documents/github/rboot/firmware/rboot.bin

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= ../dpdChatFabric ../dpdChatPacket driver user espChatFabric ../crypto ../util ../uuid_wrapper ../uuid
EXTRA_INCDIR    = include ../crypto ../includes 

# libraries used in this project, mainly provided by the SDK
#LIBS		= c gcc hal pp phy net80211 lwip wpa main
LIBS		= cirom gcc hal pp phy net80211 lwip wpa main pwm

# compiler flags using during compilation of source files -fno-inline-functions
CFLAGS		= -Os -g -Wpointer-arith -Wundef -Werror -Wno-implicit -Wl,-EL  -nostdlib -mlongcalls -mtext-section-literals -fno-inline-functions -D__ets__ -DICACHE_FLASH -DHAVE_LOCAL_CRYPTO -DESP8266 -D_LITTLE_ENDIAN

# ##### chatFabric Specific Options  ##############################
#
ifdef ESP_DEVICE_OUTLET
CFLAGS	+= -DESP_DEVICE_OUTLET
endif

VERSION_DATE='"$(shell date -u "+%Y%m%d_%H%M%S%Z")"'
VERSION_GIT='"$(shell git rev-parse HEAD)"'
CFLAGS	+= -DVERSION_DATE=$(VERSION_DATE)
CFLAGS	+= -DVERSION_GIT=$(VERSION_GIT)

#
# ##################################################################


# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static 

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc


####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
LD_SCRIPT	:= -T$(LD_SCRIPT)

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1_ADDR).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2_ADDR).bin)

FW_DATA_DEFAULT := $(SDK_BASE)/bin/esp_init_data_default.bin
FW_BLANK_DATA	:= 16kblank.bin

FLASH_SIZE := 32m  #    [--flash_size {4m,2m,8m,16m,32m,16m-c1,32m-c1,32m-c2}]

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @echo
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@
endef

.PHONY: all checkdirs flash clean

all:  $(RBOOT_BIN) checkdirs $(FIRMW_DIR)/rom0.bin $(FIRMW_DIR)/rom1.bin

$(RBOOT_BIN):
	make -C $(RBOOT_DIR) RBOOT_DELAY_MICROS=2000000 RBOOT_RTC_ENABLED=1 SPI_SIZE=1M

16kblank.bin:
	dd if=/dev/zero of=16kblank.bin bs=4k count=4


# $(FW_BASE)/%.bin: $(TARGET_OUT) | $(FW_BASE)
# 	$(vecho) "FW $(FW_BASE)"
# 	$(Q) $(ESPTOOL) elf2image --flash_size $(FLASH_SIZE) -o $(FW_BASE)/ $(TARGET_OUT)
# 
# $(TARGET_OUT): $(APP_AR)
# 	$(vecho) "LD $@"
# 	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
# 
# $(APP_AR): $(OBJ)
# 	$(vecho) "AR $@"
# 	$(Q) $(AR) cru $@ $^

%.elf: $(OBJ)
	@echo "LD $(notdir $@)"
	$(LD) -L$(SDK_LIBDIR) -T$(notdir $(basename $@)).ld $(LDFLAGS) -Wl,--start-group $(LIBS) $^ -Wl,--end-group -o $@

$(FIRMW_DIR)/%.bin: %.elf
	@echo "FW $(notdir $@)"
	$(ESPTOOL2) $(FW_USER_ARGS) $^ $@ $(FW_SECTS)


checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@

chipinfo:
	@echo "Flash ID"
	$(ESPTOOL) --port $(ESPPORT) flash_id
	@echo "Chip ID"	
	$(ESPTOOL) --port $(ESPPORT) chip_id
	@echo "MAC"	
	$(ESPTOOL) --port $(ESPPORT) read_mac
	


flash: $(FW_FILE_1) $(FW_FILE_2)
	$(ESPTOOL) --port $(ESPPORT) write_flash  --flash_size $(FLASH_SIZE) \
		$(FW_FILE_1_ADDR) $(FW_FILE_1)  \
		$(FW_FILE_2_ADDR) $(FW_FILE_2)  \
		$(FW_SYSPARM_SEC0) $(FW_DATA_DEFAULT)

flashdata: $(FW_FILE_1) $(FW_FILE_2) 16kblank.bin
	$(ESPTOOL) --port $(ESPPORT) write_flash  --flash_size $(FLASH_SIZE) \
		$(FW_FILE_1_ADDR) $(FW_FILE_1) \
		$(FW_FILE_2_ADDR) $(FW_FILE_2) \
		$(FW_SYSPARM1M_SEC0) $(FW_BLANK_DATA) \
		$(FW_SYSPARM1M_SEC0) $(FW_DATA_DEFAULT) \
		$(FW_SYSPARM_SEC0) $(FW_BLANK_DATA) \
		$(FW_SYSPARM_SEC0) $(FW_DATA_DEFAULT)
		
	

clean:
	$(Q) rm -rf $(FW_BASE) $(BUILD_BASE) ./crypto ./dpdChatFabric  ./dpdChatPacket ./util ./uuid_wrapper ./uuid 

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
