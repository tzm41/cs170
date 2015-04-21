#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

char token[20];

// Cell structure
struct Cell {
    int isCons;
    // union of symbol and cons-cell, to improve recognition and memory utilization
    union {
        char* symbol;
        struct conscell {
            struct Cell* first;
            struct Cell* rest;
        } conscell;
    } cellunion;
};

//typedef struct Cell List;

// "constant" #f cell
List* f()
{
    List* fc = (List*)malloc(sizeof(List));
    fc->isCons = 0;
    fc->cellunion.symbol = "#f";
    return fc;
}

// "constant" #t cell
List* t()
{
    List* tc = (List*)malloc(sizeof(List));
    tc->isCons = 0;
    tc->cellunion.symbol = "#t";
    return tc;
}

// "constant" empty cell
List* empty()
{
    List* empty = (List*)malloc(sizeof(List));
    empty->isCons = 1;
    empty->cellunion.conscell.first = NULL;
    empty->cellunion.conscell.rest = NULL;
    return empty;
}

/****************************************************************
 Function: s_exprh(int lvl)
 --------------------
 Private helper method that parse accordingly. At the ground level,
 lvl == 0, do not get extra token; otherwise, get the next token for
 recursive parsing.
 ****************************************************************/
List* s_exprh(int lvl)
{
    List* local = (List*)malloc(sizeof(List));
    List* temp = (List*)malloc(sizeof(List));
    if (strcmp(token, "(") == 0) {
        // case of lparen
        strcpy(token, getToken());
        local->isCons = 1;
        local->cellunion.conscell.first = s_exprh(lvl + 1);
        temp = local;
        while (strcmp(token, ")") != 0) {
            List* newCell = (List*)malloc(sizeof(List));
            newCell->isCons = 1;

            temp->cellunion.conscell.rest = newCell;
            temp = temp->cellunion.conscell.rest;
            temp->cellunion.conscell.first = s_exprh(lvl + 1);
        }
        temp->cellunion.conscell.rest = NULL;
    }
    else if (strcmp(token, "'") == 0) {
        // in the case of quotation mark
        // create the corresponding cons-cell structure
        // as the (quote) command
        strcpy(token, getToken());

        // make quote cell
        List* quote = (List*)malloc(sizeof(List));
        quote->isCons = 0;
        quote->cellunion.symbol = (char*)malloc(sizeof(char));
        strcpy(quote->cellunion.symbol, "quote");

        List* next = (List*)malloc(sizeof(List));
        next->isCons = 1;
        next->cellunion.conscell.first = s_exprh(lvl + 1);
        next->cellunion.conscell.rest = NULL;

        local->isCons = 1;
        local->cellunion.conscell.first = quote;
        local->cellunion.conscell.rest = next;
        return local;
    }
    else {
        if (strcmp(token, "#t") == 0) {
            local = t();
        }
        else if (strcmp(token, "#f") == 0) {
            local = f();
        }
        else if (strcmp(token, "()") == 0) {
            local->isCons = 1;
            local->cellunion.conscell.first = empty();
            local->cellunion.conscell.rest = NULL;
        }
        else {
            local->isCons = 0;
            local->cellunion.symbol = (char*)malloc(sizeof(char));
            strcpy(local->cellunion.symbol, token);
        }
    }
    // if not ground level, get next token for recursive parsing
    if (lvl != 0) {
        strcpy(token, getToken());
    }
    return local;
}

/****************************************************************
 Function: s_expr()
 --------------------
 Construct cons-cell structure from s-expression, preserving
 level relationship.
 ****************************************************************/
List* s_expr()
{
    strcpy(token, getToken());
    return s_exprh(0);
}

/****************************************************************
 Function: printListLevel(conscell* first, int onGround)
 --------------------
 Private helper method for recursive list printing. If it is the
 ground level for this round of printing, then does not print
 parenthesis around the result, otherwise behave the opposite.
 ****************************************************************/
void printListLevel(List* first, int onGround)
{
    if (first) {
        if (first->isCons == 0) {
            printf("%s", first->cellunion.symbol);
        }
        else {
            if (onGround != 0) {
                printf("(");
            }
            if (first->cellunion.conscell.first) {
                printListLevel(first->cellunion.conscell.first, 1);
            }
            if (first->cellunion.conscell.rest) {
                printf(" ");
                printListLevel(first->cellunion.conscell.rest, 0);
            }
            if (onGround != 0) {
                printf(")");
            }
        }
    }
}

/****************************************************************
 Function: printList(conscell* first)
 --------------------
 Print a list of cons-cells in Scheme format, starting with
 the pointer to the first cons-cell.
 ****************************************************************/
void printList(List* first)
{
    printListLevel(first, 1);
}
