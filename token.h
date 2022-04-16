/*-----------------------------------------
// NAME: Ryan Campbell
// STUDENT NUMBER: 7874398
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment #3
//
// REMARKS: Implement a function to tokenize
// strings passed to it.  The function returns
// an array of Strings.  The last token is NULL.
//-----------------------------------------*/

#ifndef SCHEDULING_TOKEN_H
#define SCHEDULING_TOKEN_H

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
char** tokenizeLine(char line[]);


void destroyTokens(char** tokens);

#endif /*SCHEDULING_TOKEN_H */
