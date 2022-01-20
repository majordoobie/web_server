TARGET=ashti
INCLUDE=../../include

CC=cc
CFLAGS+=-W -Wall -Wextra -Wpedantic --std=c11 -I$(INCLUDE)
LDLIBS+=

BIN=../../bin
OBJ=../../bin
ROOT=../..

# Collect all the lib files to link afterwards
LIB=$(wildcard $(BIN)/*.a)
OBJ_FILES += $(BIN)/c_subprocess.o
OBJ_FILES += $(BIN)/file_fetcher.o
OBJ_FILES += $(BIN)/http_handler.o
OBJ_FILES += $(BIN)/socket_factory.o

#OBJ_FILES = $(filter-out $(BIN)/server_socket.o, $(wildcard $(BIN)/*o))
#OBJ_FILES:=$(filter-out $(BIN)/main.o, $OBJ_FILES)

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, $(OBJ)/%.o, $(SRCS))


all: $(TARGET) clean_up

.PHONY: debug clean check profile

profile: CFLAGS+= -pg
profile: $(TARGET)

debug: CFLAGS+= -g
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(OBJS) $(OBJ_FILES) $(LIB) $(LDLIBS) -o $(ROOT)/$(TARGET)

$(OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean_up:
	rm -rf $(OBJS)

clean:
	rm -rf $(OBJS)
