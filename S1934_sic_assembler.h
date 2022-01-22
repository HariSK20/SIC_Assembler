// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#ifndef S1934_SIC_ASSEMBLER

#define S1934_SIC_ASSEMBLER

#define MAX_OPCODE_SIZE 8;
#define MAX_OPERAND_SIZE 15;
#define OPCODE_HEX_SIZE 2;
#define OPERAND_HEX_SIZE 4;

int u_errno;

typedef struct List
{
	char symbol[7];
	int code;
	struct List* next;
	/* data */
}List;

List* insertIntoList(List *list, char *s, int c);

int searchList(List *list, char *s);

void freeList(List *list);

void printTokens(char **tokens, int no_of_tokens);

char** splitString(char *str, char delimiter, int *no_of_tokens);

#endif

/*
Opcodes
ADD		18
AND		40
COMP	28
DIV		24
J		3C
JEQ		30
JGT		34
JLT		38
JSUB	48
LDA		00
LDCH	50
LDL		08
LDX		04
MUL		20
OR		44
RD		D8
RSUB	4C
STA		0C
STCH	54
STL		14
STSW	E8
STX		10
SUB		1C
TD		E0
TIX		2C
WD		DC

Registers: 
A 0
X 1
L 2
PC 8
SW 9
*/