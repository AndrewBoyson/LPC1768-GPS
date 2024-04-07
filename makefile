#Shared clock files
CFILES += $(wildcard ../shared/clock/*.c ../shared/clock/*/*.c ../shared/clock/*/*/*.c ../shared/clock/*/*/*/*.c ../shared/clock/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/clock/*.s ../shared/clock/*/*.s ../shared/clock/*/*/*.s ../shared/clock/*/*/*/*.s ../shared/clock/*/*/*/*/*.s)

#Shared log files
CFILES += $(wildcard ../shared/log/*.c ../shared/log/*/*.c ../shared/log/*/*/*.c ../shared/log/*/*/*/*.c ../shared/log/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/log/*.s ../shared/log/*/*.s ../shared/log/*/*/*.s ../shared/log/*/*/*/*.s ../shared/log/*/*/*/*/*.s)

#Shared lpc1768 files
CFILES += $(wildcard ../shared/lpc1768/*.c ../shared/lpc1768/*/*.c ../shared/lpc1768/*/*/*.c ../shared/lpc1768/*/*/*/*.c ../shared/lpc1768/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/lpc1768/*.s ../shared/lpc1768/*/*.s ../shared/lpc1768/*/*/*.s ../shared/lpc1768/*/*/*/*.s ../shared/lpc1768/*/*/*/*/*.s)

#Shared crypto files
CFILES += $(wildcard ../shared/crypto/*.c ../shared/crypto/*/*.c ../shared/crypto/*/*/*.c ../shared/crypto/*/*/*/*.c ../shared/crypto/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/crypto/*.s ../shared/crypto/*/*.s ../shared/crypto/*/*/*.s ../shared/crypto/*/*/*/*.s ../shared/crypto/*/*/*/*/*.s)

#Shared net files
CFILES += $(wildcard ../shared/net/*.c ../shared/net/*/*.c ../shared/net/*/*/*.c ../shared/net/*/*/*/*.c ../shared/net/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/net/*.s ../shared/net/*/*.s ../shared/net/*/*/*.s ../shared/net/*/*/*/*.s ../shared/net/*/*/*/*/*.s)

#Shared web files
CFILES += $(wildcard ../shared/web/*.c ../shared/web/*/*.c ../shared/web/*/*/*.c ../shared/web/*/*/*/*.c ../shared/web/*/*/*/*/*.c)
SFILES += $(wildcard ../shared/web/*.s ../shared/web/*/*.s ../shared/web/*/*/*.s ../shared/web/*/*/*/*.s ../shared/web/*/*/*/*/*.s)

#This files
CFILES += $(wildcard *.c */*.c */*/*.c)
SFILES += $(wildcard *.s */*.s */*/*.s)

OFILES += $(patsubst %.c,%.o,$(CFILES))
OFILES += $(patsubst %.s,%.o,$(SFILES))

PROJECT=gps

LSCRIPT=link.ld

OPTIMIZATION=2

ASFLAGS += -mcpu=cortex-m3
ASFLAGS += -mimplicit-it=thumb

GCFLAGS += -mcpu=cortex-m3
GCFLAGS += -mthumb
GCFLAGS += -Wall
GCFLAGS += -Wno-misleading-indentation
GCFLAGS += -Wno-unused-function
GCFLAGS += -Wno-unused-variable
GCFLAGS += -Werror
GCFLAGS += -I.
GCFLAGS += -I../shared 
GCFLAGS += -O$(OPTIMIZATION)

LDFLAGS += -mcpu=cortex-m3
LDFLAGS += -mthumb
LDFLAGS += -O$(OPTIMIZATION)
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,-Map=$(PROJECT).map
LDFLAGS += -T$(LSCRIPT)

GCC     = arm-none-eabi-gcc
AS      = arm-none-eabi-as
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size
DIS     = arm-none-eabi-objdump -dS
MAP     = arm-none-eabi-objdump -h
REMOVE  = rm -f
COPY    = cp -f

all: $(PROJECT).bin

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $(PROJECT).elf $(PROJECT).bin

$(PROJECT).elf: $(OFILES)
	@echo $(OFILES) > ofiles
	$(GCC) $(LDFLAGS) -Wl,@ofiles -o $(PROJECT).elf

%.o : %.c
	$(GCC) $(GCFLAGS) -c -o $@ $<

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

stats: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) $(OFILES)
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(PROJECT).elf
	$(REMOVE) $(PROJECT).map
	$(REMOVE) $(PROJECT).bin
	$(REMOVE) $(PROJECT).dis
	$(REMOVE) *.lst

dis:
	$(DIS) $(PROJECT).elf > $(PROJECT).dis
	
map:
	$(MAP) $(PROJECT).elf

copy:
	$(COPY) $(PROJECT).bin F:/
	
wd:
	@echo $(CURDIR)

sfiles:
	@echo $(SFILES)
	
cfiles:
	@echo $(CFILES)
	
subdirs:
	@echo $(SUBDIRS)
