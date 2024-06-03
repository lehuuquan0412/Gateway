.PHONY : all clean

PRJ_NAME := gateway

LIB_NAME := template

CUR_DIR := .
INC_DIR := $(CUR_DIR)/inc
SRC_DIR := $(CUR_DIR)/src
OBJ_DIR := $(CUR_DIR)/objs
BIN_DIR := $(CUR_DIR)/bin

LIB_DIR := $(CUR_DIR)/lib

CC := gcc

C_FLAGS := -c -fPIC -o
ADD_FLAG := -lpthread -lsqlite3

#create objs file 

obj:
	$(CC) $(C_FLAGS) $(OBJ_DIR)/main.o $(CUR_DIR)/main.c -I $(INC_DIR)
	$(CC) $(C_FLAGS) $(OBJ_DIR)/functions.o $(SRC_DIR)/functions.c -I $(INC_DIR)
	$(CC) $(C_FLAGS) $(OBJ_DIR)/linked_list.o $(SRC_DIR)/linked_list.c -I $(INC_DIR)
	$(CC) $(C_FLAGS) $(OBJ_DIR)/database.o $(SRC_DIR)/database.c -I $(INC_DIR)
	$(CC) $(C_FLAGS) $(OBJ_DIR)/ringbuffer.o $(SRC_DIR)/ringbuffer.c -I $(INC_DIR)
static:
	ar rcs $(LIB_DIR)/lib$(LIB_NAME).a $(OBJ_DIR)/*.o

all: clean obj static
	$(CC) $(OBJ_DIR)/main.o -L$(LIB_DIR) -l$(LIB_NAME) -o $(BIN_DIR)/$(PRJ_NAME) $(ADD_FLAG)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_DIR)/*
	rm -rf $(BIN_DIR)/*
	rm -rf gateway.log gateway.db
debug: all
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./bin/gateway 5000
