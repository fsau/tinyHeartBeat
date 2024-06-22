# Makefile for avr-gcc
MCU_TARGET ?= attiny13a
PGR = main
CCFLAGS = -Wall -Wextra -Os --short-enums -flto -MMD -MP
PGR_HW = usbasp #stk500v2 -D -P /dev/ttyUSB0
MODULES = 
# EFLAGS ?= 

DEP = $(wildcard ./build/*.d)
MODOF = $(addsuffix .o,$(addprefix build/,$(MODULES)))
MODCF = $(addsuffix .c, $(MODULES))
MODHF = $(addsuffix .h, $(MODULES))

TAG_COMMIT := $(shell git rev-list --abbrev-commit --all --max-count=1)
LFUSE = $(shell xxd -s 0 -l 1 -u -p ./build/main_fuses.bin)
HFUSE = $(shell xxd -s 1 -l 1 -u -p ./build/main_fuses.bin)
EFUSE = $(shell xxd -s 2 -l 1 -u -p ./build/main_fuses.bin)

.PHONY: all prog_lfuse prog_hfuse prog_full prog_verify clean tar size export

all: build/ build/$(PGR).hex build/$(PGR)_eeprom.hex build/$(PGR).lst

build/:
	mkdir -p build

build/$(PGR).hex: ./build/$(PGR).elf
	avr-objcopy -j .text -j .data -O ihex build/$(PGR).elf build/$(PGR).hex

build/$(PGR)_eeprom.hex: ./build/$(PGR).elf
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex build/$(PGR).elf build/$(PGR)_eeprom.hex

build/$(PGR).lst: ./build/$(PGR).elf
	avr-objdump -h -S ./build/$(PGR).elf > build/$(PGR).lst

build/$(PGR).elf ./build/$(PGR).map: ./build/$(PGR).o $(MODOF)
	avr-gcc -g $(CCFLAGS) $(EFLAGS) -mmcu=$(MCU_TARGET) -Wl,-Map,build/$(PGR).map -o build/$(PGR).elf build/$(PGR).o $(MODOF)

build/$(PGR).o: $(PGR).c $(MODHF)
	avr-gcc -g $(CCFLAGS) $(EFLAGS) -mmcu=$(MCU_TARGET) -c $(PGR).c -o build/$(PGR).o

build/$(PGR)_fuses.bin: ./build/$(PGR).elf
	avr-objcopy -O binary --only-section=.fuse build/main.elf build/$(PGR)_fuses.bin

$(MODOF): build/%.o: %.c %.h
	avr-gcc -g $(CCFLAGS) $(EFLAGS) -mmcu=$(MCU_TARGET) -c $< -o $@

-include $(DEP)

prog: build/$(PGR).hex
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U flash:w:'build/$(PGR).hex':a

prog_full: build/$(PGR).hex build/$(PGR)_eeprom.hex ./build/$(PGR)_fuses.bin
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -e -U flash:w:'build/$(PGR).hex':a -U eeprom:w:'build/$(PGR)_eeprom.hex':a -U hfuse:w:0x$(HFUSE):m -U lfuse:w:0x$(LFUSE):m

prog_eeprom: ./build/$(PGR)_eeprom.hex
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U eeprom:w:'build/$(PGR)_eeprom.hex':a

prog_lfuse: ./build/$(PGR)_fuses.bin
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U lfuse:w:0x$(LFUSE):m

prog_hfuse: ./build/$(PGR)_fuses.bin
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U hfuse:w:0x$(HFUSE):m

prog_efuse: ./build/$(PGR)_fuses.bin
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U efuse:w:0x$(EFUSE):m

prog_verify: ./build/$(PGR)_fuses.bin ./build/$(PGR).hex
	avrdude -c $(PGR_HW) -p $(MCU_TARGET) -U flash:v:'build/$(PGR).hex':a -U eeprom:v:'build/$(PGR)_eeprom.hex':a -U hfuse:v:0x$(HFUSE):m -U lfuse:v:0x$(LFUSE):m

export: build/ build/$(PGR).hex build/$(PGR)_eeprom.hex ./build/$(PGR)_fuses.bin
	mkdir -p ./hex-out/
	cp ./build/$(PGR).hex ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).hex
	cp ./build/$(PGR)_eeprom.hex ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT)_eeprom.hex
	./makebat.sh "$(PGR_HW)" $(MCU_TARGET) $(MCU_TARGET)-$(TAG_COMMIT) $(HFUSE) $(LFUSE) $(EFUSE) > ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).bat
	unix2dos ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).bat
	rm -f "./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).zip"
	7z a ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).zip ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).bat ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).hex ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT)_eeprom.hex
	rm ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).bat ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT).hex ./hex-out/$(MCU_TARGET)-$(TAG_COMMIT)_eeprom.hex

clean:
	rm -r ./build/
	# find . -maxdepth 2 -regextype awk -regex ".*\.(lst|hex|o|d|map|elf|tar.gz|zip|bin)" -delete

tar: build/$(PGR)$(TAG_COMMIT).hex Makefile
	tar --create --file $(PGR).tar.gz --gzip Makefile \
	$$(find -maxdepth 2 -regextype awk -regex ".*\.(c|h|hex)" -printf "%P ")

zip: build/$(PGR)$(TAG_COMMIT).hex Makefile
	7z a $(PGR).zip Makefile \
	$$(find -maxdepth 2 -regextype awk -regex ".*\.(c|h|hex)" -printf "%P ")

size: build/$(PGR).elf
	avr-size --mcu=$(MCU_TARGET) -B ./build/main.hex
	avr-size --mcu=$(MCU_TARGET) -A ./build/main.elf
	avr-size --mcu=$(MCU_TARGET) -C ./build/main.elf