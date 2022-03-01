#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "S1934_sic_assembler.h"


int min(int a, int b)
{
	return(a < b ? a : b);
}

void freeList(List *list)
{
	List *temp = list;
	while (temp != NULL)
	{
		temp = temp->next;
		free(list);
		list = temp;
		/* code */
	}	
}

void initializeOptab()
{
	op_tab = NULL;
	op_tab = (List *)malloc(NO_OF_OPCODES * sizeof(List));
	if(op_tab == NULL)
	{
		printf(" >! Unable to initialize OPTAB!\n");
		return;
	}
	op_tab[0] = (List){"ADD", 0x18, NULL};
	op_tab[1] = (List){"AND", 0x40, NULL};
	op_tab[2] = (List){"COMP", 0x28, NULL};
	op_tab[3] = (List){"DIV", 0x24, NULL};
	op_tab[4] = (List){"J", 0x3C, NULL};
	op_tab[5] = (List){"JEQ", 0x30, NULL};
	op_tab[6] = (List){"JGT", 0x34, NULL};
	op_tab[7] = (List){"JLT", 0x38, NULL};
	op_tab[8] = (List){"JSUB", 0x48, NULL};
	op_tab[9] = (List){"LDA", 0x00, NULL};
	op_tab[10] = (List){"LDCH", 0x50, NULL};
	op_tab[11] = (List){"LDL", 0x08, NULL};
	op_tab[12] = (List){"LDX", 0x04, NULL};
	op_tab[13] = (List){"MUL", 0x20, NULL};
	op_tab[14] = (List){"OR", 0x44, NULL};
	op_tab[15] = (List){"RD", 0xD8, NULL};
	op_tab[16] = (List){"RSUB", 0x4C, NULL};
	op_tab[17] = (List){"STA", 0x0C, NULL};
	op_tab[18] = (List){"STCH", 0x54, NULL};
	op_tab[19] = (List){"STL", 0x14, NULL};
	op_tab[20] = (List){"STSW", 0xE8, NULL};
	op_tab[21] = (List){"STX", 0x10, NULL};
	op_tab[22] = (List){"SUB", 0x1C, NULL};
	op_tab[23] = (List){"TD", 0xE0, NULL};
	op_tab[24] = (List){"TIX", 0x2C, NULL};
	op_tab[25] = (List){"WD", 0xDC, NULL};

}


List* insertIntoList(List *list, char *s, int c)
{
	List *temp = NULL;
	temp = (List *)malloc(sizeof(List));
	if(temp == NULL)
	{
		u_errno = 1;
		printf(" >! Error in insertIntoList\n");
		printf(" >! Unable to allocate memory!\n");
		freeList(list);
		return(NULL);
		// return(list);
	}

	strcpy(temp->symbol, s);
	temp->code = c;
	temp->next = NULL;
	
	if(list == NULL)
		list = temp;
	else
	{
		temp->next = list;
		list = temp;
	}

	return(list);
}

List* loadSymTab(List *symtab, char *filename)
{
	FILE *symbol_file = fopen(filename, "r");
	if( symbol_file == NULL)
	{
		printf(" >! Unable to open symbol file \n");
		perror(" >! Error ");
		return(NULL);
	}
	
	char *line = NULL;
	char delim = ' ';
	int n, flag, max_length;
	unsigned long len, r;
	char **tokens;

	len = 0;
	printf(" > Reading %s into symtab\n", filename);
	while( (r = getline(&line, &len, symbol_file)) != -1)
	{
		n = 0;
		max_length = MAX_COLUMN_SPACE_SEPARATION;
		flag = 2;
		tokens = splitString(line, delim, &n, &max_length, &flag);
		if(tokens != NULL)
		{
			if(tokens[0][0] != '#')
			{
				if(n == 2)
				{
					if(searchList(symtab, tokens[0]) >= 0)
					{
						printf(" >! Duplicate definition of Symbol %s!\n", tokens[0]);
						break;
					}
					else
					{
						symtab = insertIntoList(symtab, tokens[0], strtol(tokens[1], NULL, 16));
						if(symtab == NULL)
						{
							printf(" >! Error while trying to load symtab\n");
							break;
						}
						// fprintf(symtab_file, "%5s  %04lx\n", tokens[0], locctr);
					}
				}
				else
				{
					printf(" >! Error in loadSymtab\n");
				}
			}

		}
	}
	fclose(symbol_file);
	return(symtab);
}


int searchList(List *list, char *s)
{
	List *temp = list;
	while(temp != NULL)
	{
		if(strcmp(temp->symbol, s) == 0)
		{
			return(temp->code);
		}
		temp = temp->next;
	}
	return(-1);
}

void printTokens(char **tokens, int no_of_tokens)
{
	printf("\n Tokens are: ");
	for(int i = 0; i < no_of_tokens; i++)
	{
		printf("%s, ", tokens[i]);
	}
	printf("\n");
}

void freeTokens(char **tokens, int no_of_tokens)
{
	if(tokens != NULL)
	{	for(int i = 0; i < no_of_tokens; i++)
		{
			if(tokens[i] != NULL)
			{
				free(tokens[i]);
				tokens[i] = NULL;
			}
		}
		free(tokens);
		tokens = NULL;
	}
}

