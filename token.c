/*-----------------------------------------
// Author: Ryan Campbell
//
// REMARKS: Implement a function to tokenize
// strings passed to it.  The function returns
// an array of Strings.  The last token is NULL.
//-----------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "token.h"

#define task_input_length 100
#define task_file_length 500


/*------------------------------------------------------
// tokenizeLine
//
// PURPOSE: This method takes a line from a file, and
// tokenizes the line to be further processed.
//
// INPUT PARAMETERS:
//     Passes a line of commands to be processed.
// OUTPUT
//      Returns an array of  malloc'd tokens.
//------------------------------------------------------*/
char** tokenizeLine(char line[])
{
    int num_args;
    char *token;
    char **all_tokens = malloc(task_input_length*sizeof (char *));
    assert(all_tokens != NULL);


    /* Tokenize the command, and load it into the array of arguments */
    token = strtok(line, " \n\r\t");
    all_tokens[0] = token;
    num_args = 1;

    /*Prep the next token */
    token = strtok(NULL, " \n\r\t");

    /*Process 1 token at a time */
    while(token != NULL) {

        all_tokens[num_args] = token;

        num_args++;

        /* Get next token */
        token = strtok(NULL, " \n\r\t");
    }
    all_tokens[num_args] = NULL;

    return all_tokens;
}


void destroyTokens(char** tokens){

    int token_num;
    char *token_to_delete;

    token_num = 0;

    while(tokens[token_num] != NULL){

        token_to_delete = tokens[token_num];
        token_num++;
        free(token_to_delete);
    }
}

