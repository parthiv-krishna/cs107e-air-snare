#
# Makefile
#

AMPIHOME = AMPi/ampi
MUSIC = hihat.o snare.o crash.o kick.o

MODULES = ampienv.o util.o audio_sequence.o synth.o LSM6DS33.o read_angle.o
MODULES += $(MUSIC)

OBJECTS = $(addprefix build/obj/, $(MODULES) start.o cstart.o)

CFLAGS_EXTRA = -Werror -Wpointer-arith
CFLAGS_BASIC = -Iinclude -I$(AMPIHOME) -I$(CS107E)/include -Wall -std=gnu99 -ffreestanding $(CFLAGS_EXTRA) -nostdlib
FLOAT_ABI ?= softfp
ARCH = -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=$(FLOAT_ABI)
CFLAGS_BASIC += $(ARCH)
DEFINE = -D__circle__ -DRASPPI=1 -DOGG # For library headers
CFLAGS_BASIC += $(DEFINE)

CFLAGS_OPTIM = $(CFLAGS_BASIC) -O3
CFLAGS = $(CFLAGS_BASIC) -Og -g -mapcs-frame -fno-omit-frame-pointer -mpoke-function-name

LDFLAGS = -nostdlib -T src/boot/memmap -L$(CS107E)/lib -L$(AMPIHOME)
LDLIBS  = -lpisd -lpiextra -lpi -lampi -lm -lgcc -lc

TARGET  = build/bin/main.bin
TARGET_DOWNLOAD = build/bin/download.bin


vpath %.c src/boot src/audio src/util src/sensing
vpath %.s src/boot

all: $(TARGET)


%.bin: build/bin/%.bin ;
%.elf: build/elf/%.elf ;
%.o: build/obj/%.o ;
%.list: build/list/%.list ;

build/bin/%.bin: build/elf/%.elf | build
	arm-none-eabi-objcopy $< -O binary $@

# Build *.elf from *.o.
build/elf/%.elf: build/obj/%.o $(OBJECTS) | build
	echo Using music $(MUSIC)
	arm-none-eabi-gcc $(LDFLAGS) $^ $(LDLIBS) -o $@

# Build *.o from *.c.
build/obj/%.o: %.c | build
	arm-none-eabi-gcc $(CFLAGS) -c $< -o $@

# Build *.o from *.s.
build/obj/%.o: %.s | build
	arm-none-eabi-as $< -o $@

build/obj/%.o: media/%.ogg | build
	xxd -i $< | arm-none-eabi-gcc $(CFLAGS) -c -o $@ -x c -

build/obj/%.o: media/%.raw | build
	xxd -i $< | echo "$$(cat -)\n#define DOWNLOAD_$(notdir $(basename $<))" | arm-none-eabi-gcc $(CFLAGS) -c -o $@ -x c -

# Build *.list from *.o.
build/list/%.list: build/obj/%.o | build
	arm-none-eabi-objdump --no-show-raw-insn -d $< > $@

build/obj/synth_ogg.o: CFLAGS += -Wno-error -O2

build:
	mkdir -p build/obj build/elf build/bin build/list

clean:
	rm -rf build

install: $(TARGET)
	rpi-install.py -p  $<

download: $(TARGET_DOWNLOAD)
	rpi-install.py -p  $<

.PHONY: all clean install

# Prevent make from removing intermediate build artifacts.
.PRECIOUS: build/bin/%.bin build/elf/%.elf build/list/%.list build/obj/%.o

# Disable all built-in rules.
# https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES: