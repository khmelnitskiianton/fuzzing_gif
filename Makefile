CXXFLAGS = 	-fno-omit-frame-pointer -D _DEBUG -ggdb3 -O3 \
			-fcheck-new -fstack-protector -fstrict-overflow \
			-flto-odr-type-merging -fno-omit-frame-pointer \
			-Wlarger-than=200000 -Wstack-usage=200000 -Werror=vla \
			-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,$\
			float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,$\
			null,object-size,return,returns-nonnull-attribute,shift,$\
			signed-integer-overflow,undefined,unreachable,vla-bound,vptr

NAME = test.elf

CC	 = gcc

SRC_C_EXT 	= .c
OBJ     	= .
SRC_SRC 	= .
BIN     	= .

SOURCES_SRC  = $(wildcard ./$(SRC_SRC)/*$(SRC_C_EXT))
OBJFILES_SRC = $(patsubst ./$(SRC_SRC)/%,./$(OBJ)/%,$(SOURCES_SRC:$(SRC_C_EXT)=.o))

#========================================================================================

./$(BIN)/$(NAME): $(OBJFILES_SRC) 		#Linking with GCC
	$(CC) $(CXXFLAGS) $^ -o $@

./$(OBJ)/%.o: ./$(SRC_SRC)/%$(SRC_C_EXT) 	#Object with GCC
	$(CC)  $(CXXFLAGS) -c -o $@ $<

#========================================================================================

.PHONY: mutation
mutation: $(SOURCES_SRC)
	$(CC) -shared -Wall -O3 -I/home/anton/AFLplusplus/include -I. $^ -o gif_mutation.so

.PHONY: all
all: 
	./$(BIN)/$(NAME)

.PHONY: init
init:
	mkdir -p ./$(OBJ)
	mkdir -p ./$(BIN)
	mkdir -p ./$(SRC_SRC)

.PHONY: run
run:
	./$(BIN)/$(NAME)

.PHONY: clean
clean:
	rm -f ./$(OBJ)/*.o ./$(OBJ)/*.lst