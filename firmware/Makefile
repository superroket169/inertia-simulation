MCU    = atmega328p
F_CPU  = 16000000UL
PORT  ?= /dev/ttyUSB0
# Clones with the old bootloader need 57600.
UPLOAD_BAUD ?= 115200

CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=c11 -Os -Wall -Wextra
LDLIBS  = -lm

SRC   = main.c motor.c
BUILD = build

all: $(BUILD)/firmware.hex

$(BUILD)/firmware.elf: $(SRC) config.h motor.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDLIBS)
	avr-size --mcu=$(MCU) $@

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD):
	mkdir -p $@

flash: $(BUILD)/firmware.hex
	avrdude -p $(MCU) -c arduino -P $(PORT) -b $(UPLOAD_BAUD) -U flash:w:$<:i

clean:
	rm -rf $(BUILD)

.PHONY: all flash clean
