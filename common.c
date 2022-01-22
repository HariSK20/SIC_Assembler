#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "S1934_sic_assembler.h"

List* insertIntoList(List *list, char *s, int c)
{
	List *temp = NULL;
	temp = (List *)malloc(sizeof(List));
	if(temp == NULL)
	{
		u_errno = 1;
		printf(" >! Error in insertIntoList\n");
		printf(" >! Unable to allocate memory!\n");
		// return(list);
	}
	else
	{
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
	}
	return(list);
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

void printTokens(char **tokens, int no_of_tokens)
{
	printf("\n Tokens are: ");
	for(int i = 0; i < no_of_tokens; i++)
	{
		printf("%s, ", tokens[i]);
	}
	printf("\n");
}

char** splitString(char *str, char delimiter, int *no_of_tokens)
{
	/*
		Splits the given string using the given delimiter
		Returns array of char * 
		Size of array of char * is stored in third argument
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
			ss_length++;
			i++;
		}
	}
	if(str[n - 1] != delimiter)
		count++;
	max_sslength = max_sslength == 0 ? ss_length : max_sslength;
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
			if(str[j] != '\n')
				tokens[ i ][k] = str[j++];
			k++;
		}
		tokens[ i ][k++] = '\0';
		// printf("%d\n", k);
	}

	*no_of_tokens = count;
	return(tokens);
}

