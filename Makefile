CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror --std=c99 -fPIC
LIBCOFLAGS=--std=c99 -fPIC -DLIBCO_MP

LIB_d=lib/
OBJ_d=obj/
SRC_d=src/
BIN_d=bin/
TEST_d=test/
DIRS=$(LIB_d) $(OBJ_d) $(BIN_d)
LIB_name=roadrunner

SRCs=$(SRC_d)runner.c
OBJs=$(patsubst $(SRC_d)%.c, $(OBJ_d)%.o, $(SRCs))
LIB=$(LIB_d)lib$(LIB_name).so

LIBCO_d=deps/libco/
DEPS=$(LIB_d)libco.so

LDFLAG=-L$(LIB_d)
LDLIB=-lco
RFLAG=-Wl,-rpath,'$$ORIGIN'

TEST=$(BIN_d)test

all: dirs $(LIB)

$(OBJ_d)%.o: $(SRC_d)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJs) $(DEPS)
	$(CC) $(CFLAGS) -shared -o $@ $< $(LDFLAG) $(LDLIB) $(RFLAG) -Wl,--as-needed

$(LIB_d)libco.so: $(LIBCO_d)libco.c
	$(CC) $(LIBCOFLAGS) -shared -o $@ $<

$(TEST): $(TEST_d)main.c
	$(CC) $(CFLAGS) -o $@ $< -Llib -lco -lroadrunner -Wl,-rpath,'$$ORIGIN'/../lib -Wl,--as-needed

test: dirs $(LIB) $(TEST)

dirs: 
	@mkdir -p $(DIRS)

clean:
	@rm -rf $(OBJ_d) $(LIB_d) $(BIN_d)

.PHONY: dirs all test clean