char** splitString(char *str, char delimiter, int *no_of_tokens, int *max_length, int *flag)
{
	/*
		Splits the given string using the given delimiter
		Returns array of char **
		Number of tokens stored in third argument
		Fourth argument must have the expected no of spaces used to separate the columns
			After execution Size of array of char * is stored in fourth argument
		Fifth argument should be set to the expected no of columns
			After execution it will be set to the probable index of opcode column
				if n = 4 => opcode in 2
				if n = 3 => opcode in 1 or 2
				if n = 2 => opcode in 0 or 1
				if n = 1 => it is opcode
	*/

	int i, j, k;
	int n = strlen(str);
	
	// Find the number of possible splits
	int count = 0, bad_delim = 0;
	int ss_length = 0, max_sslength = 0;
	if(str[0] == delimiter)
		count--;
	for(i = 0; i < n; )
	{
		if(str[i] == delimiter)
		{
			max_sslength = ss_length > max_sslength ? ss_length : max_sslength;
			ss_length = 0;
			j = i;
			while(str[j] == delimiter && j < n)
			{
				bad_delim++;
				j++;
			}
			count++;
			i = j;
		}
		else
		{
			// check if line is a comment
			if(str[i] == '#' && count == 0 && ss_length == 0)
			{
				*max_length = 0;
				*no_of_tokens = 0;
				*flag = -2;
				return(NULL);
			}
			if(str[i] > 31 && str[i] < 127)
				ss_length++;
			if(str[i] == '\'')
			{
				i++;
				while(str[i] != '\'' && i < n)
				{
					ss_length++;
					i++;
				}
			}
			if(str[i] == '\"')
			{
				i++;
				while(str[i] != '\"' && i < n)
				{
					ss_length++;
					i++;
				}
			}
			i++;
		}
	}
	if(str[n - 1] != delimiter)
		count++;
	
	max_sslength = (max_sslength == 0 || max_sslength < ss_length) ? ss_length : max_sslength;
	if(max_sslength == 0)
		return NULL;
	
	char** tokens = (char **)malloc(count * sizeof(char *));
	if(tokens == NULL)
	{
		printf("\n >! Error in splitString\n");
		printf(" >! Unable to allocate memory for token!\n");
		return(NULL);
	}
	for(i = 0; i < count; i++)
	{
		tokens[i] = (char *)malloc( (max_sslength + 1)* sizeof(char));
		if(tokens[i] == NULL)
		{
			printf("\n >! Error in splitString\n");
			printf(" >! Unable to allocate memory for token[%d]!\n", i);
			while((--i) > -1)
				free(tokens[i]);
			free(tokens);
			return(NULL);
		}
	}
	// copying tokens into the array
	j = 0;
	for( i = 0; i < count; i++)
	{
		while(str[j] == delimiter && j < n)
		{
			// moving through bad tokens
			// printf("\n%c, ", str[j]);
			j++;
		}
		k = 0;
		while(str[j] != delimiter && j < n && k < max_sslength + 1)
		{
			// copy to array
			// printf("\n%c, ", str[j]);
			if(str[j] == '\'')
			{
				tokens[ i ][k++] = str[j++];
				while(str[j] != '\'' && str[j] != '\n' && j < n)
				{
					// printf("\n%c, ", str[j]);
					tokens[ i ][k++] = str[j++];
				}
			}
			else if(str[j] == '\"')
			{
				tokens[ i ][k++] = str[j++];
				while(str[j] != '\"' && str[j] != '\n' && j < n)
				{
					tokens[ i ][k++] = str[j++];
				}
			}
			else if(str[j] != '\n' && (str[i] > 31 && str[i] < 127))
				tokens[ i ][k++] = str[j++];
			else
				j++;
		}
		tokens[ i ][k++] = '\0';
		// printf("%d\n", k);
	}

	// Finding the position of opcode
	if(*flag > 0)
	{
		if(count == 1)
			*flag = 0;
		else if(*flag == 3)
		{
			if(count == 1)
				*flag = 0;
			else if(count == 2)
				*flag = (n > (*flag - 1)*(MAX_LABEL_SIZE + *max_length) ? 0 : 1);
			else if(count == 3)
				*flag = 1;
			else
				*flag = -1;
		}
		else if(*flag == 4)
		{
			if(count == 2)
				*flag = 1;
			else if(count == 3)
				*flag = (n > (*flag - 1)*(MAX_LABEL_SIZE + *max_length) ? 1 : 2);
			else if(count == 4)
				*flag = 2;
			else
				*flag = -1;
		}
		else if(*flag == 2)
		{
			;
		}
		else
		{
			*flag = -1;
			printf(" >! Error in splitTokens\n");
		}
	}
	*max_length = max_sslength + 1;
	*no_of_tokens = count;
	return(tokens);
}

int searchTokens(char **tokens, int no_of_tokens, char *str)
{
	for(int i = 0; i < no_of_tokens; i++)
	{
		if(!strcmp(str, tokens[i]))
		{
			return(i);
		}
	}
	return(-1);
}

int findOpcode(char *string)
{
	/*
		Finds which of the tokens is a valid opcode
		returns the index if found
			else -1
	*/
	if(op_tab == NULL)
	{
		initializeOptab();
		if(op_tab == NULL)
			return(-2);
		// for(int i = 0; i < NO_OF_OPCODES; i++)
		// 	printf("| %s | %d | %ls |\n", op_tab[i].symbol, op_tab[i].code, (unsigned int *)op_tab[i].next);
	}
	for(int i = 0; i < NO_OF_OPCODES; i++)
	{
		if(!strcmp(string, op_tab[i].symbol))
		{
			return(i);
		}
	}
	return(-1);
}

int getStringLength(char *str)
{
	int len = 0;
	int n = strlen(str);
	// printf("%d\n", n);
	if(n > 2)
	{
		if(str[0] == 'C' && str[1] == '\'')
		{
			for(int i = 2; i < n && str[i] != '\''; i++)
				len++;
			// printf("%d\n", len);
			return( len == (n - 3) ? len : -1);
		}
	}
	return(n);
}