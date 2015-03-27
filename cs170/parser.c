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
List* empty() {
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

// make a deep copy of a list
List* listcpy(List* list)
{
    if (list) {
        List* newList = (List*)malloc(sizeof(List));
        newList->isCons = list->isCons;
        if (newList->isCons == 1) {
            newList->cellunion.conscell.first = listcpy(list->cellunion.conscell.first);
            newList->cellunion.conscell.rest = listcpy(list->cellunion.conscell.rest);
        } else {
            newList->cellunion.symbol = (char*)malloc(sizeof(char));
            strcpy(newList->cellunion.symbol, list->cellunion.symbol);
        }
        return newList;
    }
    return NULL;
}

// implementation of car function
List* car(List* list)
{
    return eval(list->cellunion.conscell.first)->cellunion.conscell.first;
}

// implementation of cdr function
List* cdr(List* list)
{
    return eval(list->cellunion.conscell.first)->cellunion.conscell.rest;
}

// implementation of cons function
List* cons(List* list1, List* list2)
{
    // pointer pointing to the start of the list
    List* head = (List*)malloc(sizeof(List));
    head->isCons = 1;
    head->cellunion.conscell.first = eval(list1);
    List* rest = eval(list2);
    // do not attach an empty list
    if (rest->isCons == 1 && rest->cellunion.conscell.first == NULL && rest->cellunion.conscell.rest == NULL) {
        head->cellunion.conscell.rest = NULL;
    } else {
        head->cellunion.conscell.rest = rest;
    }
    return head;
}

// implementation of append function, without doing surgery to the arguments
List* append(List* list1, List* list2)
{
    // pointer pointing to the start of the list
    List* head = (List*)malloc(sizeof(List));
    head->isCons = 1;
    head->cellunion.conscell.first = listcpy(eval(list1));
    head->cellunion.conscell.rest = NULL;
    List* ptr = head->cellunion.conscell.first;
    while (ptr->cellunion.conscell.rest) {
        ptr = ptr->cellunion.conscell.rest;
    }
    ptr->cellunion.conscell.rest = listcpy(eval(list2));
    return head->cellunion.conscell.first;
}

// implementation of symbol? function
List* symbol(List* list)
{
    if (eval(list->cellunion.conscell.first)->isCons == 0)
        return t();
    else
        return f();
}

// implementation of null? function
List* null(List* list)
{
    List* head = eval(list->cellunion.conscell.first);
    if (head->isCons == 1 && head->cellunion.conscell.first == NULL && head->cellunion.conscell.rest == NULL)
        return t();
    else
        return f();
}

// implementation of quote function
List* quote(List* list)
{
    return list->cellunion.conscell.rest->cellunion.conscell.first;
}

// implementation of the equal? function
List* equal(List* list1, List* list2)
{
    if (list1->isCons == 0 && list2->isCons == 0) {
        if(strcmp(list1->cellunion.symbol, list2->cellunion.symbol) == 0)
            return t();
        else
            return f();
    }
}

/****************************************************************
 Function: eval(List* list)
 --------------------
 Evaluate a list from pointer to the start of the list, according
 to the definitions of the functions. Treat "#f" as empty list "()".
 ****************************************************************/
List* eval(List* list)
{
    char name[20];
    if (list) {
        if (list->isCons == 1) {
            // entering from a cons-cell
            if (list->cellunion.conscell.first == NULL) {
                return empty();
            }
            else if (list->cellunion.conscell.first->isCons == 0) {
                // check if next level of cons-cell contains a function name
                strcpy(name, list->cellunion.conscell.first->cellunion.symbol);
            }
            else {
                if (list->cellunion.conscell.first)
                    list = eval(list->cellunion.conscell.first);
                if (list->cellunion.conscell.rest)
                    list = eval(list->cellunion.conscell.rest);
            }
            // switch for each function
            if (strcmp(name, "quote") == 0) {
                return eval(quote(list));
            }
            // have not yet figure out how to deal with quote mark
            if (strcmp(name, "'") == 0) {
                return list->cellunion.conscell.rest;
            }
            if (strcmp(name, "car") == 0) {
                return eval(car(list->cellunion.conscell.rest));
            }
            if (strcmp(name, "cdr") == 0) {
                return eval(cdr(list->cellunion.conscell.rest));
            }
            if (strcmp(name, "symbol?") == 0) {
                return eval(symbol(list->cellunion.conscell.rest));
            }
            if (strcmp(name, "null?") == 0) {
                return eval(null(list->cellunion.conscell.rest));
            }
            if (strcmp(name, "cons") == 0) {
                return cons(list->cellunion.conscell.rest->cellunion.conscell.first, list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first);
            }
            if (strcmp(name, "append") == 0) {
                return append(list->cellunion.conscell.rest->cellunion.conscell.first, list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first);
            }
            if (strcmp(name, "exit") == 0) {
                printf("Have a nice day!\n");
                exit(0);
            }
        }
        else {
            // entering from a symbol
            strcpy(name, list->cellunion.symbol);
            // treat "#f" as an empty list
            if (strcmp(name, "#f") == 0) {
                list = empty();
            }
        }
    }
    return list;
}