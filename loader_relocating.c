#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "S1934_sic_assembler.h"

int memory_row_length = MAX_ADDRESS_SIZE + 4*4*OPCODE_HEX_SIZE + 2;
/*
	states:
		0 : initial, Haven't read header record yet
		1 : Reading commands
		2 : Read END of program
		3 : Error

*/

void writeMemoryToFile(FILE *output_file, char **text_record, unsigned long program_length)
{
	/*
		Writes data in memory into the file
	*/
	int count = 0;
	char object_record[9];
	fprintf(output_file, " +-------+-----------------------------------------+\n");
	fprintf(output_file, " |Memory |          Contents of memory             |\n");
	fprintf(output_file, " |Address| 0     3   4     7   8     B   C     F   |\n");
	fprintf(output_file, " +-------+-----------------------------------------+");
	for(int i = 0; i < program_length; i++)
	{
		// printf("rec=%s\n", text_record[i]);
		strncpy(object_record, text_record[i], MAX_ADDRESS_SIZE);
		fprintf(output_file, "\n | %.*s | ", MAX_ADDRESS_SIZE, object_record);
		count = 0;
		for(int j = MAX_ADDRESS_SIZE + 1; j < memory_row_length; j++)
		{
			object_record[count] = text_record[i][j];
			count++;
			if(count == 8)
			{
				object_record[9] = '\0';
				count = 0;
				fprintf(output_file, "%.*s  ", 8, object_record);
				strcpy(object_record, "");
			}
		}
		fprintf(output_file, "|");
	}
	fprintf(output_file, "\n +-------+-----------------------------------------+");
}

