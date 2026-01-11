# Name of the output program
TARGET  = esw
# Name of the build directory
BUILD   = build
# Base name of the toolchain
TC      = riscv32-MINIRISC-elf
CC      = $(TC)-gcc
LD      = $(TC)-gcc
SIZE    = $(TC)-size
OBJCOPY = $(TC)-objcopy
OBJDUMP = $(TC)-objdump

# RV32, no compressed via .option norvc in source
CFLAGS  += -march=rv32im_zicsr
CFLAGS  += -mabi=ilp32
CFLAGS  += -mno-relax
CFLAGS  += -W -Wall
CFLAGS  += -O2

LDFLAGS += -nostartfiles
LDFLAGS += -Wl,-Ttext=0x80000000

# Accept both .S and .s
SRCS   := $(wildcard *.S) $(wildcard *.s)
OBJS    = $(addprefix $(BUILD)/, $(SRCS:.S=.o))
OBJS    := $(OBJS:.s=.o)
DEPS    = $(OBJS:.o=.d)

.PHONY: all clean lss

all: $(BUILD)/$(TARGET).bin

# Ensure build/ exists
$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/$(TARGET).elf: | $(BUILD)
$(BUILD)/$(TARGET).bin: | $(BUILD)
$(BUILD)/$(TARGET).lss: | $(BUILD)

-include $(DEPS)

$(BUILD)/%.o: %.S | $(BUILD)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@ -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/%.o: %.s | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(LD) -o $@ $(filter %.o,$^) $(CFLAGS) $(LDFLAGS)
	@echo "────────────────────────────────────────────────────────────────────────"
	@$(SIZE) $@
	@echo "────────────────────────────────────────────────────────────────────────"

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).lss: $(BUILD)/$(TARGET).elf
	$(OBJDUMP) -h -D $< > $@

lss: $(BUILD)/$(TARGET).lss
	less $<

clean:
	@rm -rf $(BUILD)
