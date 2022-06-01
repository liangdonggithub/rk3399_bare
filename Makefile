CROSS=aarch64-linux-gnu
CC=$(CROSS)-gcc
PROJ=rk3399_bare
ELF=$(PROJ).elf
BIN=$(PROJ).bin
MAP=$(PROJ).map
LDS=$(PROJ).lds
ELF_LDR=$(PROJ)_ldr.elf
BIN_LDR=$(PROJ)_ldr.bin
LDS_LDR=loader.lds
MERGER_LDR=./tools/boot_merger
MERGER_LDR_INI=./tools/RK3399MINIALL.ini

ALL_S=$(wildcard src/*.S)
ALL_S_OBJ=$(patsubst %.S,%.o,$(ALL_S))
ALL_C=$(wildcard src/*.c)
ALL_C_OBJ=$(patsubst %.c,%.o,$(ALL_C))

ALL_OBJ= $(ALL_S_OBJ) $(ALL_C_OBJ)

ASMOPS= -g -Iinclude -nostdlib -fno-builtin
COPS= -g -Iinclude -nostdlib -fno-builtin

PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

all : $(BIN)

$(BIN): $(LDS) $(ALL_OBJ)
	$(CROSS)-ld -T $(LDS) -Map $(MAP) -o $(ELF) $(ALL_OBJ) $(PLATFORM_LIBS)
	$(CROSS)-objcopy $(ELF) -O binary $(BIN)

%.o: %.S 
	$(CROSS)-gcc $(ASMOPS) -MMD -c $<  -o $@

%.o: %.c 
	$(CROSS)-gcc $(COPS) -MMD -c $< -o $@

clean:
	rm ./src/*.o ./src_loader/*.o ./src/*.d ./src_loader/*.d *.map *.elf *.bin

loader: $(BIN_LDR) $(LDS_LDR)
	$(MERGER_LDR) $(MERGER_LDR_INI)

$(BIN_LDR): $(LDS_LDR) src_loader/loader.o
	$(CROSS)-ld -T $(LDS_LDR) -o $(ELF_LDR) src_loader/loader.o
	$(CROSS)-objcopy $(ELF_LDR) -O binary $(BIN_LDR)


.PHONY: clean loader
