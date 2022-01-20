# This line gets populated as you progress
TARGET=test_exe
INCLUDE=../include

CC=cc
CFLAGS+=-Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -g -I$(INCLUDE)
LDLIBS+=-lcheck -lm -lrt -lpthread -lsubunit -lm

BIN=../bin
OBJ=../bin

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, $(OBJ)/%.o, $(SRCS))
LIB=$(wildcard $(BIN)/*.a)


all: $(TARGET) clean_up

.PHONY: debug clean check profile

profile: CFLAGS+= -pg
profile: $(TARGET)

debug: CFLAGS+= -g
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIB) $(LDLIBS) -o $(BIN)/$(TARGET)

$(OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean_up:
	rm -rf $(OBJS)

check: $(OBJS)
	$(info )
	$(info )
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIB) -lcheck -lm -lrt -lpthread -lsubunit $(LDLIBS) -o $(BIN)/$(TARGET)
	$(info )
	$(info )
	$(info ************  RUNNING TEST CASES - WITH VALGRIND ************)
	cd ..; CK_FORK=no valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all ./bin/test_exe


clean:
	rm -rf $(OBJS)
