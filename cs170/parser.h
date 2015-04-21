/****************************************************************
 File: parser.h
 ----------------
 This is the interface for a parser for part of Scheme. From the
 flow of token generated from getToken() method, it has operation
 to construct the cons-cell structure, print a cons-cell structure,
 or evaluate some of the functions.	
 ****************************************************************/
#ifndef PARSER
#define PARSER
#include <stdlib.h>
#include "lexer.h"

struct Cell;

typedef struct Cell List;

List* empty();
List* t();
List* f();

void startTokens(int maxLength);

List* s_expr();

void printList(List* first);

#endif