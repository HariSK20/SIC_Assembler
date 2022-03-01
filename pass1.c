#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "S1934_sic_assembler.h"

/*
	states:
		0 : initial, Haven't read start yet
		1 : Reading commands
		2 : Read END of program
		3 : Error

*/

int main(int argc, char **argv)
{
	printf(" Pass 1 of a 2 pass assembler\n");

	if(argc != 2)
	{
		printf(" >! No arguments passed!\n");
		printf(" Usage: ./pass1.out input_file\n");
		printf(" Output will be written to ./%s and ./%s\n", symbol_file_name, intermediate_file_name);
		return(0);
	}

	int n, i, j, w_flag = 0, state = 0, max_length, flag, line_counter = 0;
	char delim = ' ', *search_string;
	FILE *input_file, *symtab_file, *intermediate_file;
	char *line = NULL;
	char **tokens;
	unsigned long len = 0, r, locctr = 0, prev_locctr = 0, starting_address = 0;;
	List *symbol_tab = NULL;

	input_file = fopen(argv[1], "r");
	if( input_file == NULL)
	{
		printf(" >! Unable to read input_file %s\n", argv[1]);
		perror(" >! Error ");
		return(-1);
	}
	symtab_file = fopen(symbol_file_name, "w");
	intermediate_file = fopen(intermediate_file_name, "w");
	if(symtab_file == NULL || intermediate_file == NULL)
	{
		state = 3;
		printf(" >! Unable to create Symbol and intermediate files!\n");
		fclose(intermediate_file);
		fclose(symtab_file);
		fclose(input_file);
		remove(symbol_file_name);
		remove(intermediate_file_name);		
	}

	locctr = 0;
	prev_locctr = 0;
	printf(" > Reading %s\n", argv[1]);
	while( (r = getline(&line, &len, input_file)) != -1)
	{
		prev_locctr = locctr;
		line_counter++;
		n = 0;
		max_length = MAX_COLUMN_SPACE_SEPARATION;
		flag = 3;
		tokens = splitString(line, delim, &n, &max_length, &flag);
		if(flag == -2 && len > 0)
		{
			// state = 4;
			w_flag = 1;
		}
		// printf("line %d, state %d n = %d, tokens = %p\n", line_counter, state, n, tokens);
		if(state == 2)
			w_flag = 1;
		if(tokens != NULL)
		{
			if(state == 0 && line[0] != '#')
			{
				// If First line is not a comment
				search_string = "START";
				j = searchTokens(tokens, n, search_string);
				if( j > -1)
				{
					starting_address = j > -1 ? ( n > 1 ? strtol(tokens[n - 1], NULL, 16) : 0) : 0;
					state = 1;
					locctr = starting_address;
					prev_locctr = locctr;
				}
				w_flag = 1;
			}
			else if(state > 0 && tokens[0][0] != '#') // skip comment line
			{
				// Checking Label Field
				if(flag > 0)
				{
					// Label Field (before opcode) present
					if(searchList(symbol_tab, tokens[0]) >= 0)
					{
						state = 3;
						printf(" >! Duplicate definition of Symbol %s in line %d!\n", tokens[0], line_counter);
						break;
					}
					else
					{
						symbol_tab = insertIntoList(symbol_tab, tokens[0], locctr);
						if(symbol_tab == NULL)
						{
							state = 3;
							printf(" >! exiting because of %d\n", line_counter);
							break;
						}
						fprintf(symtab_file, "%5s  %04lx\n", tokens[0], locctr);
					}
				}
				/*
					Checking if a valid opcode exists
					if n = 4 => opcode in 3
					if n = 3 => opcode in 2 or 3
					if n = 2 => opcode in 1 or 2
					if n = 1 => it is opcode
				*/
				int op_pos;
				// printf("flag %d, state %d\n", flag, state);
				op_pos = findOpcode(tokens[flag]);
				if(op_pos > -1)
				{
					locctr += INSTRUCTION_SIZE_BYTES;
					// printf(" >%d Opcode found col %d\n", line_counter, flag);
				}
				else if(op_pos == -2)
				{
					printf(" >%d! ", line_counter);
					state = 3;
					break;
				}
				else
				{
					// No SIC opcode found
					if(n == 3)
					{
						if(!strcmp(tokens[1], "WORD"))
						{
							locctr += 3;
						}
						else if(!strcmp(tokens[1], "RESW"))
						{
							locctr += 3*(atoi(tokens[2]));
						}
						else if(!strcmp(tokens[1], "BYTE"))
						{
							// printf("tok= %s\n", tokens[2]);
							int len = getStringLength(tokens[2]);
							if(len < 1)
							{
								printf("%d\n", len);
								printf(" >! Error at %d! Improper declaration of string\n", line_counter);
								state = 3;
								break;
							}
							locctr += len;
						}
						else if(!strcmp(tokens[1], "RESB"))
						{
							locctr += atoi(tokens[2]);
						}
						else
						{
							state = 3;
							printf(" >! No Valid Opcode Found on line %d!\n", line_counter);
							break;
						}
					}
					else if(n == 1)
					{
						// printf("here tokens[0] = \"%s\"", tokens[0]);
						// printf(" %d\n", tokens[0][3]);
						if(!strcmp(tokens[0], "END"))
						{
							// printf(" END reached\n");
							fprintf(symtab_file, "%5s  %04lx\n", tokens[0], locctr);
							state = 2;
						}
					}
					else
					{
						state = 3;
						printf(" >! No Valid Opcode Found on line %d!\n", line_counter);
						break;						
					}
				}
			}
		}
		// printf(" | %lu | %s \n", prev_locctr, line);
		if(state == 4 || w_flag == 1)
		{
			fprintf(intermediate_file, "       %s", line);
			// state = 1;
			w_flag = 0;
		}
		else
			fprintf(intermediate_file, "%05lx  %s", prev_locctr, line);
		freeTokens(tokens, n);
		tokens = NULL;
		free(line);
		line = NULL;
	}
	// printf(" line \"%s\"\n", line);
	// printf("state : %d\n", state);
	fprintf(intermediate_file, "\n");
	
	// close files
	fclose(intermediate_file);
	fclose(symtab_file);
	fclose(input_file);

	// remove from memory
	if(state != 2) // premature break
		freeTokens(tokens, n);
	else
	{
		printf(" > Success!\n");
		printf(" > Program length = %lu bytes\n", (locctr - starting_address));
		printf(" > Output written to ./%s and ./%s\n", symbol_file_name, intermediate_file_name);
	}

	if(state == 3)
	{
		printf(" >! Deleting symbol and intermediate files!\n");
		remove(symbol_file_name);
		remove(intermediate_file_name);
	}
	if(symbol_tab != NULL)
		freeList(symbol_tab);
	if(line != NULL)
		free(line);
	if(op_tab != NULL)
		free(op_tab);
	
	return(0);
}