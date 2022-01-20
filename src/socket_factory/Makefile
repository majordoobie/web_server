# Name of the lib you are creating for linking later
TARGET=socket_factory.a  # Name of the library

INCLUDE=../../include

CC=cc
CFLAGS+=-Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -I$(INCLUDE)
CFLAGS+=--std=c11

# Set the target directory, all obj's will go to bin so that each section can link if they need to
BIN=../../bin

# greps through the CWD for any files ending in .c
# Then match those files with files in $(BIN) with the extension of .o not .c
# This is how we'll create the rules; i.e. $(BIN)/prog.o: prog.c
# You can check them out with:
# $(info $$SRCS is [${SRCS}])
# $(info $$OBJS is [${OBJS}])

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, $(BIN)/%.o, $(SRCS))

# Target rule for call target
all: $(TARGET)

# This allows for a section to be missing like "check" only belongs in /test by
# setting it in .PHONY we don't need to edit the main Makefile
.PHONY: debug clean check profile

# Compiles with -pg
profile: CFLAGS+= -pg
profile: $(TARGET)

# Compiles with -g
debug: CFLAGS+= -g
debug: $(TARGET)

# Links all the OBJ files. Since there is no OBJ it will look for a rule
# for an obj. For example if obj is ../../bin/arg_parser.o it will match on
# $(BIN)/%.o and match it with a file with the same name in the CWD with the
# extension of .c
$(TARGET): $(OBJS)
	$(info $(OBJS))
	ar rcs $(BIN)/$(TARGET) $(OBJS)
	ranlib $(BIN)/$(TARGET)

# This is where it matches the OBJ to the source file
$(BIN)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# This is called when `make clean` is called from the master
clean:
	rm -rf $(OBJS)
	rm -rf $(BIN)/$(TARGET)
