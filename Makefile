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

ASMOPS= -Iinclude -fPIC
COPS= -g -O0 -Iinclude -nostdlib -fno-builtin -fPIC
LDOPS= -pie

all : $(BIN)

$(BIN): $(LDS) $(ALL_OBJ)
	$(CROSS)-ld $(LDOPS) -T $(LDS) -Map $(MAP) -o $(ELF) $(ALL_OBJ)
	$(CROSS)-objcopy $(ELF) -O binary $(BIN)

%.o: %.S 
	$(CROSS)-gcc $(ASMOPS) -c $<  -o $@

%.o: %.c 
	$(CROSS)-gcc $(COPS) -c $< -o $@

clean:
	rm -f ./src/*.o ./src_loader/*.o ./src/*.d ./src_loader/*.d *.map *.elf *.bin

loader: $(BIN_LDR) $(LDS_LDR)
	$(MERGER_LDR) $(MERGER_LDR_INI)

$(BIN_LDR): $(LDS_LDR) src_loader/loader.o
	$(CROSS)-ld -T $(LDS_LDR) -o $(ELF_LDR) src_loader/loader.o
	$(CROSS)-objcopy $(ELF_LDR) -O binary $(BIN_LDR)


.PHONY: clean loader
