CXXFLAGS = 	-fno-omit-frame-pointer -D _DEBUG -ggdb3 -std=c++17 -O3 					 												 \
			-fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer 		 \
			-Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual 		 \
			-Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral  \
			-Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual \
			-Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel 		 \
			-Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override 			 \
			-Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros 			 \
			-Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector 		 \
			-Wlarger-than=200000 -Wstack-usage=200000 -Werror=vla																		 \
			-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,$\
			nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,$\
			unreachable,vla-bound,vptr


NAME = test.elf

CC	 = g++ #afl compilier gcc with inserts

SRC_C_EXT 	= .cpp
OBJ     	= .
SRC_SRC 	= .
BIN     	= .

SOURCES_SRC  = $(wildcard ./$(SRC_SRC)/*$(SRC_C_EXT))
OBJFILES_SRC = $(patsubst ./$(SRC_SRC)/%,./$(OBJ)/%,$(SOURCES_SRC:$(SRC_C_EXT)=.o))

#========================================================================================

./$(BIN)/$(NAME): $(OBJFILES_SRC) 		#Linking with GCC
	$(CC) $(CXXFLAGS) $^ -o $@

./$(OBJ)/%.o: ./$(SRC_SRC)/%$(SRC_C_EXT) 	#Object with GCC
	$(CC) -Wl,--copy-dt-needed-entries $(CFLAGS) -c -o $@ $<

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