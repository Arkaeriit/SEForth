# Flags
CFLAGS ?= -Wall -Wextra -g -Werror -Wno-error=cpp

# Files lists
C_SRC := dictionary.c forth_state.c C_func.c parser.c public_api.c sef_io.c
FRT_SRC := core_forth_words.frt file_forth_func.frt string_forth_words.frt tools_forth_words.frt arg_and_exit_code_forth_words.frt shell.frt
C_HEADER := sef_io.h SEForth.h C_func.h dictionary.h errors.h forth_state.h hash.h parser.h user_words.h sef_debug.h private_api.h
TARGET := seforth
C_AUTO_SRC := $(FRT_SRC:%.frt=%.c)
C_SRC += $(C_AUTO_SRC)
C_OBJS := $(C_SRC:%.c=%.o)

EXEC_SCR := main.c
EXEC_OBJS := $(EXEC_SCR:%.c=%.o)

# Install targets
PREFIX := /usr/local
TARGET_DIR_BIN := $(PREFIX)/bin
TARGET_DIR_LIB := $(PREFIX)/lib
TARGET_DIR_INCLUDE := $(PREFIX)/include
TARGET_BIN := $(TARGET_DIR_BIN)/$(TARGET)

# Commands
HASGCC := $(shell command -v $(CROSS_COMPILE)gcc 2> /dev/null)
ifdef HASGCC
	CC := $(CROSS_COMPILE)gcc
else
	HASCLANG := $(shell command -v $(CROSS_COMPILE)clang 2> /dev/null)
	ifdef HASCLANG
		CC := $(CROSS_COMPILE)clang
	else
		CC := $(CROSS_COMPILE)cc
	endif
endif
AR := $(CROSS_COMPILE)ar
CP := cp -f
RM := rm -rf

all : $(TARGET).bin
#all : $(C_OBJS)

$(C_OBJS) $(EXEC_OBJS) : SEForth.h

%.o : %.c $(C_HEADER)
	$(CC) -c $< $(CFLAGS) -o $@

%.c : %.frt
	name=$$(echo $< | sed s:.frt*::); \
		 echo "const char* $$name = " > $@
	cat $< | sed 's:[^.]( [^)]*): :g; s:^(.*)$$::; s:\s\+\([^"]\): \1:g; s:\\ .*::;  s:\\:\\\\:g; s:":\\":g; s:^:":; s:$$:\\n":;' | grep -v '^" \?\\n"'  >> $@
	echo ';' >> $@

SEForth_template.h.o: SEForth_template.h public_api.h sef_config.h
	gcc -o $@ -E $< $(CFLAGS) -Wno-everything -w

.SECONDARY: SEForth.h
SEForth.h: SEForth_template.h.o
	cat $< | sed 's:# .*::; s:£:#:g; s:___::g; s:>>://:' | uniq > $@

$(TARGET).bin : $(EXEC_OBJS) lib$(TARGET).a
	$(CC) $(EXEC_OBJS) -L. -l$(TARGET) $(CFLAGS) -o $@

lib$(TARGET).a : $(C_OBJS)
	$(AR) -rcs $@ $^

install : $(TARGET).bin lib$(TARGET).a SEForth.h
	mkdir -p $(TARGET_DIR_BIN)
	$(CP) $(TARGET).bin $(TARGET_BIN)
	$(CP) lib$(TARGET).a $(TARGET_DIR_LIB)
	$(CP) SEForth.h $(TARGET_DIR_INCLUDE)

uninstall :
	$(RM) $(TARGET_BIN)

clean : 
	$(RM) *.bin
	$(RM) *.bin.*
	$(RM) *.a
	$(RM) $(C_OBJS)
	$(RM) $(EXEC_OBJS)
	$(RM) $(C_AUTO_SRC)
	$(RM) test.txt
	$(RM) SEForth.h
	$(RM) *_template.h.o

test : $(TARGET).bin
	cd ./non-regression-tests && \
		./run-test.sh && \
		rm -f test.txt

