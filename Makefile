CC = gcc

FLAGS = -o

DEBUG_FLAG = -g

all: pass1 pass2 absolute relocating

pass1: pass1.c common.c
	${CC} ${DEBUG_FLAG} pass1.c common.c ${FLAGS} pass1.out

pass2: pass2.c common.c
	${CC} ${DEBUG_FLAG} pass2.c common.c ${FLAGS} pass2.out

absolute: loader_absolute.c common.c 
	${CC} ${DEBUG_FLAG} loader_absolute.c common.c ${FLAGS} loader_absolute.out

relocating: loader_relocating.c common.c 
	${CC} ${DEBUG_FLAG} loader_relocating.c common.c ${FLAGS} loader_relocating.out

macro_processor: macro_proc.c common.c 
	${CC} ${DEBUG_FLAG} macro_proc.c common.c -o macro_processor.out

test: test.c common.c
	${CC} ${DEBUG_FLAG} test.c common.c ${FLAGS} test.out

clean:
	rm *.out