CC = gcc

FLAGS = -o

DEBUG_FLAG = -g

all: pass1 pass2

pass1: pass1.c common.c
	${CC} ${DEBUG_FLAG} pass1.c common.c ${FLAGS} pass1.out

pass2: pass2.c common.c
	${CC} ${DEBUG_FLAG} pass2.c common.c ${FLAGS} pass2.out

test: test.c common.c
	${CC} ${DEBUG_FLAG} test.c common.c ${FLAGS} test.out