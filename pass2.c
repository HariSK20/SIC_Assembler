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

void writeTextRecord(FILE *output_file, char *text_record, char *temp_text_record, int *no_of_record_written, unsigned long *locctr)
{
	// Get the first executable address
	// sscanf();
	if(*locctr == -1)
		*locctr = 0;
	sprintf(text_record, "T %.*lx %02x%.*s", (OPCODE_HEX_SIZE + OPERAND_HEX_SIZE), *locctr, *no_of_record_written, min(*no_of_record_written, TEXT_RECORD_COUNT)*(OPCODE_HEX_SIZE + OPERAND_HEX_SIZE + 1), temp_text_record);
	*no_of_record_written = 0;
	fprintf(output_file, "%s\n", text_record);
	strcpy(temp_text_record, "");
	strcpy(text_record, "");
	*locctr = -1;
}


int main(int argc, char **argv)
{
	printf(" Pass 2 of a 2 pass assembler\n");

	if(argc != 1)
	{
		printf(" >! No arguments required!\n");
		printf(" Usage: ./pass2.out \n");
		printf(" Input will be read from ./%s and ./%s\n", symbol_file_name, intermediate_file_name);
		printf(" Output will be written to ./%s\n", output_file_name);
		return(0);
	}

	int n, i, j, w_flag = 0, state = 0, max_length, flag, line_counter = 0;
	char delim = ' ', *search_string;
	FILE *output_file, *symtab_file, *intermediate_file;
	char *line = NULL;
	int text_record_length = 4 + ( OPCODE_HEX_SIZE + OPERAND_HEX_SIZE + 1)*TEXT_RECORD_COUNT;
	int no_of_record_written = 0;
	char text_record[text_record_length], temp_text_record[text_record_length - 3];
	char **tokens;
	unsigned long len = 0, r, locctr = 0, prev_locctr = 0, starting_address = 0;;
	List *symbol_tab = NULL;

	output_file = fopen(output_file_name, "w");
	if( output_file == NULL)
	{
		printf(" >! Unable to create output_file \n");
		perror(" >! Error ");
		return(-1);
	}
	// symtab_file = fopen(symbol_file_name, "r");
	intermediate_file = fopen(intermediate_file_name, "r");
	if(intermediate_file == NULL)
	{
		state = 3;
		printf(" >! Unable to read intermediate file!\n");
		fclose(intermediate_file);
		fclose(output_file);
		remove(output_file_name);
	}

	symbol_tab = loadSymTab(symbol_tab, symbol_file_name);
	if(symbol_tab == NULL)
	{
		state = 3;
	}
	printf(" > Reading %s\n", intermediate_file_name);

	strcpy(temp_text_record, "\0");
	strcpy(text_record, "\0");

	locctr = 0;
	prev_locctr = 0;
	while( (r = getline(&line, &len, intermediate_file)) != -1)
	{
		// prev_locctr = locctr;
		line_counter++;
		n = 0;
		max_length = MAX_COLUMN_SPACE_SEPARATION;
		flag = 4;
		tokens = splitString(line, delim, &n, &max_length, &flag);
		if(flag == -1)
		{
			state = 3;
			printf(" >! Error at line %d\n", line_counter);
			break;
		}
		if(flag == -2 && len > 0)
		{
			// state = 4;
			// comments handled here
			continue;
		}
		// printf("line %d, state %d n = %d, tokens = %p\n", line_counter, state, n, tokens);
		// printf("%s\n", temp_text_record);
		// printf("%d | %d\n", line_counter, no_of_record_written);
		if(state == 2)
			w_flag = 1;
		if(tokens != NULL)
		{
			/*
				As the intermediate file instruction line has a structure of 
					address  label  opcode  operand
				We assume tokens[0] has the address field
			*/
			// line_counter++;
			if(state == 0 && line[0] != '#')
			{
				// If First line is not a comment
				// Checking For START of program
				search_string = "START";
				j = searchTokens(tokens, n, search_string);
				if( j > -1)
				{
					starting_address = j > -1 ? ( n > 1 ? strtol(tokens[n - 1], NULL, 16) : 0) : 0;
					state = 1;
					// Writing Header Record
					search_string = "END";
					locctr = searchList(symbol_tab, search_string);
					if(locctr < 0)
					{
						locctr = 0;
					}
					// Write Header record
					fprintf(output_file, "H %.*s %.*lx %.*lx\n", MAX_LABEL_SIZE, tokens[0], MAX_ADDRESS_SIZE, starting_address, MAX_ADDRESS_SIZE, locctr);
					locctr = starting_address;
					prev_locctr = locctr;
				}
			}
			else if(state > 0 && tokens[0][0] != '#') // skip comment line
			{
				locctr = strtol(tokens[0], NULL, 16);
				char object_code[OPCODE_HEX_SIZE+OPERAND_HEX_SIZE+1] = {'\0'};
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
					// OpCode Found
					// printf(" >%d Opcode found col %d\n", line_counter, flag);
					strcat(temp_text_record, " ");
					sprintf(object_code, "%.*x", OPCODE_HEX_SIZE, op_tab[op_pos].code);
					if(flag < n)
					{
						int temp = searchList(symbol_tab, tokens[flag+1]);
						if(temp >= 0)
						{
							// Operand found
							// symbol_tab = insertIntoList(symbol_tab, tokens[0], locctr);
							sprintf(object_code, "%.*s%.*x", OPCODE_HEX_SIZE, object_code,  OPERAND_HEX_SIZE, temp);
						}
						else
						{
							state = 3;
							printf(" >! Undefined Operand %s in line %d!\n", tokens[1], line_counter);
							break;
						}
					}
					else
					{
						sprintf(object_code, "%.*s%.*x", OPCODE_HEX_SIZE, object_code, OPERAND_HEX_SIZE, 0);
					}
					if(no_of_record_written == 0)
						prev_locctr = locctr;
					strcat(temp_text_record, object_code);
					no_of_record_written++;

				}
				else if(op_pos == -2)
				{
					printf(" >! line %d", line_counter);
					state = 3;
					break;
				}
				else
				{
					// if(prev_locctr == -1)
					// No SIC opcode found
					if(n == 4)
					{
						// strcat(temp_text_record, ",");
						if(!strcmp(tokens[flag], "WORD"))
						{
							strcat(temp_text_record, " ");
							sprintf(object_code, "%.*lx", (OPCODE_HEX_SIZE+OPERAND_HEX_SIZE), strtol(tokens[n-1], NULL, 16));
							strcat(temp_text_record, object_code);
							no_of_record_written++;
							// locctr += 3;
						}
						else if(!strcmp(tokens[flag], "BYTE"))
						{
							strcpy(object_code, "");
							int len = getStringLength(tokens[flag+1]);
							if(len < 1)
							{
								printf("%d\n", len);
								printf(" >! Error at %d! Improper declaration of string\n", line_counter);
								state = 3;
								break;
							}
							// char buffer[2*INSTRUCTION_SIZE_BYTES];
							// printf("tok= %s, len = %d\n", tokens[flag+1], len);
							for(int k = 0; k < (len/INSTRUCTION_SIZE_BYTES + 1)*INSTRUCTION_SIZE_BYTES ; k ++)
							{
								// printf("%s\n", temp_text_record);
								if(k >= len)
								{
									strcat(temp_text_record, " ");
									while(k%INSTRUCTION_SIZE_BYTES != 0)
									{
										strcat(temp_text_record, "00");
										// sprintf(object_code, "00%-.*s", OPCODE_HEX_SIZE*(k%INSTRUCTION_SIZE_BYTES), object_code);
										k++;
									}
									// sprintf(object_code, "00%-.*s", OPCODE_HEX_SIZE*((k-1)%INSTRUCTION_SIZE_BYTES), object_code);
								}
								if(k%INSTRUCTION_SIZE_BYTES == 0 && k!=0)
								{
									if(k != 0 && k < len)
										strcat(temp_text_record, " ");
									strcat(temp_text_record, object_code);
									strcpy(object_code, "");
									no_of_record_written++;
									// printf("%d\n", no_of_record_written);
								}
								sprintf(object_code, "%-.*s%02x", OPCODE_HEX_SIZE*(k%INSTRUCTION_SIZE_BYTES), object_code, tokens[flag+1][k+2]);
								if(no_of_record_written >= TEXT_RECORD_COUNT)
								{
									writeTextRecord(output_file, text_record, temp_text_record, &no_of_record_written, &prev_locctr);
								}
							}
						}
						else if(!strcmp(tokens[flag], "RESW"))
						{
							printf(" > RESW at %d, added RESW to output file\n", line_counter);
							int len = atoi(tokens[flag+1]);
							sprintf(object_code, "%0*d", 2*INSTRUCTION_SIZE_BYTES, 0);
							for(int k = 0; k < len; k ++)
							{
								// printf("%s\n", temp_text_record);
								// if(k != 0)
									strcat(temp_text_record, " ");
								strcat(temp_text_record, object_code);
								no_of_record_written++;
								if(no_of_record_written >= TEXT_RECORD_COUNT)
								{
									writeTextRecord(output_file, text_record, temp_text_record, &no_of_record_written, &prev_locctr);
								}
							}
						}
						else if(!strcmp(tokens[flag], "RESB"))
						{
							printf(" > RESB at %d, added RESB to output file\n", line_counter);
							int len = atoi(tokens[flag+1]);
							sprintf(object_code, "%*s", 2*INSTRUCTION_SIZE_BYTES, "RESB");
							for(int k = 0; k < len; k ++)
							{
								// printf("%s\n", temp_text_record);
								if(k != 0)
									strcat(temp_text_record, " ");
								strcat(temp_text_record, object_code);
								no_of_record_written++;
								if(no_of_record_written >= TEXT_RECORD_COUNT)
								{
									writeTextRecord(output_file, text_record, temp_text_record, &no_of_record_written, &prev_locctr);
								}
							}
						}
						else
						{
							state = 3;
							printf(" >! No Valid Opcode Found on line %d!\n", line_counter);
							break;
						}
					}
					else if(n == 2)
					{
						// printf("here tokens[0] = \"%s\"", tokens[0]);
						// printf(" %d\n", tokens[0][3]);
						if(!strcmp(tokens[flag], "END"))
						{
							// printf(" END reached\n");
							state = 2;
							if(no_of_record_written > 0)
							{
								writeTextRecord(output_file, text_record, temp_text_record, &no_of_record_written, &prev_locctr);
							}
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
			fprintf(output_file, "E %.*lx\n", MAX_ADDRESS_SIZE, starting_address);
			// state = 1;
			w_flag = 0;
		}
		else
			if(no_of_record_written >= TEXT_RECORD_COUNT)
			{
				writeTextRecord(output_file, text_record, temp_text_record, &no_of_record_written, &prev_locctr);
			}
		// printf("%s\n", text_record);
		freeTokens(tokens, n);
		tokens = NULL;
		free(line);
		line = NULL;
	}
	// printf(" line \"%s\"\n", line);
	// printf("state : %d\n", state);
	fprintf(output_file, "\n");
	
	// close files
	fclose(intermediate_file);
	fclose(output_file);

	// remove from memory
	if(state != 2) // premature break
		freeTokens(tokens, n);
	else
	{
		printf(" > Success!\n");
		printf(" > Program length = %lu bytes\n", (locctr - starting_address));
		printf(" > Output written to ./%s \n", output_file_name);
	}

	if(state == 3)
	{
		printf(" >! Deleting Output file!\n");
		remove(output_file_name);
	}
	if(symbol_tab != NULL)
		freeList(symbol_tab);
	if(line != NULL)
		free(line);
	if(op_tab != NULL)
		free(op_tab);
	
	return(0);
}