CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror --std=c99 -fPIC

AR=ar
LIBCOFLAGS=--std=c99 -fPIC -DLIBCO_MP

LIB_d=lib/
OBJ_d=obj/
SRC_d=src/
BIN_d=bin/
TEST_d=test/
DIRS=$(LIB_d) $(OBJ_d) $(BIN_d)
LIB_name=road

SRCs=$(SRC_d)runner.c
OBJs=$(patsubst $(SRC_d)%.c, $(OBJ_d)%.o, $(SRCs))
LIB=$(LIB_d)lib$(LIB_name).so

LIBCO_d=deps/libco/
DEPS=$(LIB_d)libco.a

LDFLAG=-L$(LIB_d) -l$(LIB_name)
RFLAG=-Wl,-rpath,'$$ORIGIN'/../lib

TEST=$(BIN_d)test

all: dirs $(LIB)

$(OBJ_d)%.o: $(SRC_d)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJs) $(DEPS)
	$(CC) $(CFLAGS) -shared -o $@ $^ 

# compile only libco
$(LIB_d)libco.a: $(LIBCO_d)libco.c
	$(CC) $(LIBCOFLAGS) -c -o $(OBJ_d)libco.o $<
	$(AR) rcs $@ $(OBJ_d)libco.o
	@rm $(OBJ_d)libco.o

$(TEST): $(TEST_d)main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAG) $(RFLAG) 

test: dirs $(LIB) $(TEST)

dirs: 
	@mkdir -p $(DIRS)

clean:
	@rm -rf $(OBJ_d) $(LIB_d) $(BIN_d)

.PHONY: dirs all test clean