int main(int argc, char **argv)
{
	printf(" Relocating Loader Program\n");

	if(argc != 2)
	{
		printf(" >! Improper Arguments\n");
		printf(" Usage: ./relocating_loader.out input_file\n");
		// printf(" Input will be read from file_name\n", output_file_name);
		printf(" Output will be written to %s\n", relocated_loaded_mem_file_name);
		return(0);
	}

	int n, i, j, state = 0, max_length, flag, line_counter = 0, memory_row_n, memory_col_n;
	char delim = ' ', *search_string;
	FILE *output_file, *relocated_loaded_mem_file;
	char *line = NULL;
	char **tokens;
	char **memory_blocks;
	unsigned long len = 0, record_length, r, locctr = 0, prev_locctr = 0, starting_address = 0, program_length = 0, no_of_records = 0;
	unsigned long modification_address = 0, modification_length, new_load_address; 
	strcpy(output_file_name, argv[1]);
	output_file = fopen(output_file_name, "r");
	if( output_file == NULL)
	{
		printf(" >! Unable to read object_file \n");
		perror(" >! Error ");
		return(-1);
	}

	relocated_loaded_mem_file = fopen(relocated_loaded_mem_file_name, "w");
	if( relocated_loaded_mem_file == NULL)
	{
		printf(" >! Unable to create output_file \n");
		perror(" >! Error ");
		return(-1);
	}

	printf(" > Reading %s\n", output_file_name);

	locctr = 0;
	prev_locctr = 0;
	while( (r = getline(&line, &len, output_file)) != -1)
	{
		// prev_locctr = locctr;
		line_counter++;
		n = 0;
		max_length = MAX_COLUMN_SPACE_SEPARATION;
		flag = 0;
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
		// printf("%s\n", temp_memory_row);
		// printf("%d | %d\n", line_counter, no_of_bytes_in_temp);
		if(tokens != NULL)
		{
			char column_of_row[4*OPCODE_HEX_SIZE+1] = {'\0'};
			// Read header record
			if(!strcmp(tokens[0], "H") && state == 0)
			{
				starting_address = strtol(tokens[2], NULL, 16);
				program_length = strtol(tokens[3], NULL, 16);
				program_length = program_length - starting_address;
				printf(" > Header record Found!\n");
				printf(" > Where should the program %s be relocated to ? (Provide Address > 1000)\n", tokens[1]);
				printf(" >>> ");
				scanf("%lx", &new_load_address);
				// printf("=>%ld\n", program_length);
				if(program_length % 3 != 0)
					program_length += 3 - (program_length%3);
				locctr = (new_load_address / 16)*16;
				no_of_records = program_length/16 + 1;
				// CREATE MEMORY
				// printf("=%ld %ld %ld %ld\n", starting_address, locctr, no_of_records, program_length);
				memory_blocks = (char **)calloc(no_of_records, sizeof(char *));
				if(memory_blocks == NULL)
				{
					printf(" >! Unable to allocate memory!\n");
					state = 3;
					break;
				}
				// Initializing memory
				for(i = 0; i < no_of_records; i++)
				{
					memory_blocks[i] = (char *)calloc(memory_row_length, sizeof(char *));
					if(memory_blocks[i] == NULL)
					{
						printf(" >! Unable to allocate memory %d!\n", i);
						state = 3;
						break;
					}
					for(j = 0; j < memory_row_length; j++)
						memory_blocks[i][j] = '\0';			
				}
				if(state == 3)
					break;
				memory_row_n = 0;
				memory_col_n = 0;
				sprintf(memory_blocks[memory_row_n], "%.*lx ", MAX_ADDRESS_SIZE, locctr);
				memory_col_n += MAX_ADDRESS_SIZE + 1;
				// If program doesnt start at an proper position
				if(new_load_address % 16 != 0)
				{
					for(i = 0; i < 2*(new_load_address % 16); i++)
					{
						memory_blocks[memory_row_n][memory_col_n] = 'X';
						memory_col_n++;
					}
				}
				// printf("%d \n", memory_col_n);
				state = 1;
			}
			// Read Text Record
			if(!strcmp(tokens[0], "T") && state == 1)
			{
				if(memory_blocks == NULL)
				{
					printf(" >! Memory block not initialized\n");
					state = 3;
					break;
				}
				record_length = strtol(tokens[2], NULL, 16);
				for(i = 0; i < record_length; i++)
				{
					j = 0;
					while(j <  (OPERAND_HEX_SIZE + OPCODE_HEX_SIZE))
					{
						memory_blocks[memory_row_n][memory_col_n++] = tokens[3+i][j++];
						if(memory_col_n == memory_row_length)
						{
							memory_blocks[memory_row_n][memory_row_length] = '\0';
							j--;
							memory_row_n++;
							locctr += 16;
							sprintf(memory_blocks[memory_row_n], "%.*lx ", MAX_ADDRESS_SIZE, locctr);
							memory_col_n = MAX_ADDRESS_SIZE + 1;
						}
					}
				}
			}
			// Read Modification record
			if(!strcmp(tokens[0], "M"))
			{
				char modification_place[7];
				modification_address = strtol(tokens[1], NULL, 16);
				modification_length = strtol(tokens[2], NULL, 16);
				i = (modification_address - starting_address - (new_load_address % 16))/(4*4*OPCODE_HEX_SIZE);
				j = (2*(modification_address - starting_address) - 4*4*OPCODE_HEX_SIZE*i + MAX_ADDRESS_SIZE + 1);
				int k, t = j;
				for(k = 0; k < modification_length; k++)
				{
					modification_place[k] = memory_blocks[i][t++];
				}
				modification_place[k] = '\0';
				unsigned long modification_value = strtol(modification_place, NULL, 16);
				// printf("> %d, %d, %lx, modification %s\n", i, j, modification_value, modification_place);
				modification_value += new_load_address;
				modification_value = modification_value % 160000;
				sprintf(modification_place, "%.*lx", (int)modification_length, modification_value);
				modification_place[modification_length] = '\0';
				// printf("> %d, %d, %lx, modification %s\n", i, j, modification_value, modification_place);
				t = j;
				for(k = 0; k < modification_length; k++)
				{
					memory_blocks[i][t++] = modification_place[k];
				}
			}
			// Read End Record
			if(!strcmp(tokens[0], "E"))
			{
				// If data does not fill the complete row,
				// padding it with 'X'
				if(memory_col_n < memory_row_length)
				{
					for(i = memory_col_n; i < memory_row_length - 1; i++)
						memory_blocks[memory_row_n][i] = 'X';
					memory_blocks[memory_row_n][memory_row_length] = '\0';
				}
				writeMemoryToFile(relocated_loaded_mem_file, memory_blocks, no_of_records);
				state = 2;
			}
						
		}
		// printf("%s\n", memory_row);
		freeTokens(tokens, n);
		tokens = NULL;
		free(line);
		line = NULL;
		if(state == 2)
			break;
	}
	// printf(" line \"%s\"\n", line);
	// printf("state : %d\n", state);
	fprintf(relocated_loaded_mem_file, "\n");
	
	// close files
	fclose(output_file);
	fclose(relocated_loaded_mem_file);

	// remove from memory
	if(state != 2) // premature break
		freeTokens(tokens, n);
	else
	{
		printf(" > Success!\n");
		printf(" > Program length = %lu bytes\n", ((locctr - new_load_address) + memory_col_n - MAX_ADDRESS_SIZE - 1));
		printf(" > Output written to ./%s \n", relocated_loaded_mem_file_name);
	}

	if(state == 3)
	{
		printf(" >! Deleting Output file!\n");
		remove(relocated_loaded_mem_file_name);
	}
	if(line != NULL)
		free(line);
	if(op_tab != NULL)
		free(op_tab);
	if(memory_blocks != NULL)
		freeTokens(memory_blocks, no_of_records);
	return(0);
}