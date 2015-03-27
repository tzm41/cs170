#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

char token[20];

static char *lexeme;
static char c;
static int lookahead;

struct cell {
    int isCons;
    union {
        char* symbol;
        struct conscell {
            struct cell* first;
            struct cell* rest;
        } conscell;
    } cellunion;
};

void printAtom (char *con, int level) {
    for (int i = 0; i < level; i++) {
        printf(" ");
    }
    printf("%s\n", con);
}

static void newToken (int maxLength) {
    if (lexeme != NULL)
        free(lexeme);
    
    if ((lexeme = (char *) calloc(maxLength, sizeof(char))) == NULL)
    {
        printf("Out of memory, too many tokens.\n");
        exit(0);
    }
}

void startTokens (int maxLength) {
    lookahead = 0;
    lexeme = NULL;
    newToken(maxLength);
}

char *getToken () {
    int i;                            //local index for lexeme
    
    if (!lookahead)                   //get first char
        c = getchar();
    
    while ((c == ' ') || (c == '\n')) //skip white space
        c = getchar();
    
    if ((c == ')') || (c == '\'')) {  //Case (1): right paren or quote
        lexeme[0] = c;
        lexeme[1] = '\0';
        lookahead = 0;
    }
    else if (c == '(') {              //Case (2): left paren or ()
        lookahead = 1;
        c = getchar();
        while ((c == ' ') || (c == '\n'))
            c = getchar();
        if (c == ')') {
            strcpy(lexeme, "()");          //empty list token
            lookahead = 0;
        }
        else
            strcpy(lexeme, "(");
    }
    else if (c == '#') {              //Case (3): #t or #f
        lookahead = 0;
        c = getchar();
        if ((c != 't') && (c != 'f')) {
            printf("Illegal symbol after #.\n");
            exit(1);
        }
        if (c == 't')
            strcpy(lexeme, "#t");
        else
            strcpy(lexeme, "#f");
    }
    else {                            //Case (4): scan for symbol
        i = 0;
        lookahead = 1;
        while ((c != '(') && (c != ')') && (c != '\'') && (c != ' ') && (c != '\n')) {
            lexeme[i++] = c;
            c = getchar();
        }/* while */
        lexeme[i] = '\0';
    }
    
    return lexeme;
}

void se (int level) {
        if (!strcmp(token, "(")) {
            // case of lparen
            for (int i = 0; i < level; i++) {
                printf(" ");
            }
            printf("S_Expression\n");
            for (int i = 0; i < level; i++) {
                printf(" ");
            }
            printf("(\n");
            strcpy(token, getToken());
            se(level + 2);
            // while inside the paren
            while (strcmp(token, ")")) {
                strcpy(token, getToken());
                se(level + 2);
            }
            strcpy(token, getToken());
        } else if (strcmp(token, ")")) {
            // case of not paren
            for (int i = 0; i < level; i++) {
                printf(" ");
            }
            printf("S_Expression\n");
            printAtom(token, level + 2);
        } else if (!strcmp(token, ")")) {
            // case of not paren
            for (int i = 0; i < level - 2; i++) {
                printf(" ");
            }
            printf(")\n");
            if (level > 2) {
                strcpy(token, getToken());
                se(level - 2);
            }
            return;
        }
}

void S_Expression () {
    strcpy(token, getToken());
    se (0);
}

void printList (struct cell first) {
    
}