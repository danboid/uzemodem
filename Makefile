## Programs and Paths
KERNEL_DIR = ../../kernel
KERNEL_OPTIONS = -DVIDEO_MODE=80 -DVIDEO_MODE_PATH=\"customVideoMode80\"
KERNEL_OPTIONS += -DSCREEN_TILES_H=80 -DSCREEN_TILES_V=25 -DFIRST_RENDER_LINE=28 -DFONT_TILE_INDEX=0
KERNEL_OPTIONS += -DMIXER_WAVES=\"$(KERNEL_DIR)/data/sounds.inc\"  -DSOUND_MIXER=1  -DSOUND_CHANNEL_5_ENABLE=0 -DENABLE_MIXER=0
KERNEL_OPTIONS += -DUART_RX_BUFFER_SIZE=256 -DUART_TX_BUFFER_SIZE=128 -DUART=1
KERNEL_OPTIONS += -DPF_USE_DIR=0 -DPF_USE_LSEEK=1 -DPF_USE_WRITE=1 -DPF_USE_READ=1
PFF_DIR = $(KERNEL_DIR)/petitfatfs

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
NM = avr-nm
PACKER = packrom
INFO=gameinfo.properties

## General Flags
MCU = atmega644
TARGET = Uzemodem
CFLAGS = -mmcu=atmega644 -DF_CPU=28636360UL -Wall -Os -mcall-prologues -fno-exceptions -ffunction-sections -std=gnu99 $(KERNEL_OPTIONS) -I$(KERNEL_DIR)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Project Source Files
OBJFILES = zheaders.o znumbers.o zserial.o crc16.o crc32.o rz.o pff.o diskio.o uzeboxVideoEngineCore.o  uzeboxCore.o uzeboxSoundEngine.o uzeboxSoundEngineCore.o uzeboxVideoEngine.o

INCLUDES = -I$(KERNEL_DIR) -I$(PFF_DIR) -I./include -I./customVideoMode80

## Build Rules
all: $(TARGET).uze

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pff.o: $(PFF_DIR)/pff.c
	$(CC) $(CFLAGS) -c -o $@ $<

diskio.o: $(PFF_DIR)/diskio.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).elf: $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(TARGET).elf $(LDFLAGS)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).uze: $(TARGET).hex
	$(PACKER) $(TARGET).hex $(TARGET).uze $(INFO)

size: $(TARGET).elf
	$(SIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm -rf *.o $(TARGET).elf $(TARGET).hex $(TARGET).uze
