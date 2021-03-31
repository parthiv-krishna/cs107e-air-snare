#
# Makefile
#

AMPIHOME = ../ampi

OBJS	= main.o env.o stub.o sysinit.o util.o

LIBS	= $(AMPIHOME)/libampi.a $(CS107E)/lib/libpi.a

TARGET  = kernel

PREFIX	?= arm-none-eabi-
CC	= $(PREFIX)gcc
FLOAT_ABI ?= softfp
ARCH	?= -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=$(FLOAT_ABI)
LIBM	= $(shell $(CC) $(ARCH) -print-file-name=libm.a)
EXTRALIBS += $(LIBM)

LIBGCC	= $(shell $(CC) $(ARCH) -print-file-name=libgcc.a)
EXTRALIBS += $(LIBGCC)

ifeq ($(strip $(OGG)),1)
LIBC	= $(shell $(CC) $(ARCH) -print-file-name=libc.a)
EXTRALIBS += $(LIBC)
DEFINE	+= -DOGG
OBJS	+= synth_ogg.o music.o
else
OBJS	+= synth.o
endif

all: $(TARGET).img

$(TARGET).img: $(OBJS) $(LIBS) link.ld
	@echo "  LD    $(TARGET).elf"
	@$(LD) -o $(TARGET).elf $(LDFLAGS) \
		-T link.ld $(CRTBEGIN) $(OBJS) \
		--start-group $(LIBS) $(EXTRALIBS) --end-group $(CRTEND)
	@echo "  COPY  $(TARGET).img"
	@$(PREFIX)objcopy $(TARGET).elf -O binary $(TARGET).img
	@echo -n "  WC    $(TARGET).img => "
	@wc -c < $(TARGET).img

clean:
	rm -f *.o $(TARGET).elf $(TARGET).img

include $(AMPIHOME)/Rules.mk

music.o: music.ogg
	@echo "  OGG   music.ogg"
	@xxd -i music.ogg | $(CC) $(CFLAGS) -std=gnu99 -c -o music.o -x c -

install: $(TARGET).img
	rpi-install.py -p  $<

.PHONY: all clean install