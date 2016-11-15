SRC      = src/
LIB      = lib/
BUILD    = build/
OUT      = autodoor
VERSION  = 0.1

# TODO - build number

CFLAGS   = -Wall -pedantic #-ffunction-sections -fdata-sections#-fstack-protector-all -Wall -g
LFLAGS   = -lm #-lncurses

# Slow motion
SLOWMOT  = #sleep 0.2

# PLATFORM SPECIFIC ATmega2560
#PS_TTY   = /dev/ttyUSB0
PS_TTY   = usb
PS_BR    = 115200
#PS_BR    = 57600
#PS_BR    = 9600
#PS_CPU   = atmega328p
PS_CPU   = atmega2560
PS_FCPU  = 16000000L
PS_CC    = avr-gcc
PS_LD    = avr-gcc
PS_CF    = -mmcu=${PS_CPU} -DF_CPU=${PS_FCPU} -ffunction-sections -fdata-sections -Os -Wall
PS_LF    = -mmcu=${PS_CPU} -Wl,-Os -Wl,--gc-sections -Wl,-u,vfprintf -lprintf_flt -lm

PS_BUILDS = ${OUT} ${OUT}.elf ${OUT}.hex ${OUT}.eeprom.hex
PS_SIZE   = 'avr-size'
#PS_MFLASH = 30720
#PS_MSRAM  = 2048
#PS_MFLASH = 253952
PS_MFLASH = 262144
PS_MSRAM  = 8192
PS_MEEP   = 4096

define PS_RUN =
    sudo avrdude \
        -C '/etc/avrdude.conf'\
        -p ${PS_CPU}\
        -P ${PS_TTY}\
        -c 'avrispmkII'\
        -b ${PS_BR}\
        -e\
        -U 'flash:w:$(1).hex:i'
endef
define PS_EEPROM =
    sudo avrdude \
        -C '/etc/avrdude.conf'\
        -p ${PS_CPU}\
        -P ${PS_TTY}\
        -c 'avrispmkII'\
        -D\
        -U eeprom:w:$(1).eeprom.hex
endef
#        -c 'arduino'\
#        -b ${PS_BR}\
# ENDOF PLATFORM SPECIFIC



# prepare and select files
BUILD   := ${BUILD}${PS_CPU}/
BUILDOBJ = ${BUILD}obj/
OBJLSC   = $(patsubst %.c,%.o,$(subst $(SRC),,$(shell find ${SRC} -type f -name '*.c')))
OBJALL   = ${OBJLSC} ${OBJLSCPP}

COMPC    = ${PS_CC} ${PS_CF}
LINK     = ${PS_CC} ${PS_LF}

# colours
CGREEN   = "\\e[32m"
CYELLOW  = "\\e[33m"
CCYAN    = "\\e[36m"
CLCYAN   = "\\e[96m"
CBOLD    = "\\e[1m"
CNONE    = "\\e[0m"
CVOID    = "                        "

# default build
all: ${PS_BUILDS}
	@printf "${CLCYAN}${CBOLD}Done${CNONE}${CLCYAN} for ${PS_CPU}${CNONE}\n"

# compile every single source
${BUILDOBJ}%.o: ${SRC}%.c
	@printf "${CGREEN}${CBOLD}Compiling $<...${CNONE}\r"
	@mkdir -p "$(@D)"
	@${PS_CC} ${PS_CF} -DCPUARCH=${PS_CPU} -DVERSION=${VERSION} -o $@ -c $<
	@${SLOWMOT}
	@printf "\r${CGREEN}Compiled $<     ${CNONE}\n"

# link all stuff
${OUT}: $(addprefix ${BUILDOBJ}, $(OBJALL))
	@printf "${CYELLOW}${CBOLD}Linking...${CNONE}\r"
	@${PS_LD} ${PS_LF} -DCPUARCH=${PS_CPU} -o ${BUILD}$@ $^
	@${SLOWMOT}
	@printf "\r${CYELLOW}Linked    ${CNONE}\n"
	@printf ${CCYAN}
	@./tootils/freespace ${BUILD}$@ ${PS_MFLASH} ${PS_MSRAM} ${PS_MEEP} ${PS_SIZE}
	@printf ${CNONE}

# call it a .elf file
${OUT}.elf: ${OUT}
	@rm -f ${BUILD}${OUT}.elf
	@mv ${BUILD}${OUT} ${BUILD}${OUT}.elf

# build the .hex file
${OUT}.hex: ${OUT}.elf
	@avr-objcopy -O ihex -R .eeprom ${BUILD}${OUT}.elf ${BUILD}${OUT}.hex


#build the .eeprom.hex file
${OUT}.eeprom.hex: ${OUT}.elf
	@#avr-objcopy -j .eeprom -O ihex ${BUILD}${OUT}.elf ${BUILD}${OUT}.eeprom.hex
	@avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex ${BUILD}${OUT}.elf ${BUILD}${OUT}.eeprom.hex

# clean a build
clean:
	@rm -rf ${BUILD}

# run this build
run: all
	@$(call PS_RUN,${BUILD}${OUT})

# run this build
eeprom: ${OUT}.eeprom.hex
	@$(call PS_EEPROM,${BUILD}${OUT})

# the freshes
fresh: clean all
freshrun: clean all run


