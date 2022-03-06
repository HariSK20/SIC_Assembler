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
	printf(" Macro Processor\n");
	printf(" Currently does not support nested macro definition\n");

	if(argc != 2)
	{
		printf(" >! No arguments passed!\n");
		printf(" Usage: ./macro_proc.out input_file\n");
		printf(" Output will be written to ./%s\n", macro_processed_file_name);
		return(0);
	}
	int number_of_macros = 0;
	int n, i, j, w_flag = 0, state = 0, max_length, flag, line_counter = 0, output_line_counter = 0;
	char delim = ' ', *search_string;
	FILE *input_file, *macro_processed_file, *arg_tab_file, *def_tab_file, *name_tab_file;
	char *line = NULL;
	char **tokens;
	unsigned long len = 0, r, locctr = 0, prev_locctr = 0, starting_address = 0;
	Table *macro_name_tab = NULL;

	input_file = fopen(argv[1], "r");
	if( input_file == NULL)
	{
		printf(" >! Unable to read input_file %s\n", argv[1]);
		perror(" >! Error ");
		return(-1);
	}
	macro_processed_file = fopen(macro_processed_file_name, "w");
	if(macro_processed_file == NULL)
	{
		state = 3;
		printf(" >! Unable to create output file!\n");
		perror(" >! Error\n");
		fclose(macro_processed_file);
		fclose(input_file);
		remove(macro_processed_file_name);		
	}

	name_tab_file = fopen(macro_name_file_name, "w");
	if(name_tab_file == NULL)
	{
		state = 3;
		printf(" >! Unable to create output file!\n");
		perror(" >! Error\n");
		fclose(name_tab_file);
		fclose(macro_processed_file);
		fclose(input_file);
		remove(macro_processed_file_name);		
	}

	arg_tab_file = fopen(macro_argument_file_name, "w");
	if(macro_processed_file == NULL)
	{
		state = 3;
		printf(" >! Unable to create output file!\n");
		perror(" >! Error\n");
		fclose(arg_tab_file);
		fclose(name_tab_file);
		fclose(macro_processed_file);
		fclose(input_file);
		remove(macro_processed_file_name);		
	}

	def_tab_file = fopen(macro_definition_file_name, "w");
	if(macro_processed_file == NULL)
	{
		state = 3;
		printf(" >! Unable to create output file!\n");
		perror(" >! Error\n");
		fclose(def_tab_file);
		fclose(arg_tab_file);
		fclose(name_tab_file);
		fclose(macro_processed_file);
		fclose(input_file);
		remove(macro_processed_file_name);		
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
			/*
				Checking if a valid opcode exists
				if n = 4 => opcode in 3
				if n = 3 => opcode in 2 or 3
				if n = 2 => opcode in 1 or 2
				if n = 1 => it is opcode
			*/

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
				// w_flag = 1;
			}
			else if(state > 0 && tokens[0][0] != '#') // skip comment line
			{
				// Checking Label Field
				// printTable(macro_name_tab);
				if(n > 0)
				{
					int run_stat = 0;
					if(n > 1)
					{
						// printTokens(tokens, n);
						if(strcmp(tokens[1], "MACRO") == 0)
						{
							run_stat ++;
							// DEFINE MACRO
							if(searchTable(macro_name_tab, tokens[0]) != NULL)
							{
								state = 3;
								printf(" >! Duplicate definition of Symbol %s in line %d!\n", tokens[0], line_counter);
								break;
							}
							else
							{
								// New macro found
								macro_name_tab = insertIntoTable(macro_name_tab, tokens, n);
								// printTokens(tokens, n);
								if(macro_name_tab == NULL)
								{
									state = 3;
									printf(" >! ERROR! exiting because of line %d\n", line_counter);
									break;
								}
								// Inserting macro into def_tab
								fprintf(def_tab_file, "%s", line);
								fprintf(name_tab_file, "%s\n", macro_name_tab->name);
								int prev_line_counter = line_counter;
								// printf(" !!! %d %s\n", macro_name_tab->no_of_arguments, macro_name_tab->arg_tab[0].symbol);
								for( i = 0; i < macro_name_tab->no_of_arguments; i++)
								{
									// printf("! %s \n", macro_name_tab->arg_tab[i].symbol);
									fprintf(arg_tab_file, "%s\n", macro_name_tab->arg_tab[i].symbol);
								}

								i = 0;
								while((r = getline(&line, &len, input_file)) != -1)
								{
									n = 0;
									max_length = MAX_COLUMN_SPACE_SEPARATION;
									flag = 3;
									tokens = splitString(line, delim, &n, &max_length, &flag);
									if(n > 0)
									{
										if(strcmp(tokens[1], "MEND") != 0)
										{
											// Increase the size of def_tab when needed
											// printf(" >> %d ", i);
											if(i % DISCRETE_REALLOC_SIZE == 0)
											{
												if(macro_name_tab->number_of_lines == 0)
												{
													macro_name_tab->definition = (char **)calloc(DISCRETE_REALLOC_SIZE, sizeof(char *));
													if(macro_name_tab->definition == NULL)
													{
														printf(" >! Error while trying to create def_tab\n");
														printf(" >! Unable to allocate memory!\n");
														state = 3;
														break;
													}
												}
												else 
												{
													// printf(">> ??%ld\n", (macro_name_tab->number_of_lines + DISCRETE_REALLOC_SIZE)*sizeof(char *));
													macro_name_tab->definition = (char **)realloc(macro_name_tab->definition, (macro_name_tab->number_of_lines + DISCRETE_REALLOC_SIZE)*sizeof(char *) );
													if(macro_name_tab->definition == NULL)
													{
														printf(" >! Error while trying to increase def_tab\n");
														printf(" >! Unable to allocate memory!\n");
														state = 3;
														break;
													}
												}
												// Initialize each new array
												for(j = macro_name_tab->number_of_lines; j < macro_name_tab->number_of_lines + DISCRETE_REALLOC_SIZE; j++)
												{
													macro_name_tab->definition[j] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
													if(macro_name_tab->definition == NULL)
													{
														printf(" >! Error while trying to allocate def_tab[i]\n");
														printf(" >! Unable to allocate memory!\n");
														state = 3;
														break;
													}
												}
												if(state == 3)
													break;
											}
											// Save definition line
											fprintf(def_tab_file, "%s", line);
											sprintf(macro_name_tab->definition[i], "%s", line);
											i++;
											macro_name_tab->number_of_lines = i;
										}
										else 
										{
											if(strcmp(tokens[0], macro_name_tab->name) != 0)
											{
												printf(" >! Improper end of Macro %s at line %d\n", macro_name_tab->name, line_counter);
												state = 3;
												freeTokens(tokens, n);
												tokens = NULL;
												break;
											}
											fprintf(def_tab_file, "%s", line);
											w_flag = 1;
											line_counter++;
											break;
										}
									}
									if(tokens != NULL)
									{
										freeTokens(tokens, n);
										tokens = NULL;
									}
									line_counter++;
								}
								if(state == 3)
								{
									if(tokens != NULL)
									{
										freeTokens(tokens, n);
										tokens = NULL;
									}
									break;
								}

								locctr += 3*(line_counter - prev_line_counter);
								line_counter = prev_line_counter - 1;
							}
						}
					}
					else if(n > 0)
					{
						// printf(" Here");
						if(!strcmp(tokens[0], "END"))
						{
							run_stat++;
							// printf(" END reached\n");
							// fprintf(symtab_file, "%5s  %04lx\n", tokens[0], locctr);
							state = 2;

						}
					}
					if(run_stat == 0)
					{
						// Check if a macro was called
						// printTokens(tokens, n);
						for( i = 0; i < n; i++)
						{
							Table *temp = searchTable(macro_name_tab, tokens[i]);
							if(temp != NULL)
							{
								// printf(" >%d> %s %d \n", line_counter, temp->name, i);
								// Macro invocation found
								// EXPAND
								w_flag = 1;
								for(j = 0; j < temp->number_of_lines; j++)
								{
									if(temp->no_of_arguments > 0)
									{
										// Split definition line to check for arguments
										int n2 = 0, max_length2 = MAX_COLUMN_SPACE_SEPARATION, flag2 = 3; 
										char **tokens2;
										tokens2 = splitString(temp->definition[j], delim, &n2, &max_length2, &flag2);
										if(i < n)
										{
											if(n - i - 1 != temp->no_of_arguments)
											{
												state = 3;
												freeTokens(tokens2, n2);
												tokens2 = NULL;
												printf(" >! Imporoper call of macro %s at line %d\n", temp->name, line_counter);
												break;
											}
											// Arguments were specified
											else 
											{
												for(int k = 0; k < n2; k++)
												{
													if(tokens2[k][0] == '&')
													{
														// printf(" >> 243");
														// linear search argument
														int l;
														for(l = 0; l < temp->no_of_arguments; l++)
														{
															// printf(" |%s|, |%s|", temp->arg_tab[l].symbol, tokens2[k]);
															if(strcmp(tokens2[k], temp->arg_tab[l].symbol ) == 0)
															{	
																// printf("yes\n");
																strcpy(tokens2[k], tokens[i+1 + l]);
																break;
															}
														}
													}
												}
											}
										}
										if(flag2 == 0)
										{
											for(int k = 0; k < MAX_LABEL_SIZE; k++)
												fprintf(macro_processed_file, " ");
											fprintf(macro_processed_file, "  ");	
										}
										for(int k = 0; k < n2; k++)
										{
											fprintf(macro_processed_file, "%.*s  ", MAX_LABEL_SIZE, tokens2[k]);
											for(int _ = 0; _ < MAX_COLUMN_SPACE_SEPARATION; _++)
												fprintf(macro_processed_file, " ");

										}
										fprintf(macro_processed_file, "\n");
										freeTokens(tokens2, n2);
										tokens2 = NULL;
									}
									else
										fprintf(macro_processed_file, "%s\n", temp->definition[j]);
									output_line_counter++;
								}
								if(state == 3)
									break;
							}
							// else 
							// {
							// 	printf(" %s Not found\n", tokens[i]);
							// }
						}
						if(state == 3)
							break;
					}

				}
			}
		}
		// printf(" | %lu | %s \n", prev_locctr, line);
		if(state == 4 || w_flag == 1)
		{
			// fprintf(macro_processed_file, "%s", line);
			// state = 1;
			w_flag = 0;
		}
		else 
		{
			fprintf(macro_processed_file, "%s", line);
			output_line_counter++;
		}
		freeTokens(tokens, n);
		tokens = NULL;
		free(line);
		line = NULL;
	}
	// printf(" line \"%s\"\n", line);
	// printf("state : %d\n", state);
	fprintf(macro_processed_file, "\n");
	
	// close files
	fclose(def_tab_file);
	fclose(arg_tab_file);
	fclose(name_tab_file);
	fclose(macro_processed_file);
	fclose(input_file);

	// remove from memory
	if(state != 2) // premature break
		freeTokens(tokens, n);
	else
	{
		printf(" > Success!\n");
		printf(" > Program length = %d lines\n", output_line_counter);
		printf(" > Output written to ./%s\n", macro_processed_file_name);
	}

	if(state == 3)
	{
		printf(" >! Deleting incomplete files!\n");
		remove(macro_processed_file_name);
	}
	if(macro_name_tab != NULL)
		freeTable(macro_name_tab);
	if(line != NULL)
		free(line);
	if(op_tab != NULL)
		free(op_tab);
	
	return(0);
}