#ifndef PARSER
#define PARSER
#include <stdlib.h>

void printAtom (char *con, int level);

void startTokens (int maxLength);

char * getToken ();

void S_Expression ();

void printList ();

#endif