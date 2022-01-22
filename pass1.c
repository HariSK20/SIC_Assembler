#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "S1934_sic_assembler.h"


int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf(" >! No arguments passed!\n");
		printf(" Usage: ./pass1.out input_file\n");
		printf(" Output written to ./symtab.txt and ./intermediate.txt\n");
		return(0);
	}

	int n, i, j;
	char delim = ' ';
	FILE *file;
	char *line = NULL;
	unsigned long len = 0, r;
	List *symtab = NULL, *temp = NULL;

	file = fopen(argv[1], "r");
	if( file == NULL)
	{
		printf(" >! Unable to read file %s\n", argv[1]);
		perror(" >! Error ");
		return(-1);
	}

	printf(" > Reading %s\n", argv[1]);
	while( (r = getline(&line, &len, file)) != -1)
	{
		
	}

	fclose(file);
	if(line != NULL)
		free(line);
	
	return(0);
}