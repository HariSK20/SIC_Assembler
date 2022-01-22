#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "S1934_sic_assembler.h"

int main(int argc, char** argv)
{
	int counter, n;
	char delim = ' ';
    char **tokens;
	printf("Program Name Is: %s",argv[0]);
    if(argc==1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
    if(argc>=2)
    {
        printf("\nNumber Of Arguments Passed: %d",argc);
        printf("\n----Following Are The Command Line Arguments Passed----");
        for(counter=0;counter<argc;counter++)
        {
		    printf("\nargv[%d]: %s",counter,argv[counter]);
			n = 0;
			tokens = splitString(argv[counter], delim, &n);
			printTokens(tokens, n);
			free(tokens);
		}
    }

	FILE *ptr;
	char *line = NULL;
	unsigned long len = 0, r;

	ptr = fopen(argv[1], "r");
    if (ptr == NULL)
    {
		printf(" >! Unable to read file %s\n", argv[1]);
		return(-1);
	}

    while ((r = getline(&line, &len, ptr)) != -1) {
        printf("Retrieved line of length %zu:\n", r);
        printf("%s", line);
		n = 0;
		tokens = splitString(line, delim, &n);
		printTokens(tokens, n);
		free(tokens);
    }

    fclose(ptr);
    if (line)
        free(line);

	printf("\n");
	return(0);
}