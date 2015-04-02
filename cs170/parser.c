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
    else if (strcmp(token, "'") == 0) {
        // in the case of quotation mark
        // create the corresponding cons-cell structure
        // as the (quote) command
        strcpy(token, getToken());
        
        // make quote cell
        List* quote = (List*)malloc(sizeof(List));
        quote->isCons = 0;
        quote->cellunion.symbol = (char*)malloc(sizeof(char*));
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

/*
 * Implementation of all the evaluation methods
 */
List* evals(List* list, int lvl);

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
    List* head = evals(list->cellunion.conscell.first, 2);
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

// helper for the equal? function
int equals(List* list1, List* list2)
{
    if (list1->isCons == 0 && list2->isCons == 0) {
        // a symbol cell, compare the symbol
        if(strcmp(list1->cellunion.symbol, list2->cellunion.symbol) == 0)
            return 1;
        else
            return 0;
    } else if(list1->isCons == 1 && list2->isCons == 1) {
        // a cons-cell, recursively compare its first and rest
        // if both pointers are NULL, then that branch is equal
        int x;
        int y;
        if (list1->cellunion.conscell.first && list2->cellunion.conscell.first)
            x = equals(list1->cellunion.conscell.first, list2->cellunion.conscell.first);
        else if (list1->cellunion.conscell.first == NULL && list2->cellunion.conscell.first == NULL)
            x = 1;
        else
            x = 0;
        if (list1->cellunion.conscell.rest && list2->cellunion.conscell.rest)
            y = equals(list1->cellunion.conscell.rest, list2->cellunion.conscell.rest);
        else if (list1->cellunion.conscell.rest == NULL && list2->cellunion.conscell.rest == NULL)
            y = 1;
        else
            y = 0;
        // if both branch is equal, comparsion is equal
        if (x == 1 && y == 1)
            return 1;
        else
            return 0;
    } else if(list1 == NULL && list2 == NULL)
        return 1;
    else
        return 0;
}

// implementation of the equal? function
List* equal(List* list1, List* list2)
{
    if (equals(list1, list2) == 1)
        return t();
    else
        return f();
}

// implementation of the assoc function
// returns the pair associated with the symbol
// and #f if the symbol is not the first element of any pair
List* assoc(List* symbol, List* list)
{
    List* ptr = list;
    while (ptr->cellunion.conscell.rest != NULL) {
        // going to the next pair, checking the first symbol
        if (strcmp(symbol->cellunion.symbol, ptr->cellunion.conscell.first->cellunion.conscell.first->cellunion.symbol) == 0)
            return ptr->cellunion.conscell.first;
        ptr = ptr->cellunion.conscell.rest;
    }
    return f();
}

// implementation of the numeric > function
List* numgt(List* list1, List* list2)
{
    if (atoi(list1->cellunion.symbol) > atoi(list2->cellunion.symbol))
        return t();
    else
        return f();
}

// implementation of the numeric < function
List* numle(List* list1, List* list2)
{
    if (atoi(list1->cellunion.symbol) < atoi(list2->cellunion.symbol))
        return t();
    else
        return f();
}

// implementation of the numeric = function
List* numeq(List* list1, List* list2)
{
    if (atoi(list1->cellunion.symbol) == atoi(list2->cellunion.symbol))
        return t();
    else
        return f();
}

// implementation of the numeric + function
List* numadd(List* list1, List* list2)
{
    int add = atoi(list1->cellunion.symbol) + atoi(list2->cellunion.symbol);
    char a[20];
    sprintf(a, "%d", add);
    List* result = (List*)malloc(sizeof(List*));
    result->isCons = 0;
    result->cellunion.symbol = a;
    return result;
}

// implementation of the numeric - function
List* numsub(List* list1, List* list2)
{
    int add = atoi(list1->cellunion.symbol) - atoi(list2->cellunion.symbol);
    char a[20];
    sprintf(a, "%d", add);
    List* result = (List*)malloc(sizeof(List*));
    result->isCons = 0;
    result->cellunion.symbol = a;
    return result;
}

// implementation of the cond function
List* cond(List* list)
{
    List* top = list;
    while (top->cellunion.conscell.rest != NULL) {
        top = top->cellunion.conscell.rest;
        // true if condition cons-cell evaluated to be ture
        // or the keyword is else
        if (strcmp(evals(top->cellunion.conscell.first->cellunion.conscell.first, 0)->cellunion.symbol, "else") == 0 || strcmp(evals(top->cellunion.conscell.first->cellunion.conscell.first, 0)->cellunion.symbol, "#t") == 0) {
            return top->cellunion.conscell.first->cellunion.conscell.rest;
        }
    }
    return f();
}

// implementation of the if function
List* iff(List* list)
{
    if (strcmp(evals(list->cellunion.conscell.first, 0)->cellunion.symbol, "#t") == 0)
        return evals(list->cellunion.conscell.rest->cellunion.conscell.first, 2);
    else
        return evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, 2);
}

// helper method for eval, keeping track of the level
List* evals(List* list, int lvl)
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
                    list = evals(list->cellunion.conscell.first, lvl + 1);
                if (list->cellunion.conscell.rest)
                    list = evals(list->cellunion.conscell.rest, lvl + 1);
            }
            // switch for each function
            if (strcmp(name, "quote") == 0) {
                return quote(list);
            }
            if (strcmp(name, "car") == 0) {
                return evals(car(list->cellunion.conscell.rest), lvl + 1);
            }
            if (strcmp(name, "cdr") == 0) {
                return evals(cdr(list->cellunion.conscell.rest), lvl + 1);
            }
            if (strcmp(name, "symbol?") == 0) {
                return evals(symbol(list->cellunion.conscell.rest), lvl + 1);
            }
            if (strcmp(name, "null?") == 0) {
                return evals(null(list->cellunion.conscell.rest), lvl + 1);
            }
            if (strcmp(name, "equal?") == 0) {
                return evals(equal(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2)), lvl + 1);
            }
            if (strcmp(name, "cons") == 0) {
                return cons(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "append") == 0) {
                return append(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "assoc") == 0) {
                return assoc(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, ">") == 0) {
                return numgt(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "<") == 0) {
                return numle(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "=") == 0) {
                return numeq(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "+") == 0) {
                return numadd(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "-") == 0) {
                return numsub(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
            }
            if (strcmp(name, "cond") == 0) {
                return evals(cond(list), lvl + 1);
            }
            if (strcmp(name, "if") == 0) {
                return iff(list->cellunion.conscell.rest);
            }
            if (strcmp(name, "exit") == 0) {
                printf("Have a nice day!\n");
                exit(0);
            }
        }
        else {
            // entering from a symbol
            strcpy(name, list->cellunion.symbol);
            // treat "#f" as an empty list, if the level is not 0 or 1
            // which is only for aesthetic purpose
            if (strcmp(name, "#f") == 0 && !(lvl == 0 || lvl == 1)) {
                list = empty();
            }
        }
    }
    return list;
}

/****************************************************************
 Function: eval(List* list)
 --------------------
 Evaluate a list from pointer to the start of the list, according
 to the definitions of the functions. Treat "#f" as empty list "()".
 ****************************************************************/
List* eval(List* list) {
    return evals(list, 0);
}