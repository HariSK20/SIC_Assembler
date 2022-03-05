// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#ifndef S1934_SIC_ASSEMBLER

#define S1934_SIC_ASSEMBLER 1

#define MAX_LABEL_SIZE 5
#define MAX_ADDRESS_SIZE 5
#define MAX_COLUMN_SPACE_SEPARATION 2
#define NO_OF_OPCODES 26
#define OPCODE_HEX_SIZE 2
#define OPERAND_HEX_SIZE 4
#define INSTRUCTION_SIZE_BYTES 3
#define TEXT_RECORD_COUNT 5
#define DESCRETE_REALLOC_SIZE 5
int u_errno;

static char symbol_file_name[] = "symbols.txt";
static char intermediate_file_name[] = "intermediate.txt";
static char output_file_name[] = "output.txt";
static char absolute_loaded_mem_file_name[] = "absolute_loaded_mem.txt";
static char relocated_loaded_mem_file_name[] = "relocated_loaded_mem.txt";
static char macro_processed_file_name[] = "macro_processed_file.txt";


typedef struct List
{
	char symbol[7];
	int code;
	struct List* next;
	/* data */
}List;

List *op_tab;

int min(int a, int b);

List* insertIntoList(List *list, char *s, int c);

List* loadSymTab(List *symtab, char *filename);

int searchList(List *list, char *s);

void freeList(List *list);

void freeTokens(char **tokens, int no_of_tokens);

void printTokens(char **tokens, int no_of_tokens);

char** splitString(char *str, char delimiter, int *no_of_tokens, int *max_length, int *flag);

int searchTokens(char **tokens, int no_of_tokens, char *str);

int findOpcode(char *string);

int getStringLength(char *str);

#endif

/*
ADD	 M	18	A = A + M
AND	 M	40	A = A AND M
COMP M	28	compares A and M
DIV	 M	24	A = A / M
J	 M	3C	PC = M
JEQ	 M	30	if CC set to =, PC = M
JGT	 M	34	if CC set to >, PC = M
JLT	 M	38	if CC set to <, PC = M
JSUB M	48	L = PC ; PC = M
LDA	 M	00	A = M
LDCH M	50	A[RMB] = M[RMB]
LDL	 M	08	L = M
LDX	 M	04	X = M
MUL	 M	20	A = A * M
OR	 M	44	A = A OR M
RD	 M	D8	A[RMB] = data specified by M[RMB]
RSUB 	4C	PC = L
STA	 M	0C	M = A
STCH M	54	M[RMB] = A[RMB]
STL	 M	14	M = L
STSW M	E8	M = SW
STX	 M	10	M = X
SUB	 M	1C	A = A – M
TD	 M	E0	test device specified by M
TIX	 M	2C	X = X + 1 ; compare X with M
WD	 M	DC	device specified by M[RMB] = A[RMB]
*/