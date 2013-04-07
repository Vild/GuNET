CC := gcc
CFLAGS := -Wall -Werror -pedantic -std=c89 -ggdb -fPIC -Os
LFLAGS := -shared -lm -levent_core

SRC := src/
OBJ := obj/
BIN := bin/

TARGET := $(BIN)libGuNET.so
TARGET_SERVER_TEST := $(BIN)GuNET_Server_Test
TARGET_CLIENT_TEST := $(BIN)GuNET_Client_Test

LFLAGS_TEST := -L$(BIN) -lGuNET -Wl,-rpath=. -Wl,-rpath=lib/ -Wl,-rpath=libs/

OBJECTS = $(OBJ)GuNET_Server.o $(OBJ)GuNET_Client.o
OBJECTS_SERVER_TEST = $(OBJ)GuNET_Server_Test.o
OBJECTS_CLIENT_TEST = $(OBJ)GuNET_Client_Test.o

.PHONY: all clean $(TARGET) $(TARGET_CLIENT_TEST) $(TARGET_SERVER_TEST)

all: clean $(TARGET) $(TARGET_CLIENT_TEST) $(TARGET_SERVER_TEST)

$(OBJ)%.o: $(SRC)%.c $(SRC)%.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)%.o: $(SRC)%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN)
	$(CC) $(LFLAGS) $^ -o $@

$(TARGET_CLIENT_TEST): $(OBJECTS_CLIENT_TEST) $(TARGET)
	@mkdir -p $(BIN)
	$(CC) $(LFLAGS_TEST) $< -o $@

$(TARGET_SERVER_TEST): $(OBJECTS_SERVER_TEST) $(TARGET)
	@mkdir -p $(BIN)
	$(CC) $(LFLAGS_TEST) $< -o $@

clean:
	rm -rf obj/* bin/*