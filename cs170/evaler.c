//
//  evaler.c
//
//  Created by Colin Tan on 4/20/15.
//  Copyright (c) 2015 Colin Tan. All rights reserved.
//  Implementation of evaluation methods
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evaler.h"
#include "parser.h"

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

// private pointer to association list
List* asl;
// private pointer to function list
List* fcl;

// make a deep copy of a list
List* listcpy(List* list)
{
    if (list) {
        List* newList = (List*)malloc(sizeof(List));
        newList->isCons = list->isCons;
        if (newList->isCons == 1) {
            newList->cellunion.conscell.first = listcpy(list->cellunion.conscell.first);
            newList->cellunion.conscell.rest = listcpy(list->cellunion.conscell.rest);
        }
        else {
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
    return evals(list->cellunion.conscell.first, 0)->cellunion.conscell.first;
}

// implementation of cdr function
List* cdr(List* list)
{
    List* rest = evals(list->cellunion.conscell.first, 0)->cellunion.conscell.rest;
    if (rest == NULL)
        return empty();
    else
        return rest;
}

// implementation of cons function
List* cons(List* list1, List* list2)
{
    // pointer pointing to the start of the list
    List* head = (List*)malloc(sizeof(List));
    head->isCons = 1;
    head->cellunion.conscell.first = evals(list1, 0);
    List* rest = evals(list2, 0);
    // do not attach an empty list
    if (rest->isCons == 1 && rest->cellunion.conscell.first == NULL && rest->cellunion.conscell.rest == NULL) {
        head->cellunion.conscell.rest = NULL;
    }
    else {
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
    head->cellunion.conscell.first = listcpy(list1);
    head->cellunion.conscell.rest = NULL;
    List* ptr = head->cellunion.conscell.first;
    while (ptr->cellunion.conscell.rest) {
        ptr = ptr->cellunion.conscell.rest;
    }
    // do not attach an empty list
    List* rest = listcpy(list2);
    if (rest->isCons == 1 && rest->cellunion.conscell.first == NULL && rest->cellunion.conscell.rest == NULL) {
        ptr->cellunion.conscell.rest = NULL;
    }
    else {
        ptr->cellunion.conscell.rest = rest;
    }
    return head->cellunion.conscell.first;
}

// implementation of symbol? function
List* symbolQ(List* list)
{
    if (evals(list->cellunion.conscell.first, 0)->isCons == 0)
        return t();
    else
        return f();
}

// implementation of list? function
List* listQ(List* list)
{
    if (evals(list->cellunion.conscell.first, 0)->isCons == 1)
        return t();
    else
        return f();
}

// implementation of null? function
List* nullQ(List* list)
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
        if (strcmp(list1->cellunion.symbol, list2->cellunion.symbol) == 0)
            return 1;
        else
            return 0;
    }
    else if (list1->isCons == 1 && list2->isCons == 1) {
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
    }
    else if (list1 == NULL && list2 == NULL)
        return 1;
    else
        return 0;
}

// implementation of the equal? function
List* equalQ(List* list1, List* list2)
{
    if (equals(list1, list2) == 1)
        return t();
    else
        return f();
}

/****************************************************************
 * Function: assoc(List* symbol, List* list)
 * --------------------
 * Implementation of the assoc function. Returns the pair
 * associated with the symbol or #f if the symbol is not
 * the first element of any pair
 ****************************************************************/
List* assoc(List* symbol, List* list)
{
    List* ptr = list;
    while (ptr != NULL) {
        // going to the next pair, checking the first symbol
        if (ptr->cellunion.conscell.first != NULL) {
            if (ptr->cellunion.conscell.first->cellunion.conscell.first != NULL) {
                if (ptr->cellunion.conscell.first->cellunion.conscell.first->cellunion.symbol != NULL) {
                    if (strcmp(symbol->cellunion.symbol, ptr->cellunion.conscell.first->cellunion.conscell.first->cellunion.symbol) == 0)
                        return ptr->cellunion.conscell.first;
                }
            }
        }
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
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", add);
    return result;
}

// implementation of the numeric - function
List* numsub(List* list1, List* list2)
{
    int add = atoi(list1->cellunion.symbol) - atoi(list2->cellunion.symbol);
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", add);
    return result;
}

// implementation of the numeric * function
List* nummul(List* list1, List* list2)
{
    int add = atoi(list1->cellunion.symbol) * atoi(list2->cellunion.symbol);
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", add);
    return result;
}

// implementation of the numeric / function
List* numdiv(List* list1, List* list2)
{
    int add = atoi(list1->cellunion.symbol) / atoi(list2->cellunion.symbol);
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", add);
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


// implementation of function? function
List* funcQ(List* list)
{
    List* ptr = evals(list->cellunion.conscell.first, 0);
    if (ptr->isCons == 0) {
        // is a symbol
        if (assoc(ptr, fcl)->cellunion.conscell.first != NULL) {
            return t();
        } else
            return f();
    } else
        return f();
}

/****************************************************************
 * Function: eval(List* list, List* assocList)
 * --------------------
 * Helper method for eval, keeping track of the level. Also modifies
 * and utilizes the association list.
 ****************************************************************/
List* evals(List* list, int lvl)
{
    char name[20] = "";
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
            // try to evaluate definitions
            List* nameCell = (List*)malloc(sizeof(List));
            nameCell->isCons = 0;
            nameCell->cellunion.symbol = (char*)malloc(sizeof(char));
            strcpy(nameCell->cellunion.symbol, name);
            
            List* funcResult = assoc(nameCell, fcl);
            List* assocResult = assoc(nameCell, asl);
            if (equals(funcResult, f()) != 1) {
                // evaluate function
                printf("func detected");
                List* funcDef = funcResult->cellunion.conscell.rest->cellunion.conscell.first;
                printList(funcDef);
            } else if (equals(assocResult, f()) != 1) {
                // evaluate symbol defintion
                if (assocResult->isCons == 1) {
                    list = assocResult->cellunion.conscell.rest;
                }
            } else {
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
                    return evals(symbolQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "function?") == 0) {
                    return evals(funcQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "list?") == 0) {
                    return evals(listQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "null?") == 0) {
                    return evals(nullQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "equal?") == 0) {
                    return evals(equalQ(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2)), lvl + 1);
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
                if (strcmp(name, "*") == 0) {
                    return nummul(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
                }
                if (strcmp(name, "/") == 0) {
                    return numdiv(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
                }
                if (strcmp(name, "cond") == 0) {
                    return evals(cond(list), lvl + 1);
                }
                if (strcmp(name, "if") == 0) {
                    return iff(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "define") == 0) {
                    if (equals(symbolQ(list->cellunion.conscell.rest), t()) == 1) {
                        // defining symbol
                        // name cell of the symbol
                        List* name = (List*)malloc(sizeof(List));
                        name->isCons = 0;
                        name->cellunion.symbol = (char*)malloc(sizeof(char));
                        strcpy(name->cellunion.symbol, list->cellunion.conscell.rest->cellunion.conscell.first->cellunion.symbol);
                        
                        // definition of the symbol
                        List* def = (List*)malloc(sizeof(List));
                        def->isCons = 1;
                        def->cellunion.conscell.first = evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 1);
                        def->cellunion.conscell.rest = NULL;
                        
                        // single list of the name, and the defintion
                        List* cell = (List*)malloc(sizeof(List));
                        cell->isCons = 1;
                        cell->cellunion.conscell.first = name;
                        cell->cellunion.conscell.rest = def;
                        
                        // encapsulation of the symbol definition
                        List* cap = (List*)malloc(sizeof(List));
                        cap->isCons = 1;
                        cap->cellunion.conscell.first = cell;
                        cap->cellunion.conscell.rest = NULL;
                        
                        asl = append(cap, asl);

                    } else if (equals(listQ(list->cellunion.conscell.rest), t()) == 1) {
                        // defining function
                        // name cell of the function
                        List* name = (List*)malloc(sizeof(List));
                        name->isCons = 0;
                        name->cellunion.symbol = (char*)malloc(sizeof(char));
                        strcpy(name->cellunion.symbol, list->cellunion.conscell.rest->cellunion.conscell.first->cellunion.conscell.first->cellunion.symbol);
                        
                        // definition cell of the function
                        List* def = (List*)malloc(sizeof(List));
                        def->isCons = 1;
                        def->cellunion.conscell.first = list;
                        def->cellunion.conscell.rest = NULL;
                        
                        // the single list of the function
                        List* cell = (List*)malloc(sizeof(List));
                        cell->isCons = 1;
                        cell->cellunion.conscell.first = name;
                        cell->cellunion.conscell.rest = def;
                        
                        // encapsulation of the function
                        List* cap = (List*)malloc(sizeof(List));
                        cap->isCons = 1;
                        cap->cellunion.conscell.first = cell;
                        cap->cellunion.conscell.rest = NULL;
                        
                        fcl = append(cap, fcl);
                    }
                    return list;
                }
                // quit
                if (strcmp(name, "exit") == 0) {
                    printf("Have a nice day!\n");
                    exit(0);
                }
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
            // evaluate definitions
            // TODO: add function
            List* nameCell = (List*)malloc(sizeof(List));
            nameCell->isCons = 0;
            nameCell->cellunion.symbol = (char*)malloc(sizeof(char));
            strcpy(nameCell->cellunion.symbol, name);
            
            List* assocResult = assoc(nameCell, asl);
            if (assocResult != NULL) {
                if (assocResult->isCons == 1) {
                    list = assocResult->cellunion.conscell.rest->cellunion.conscell.first;
                }
            }
        }
    }
    return list;
}

/****************************************************************
 Function: eval(List* list, List* assocList)
 --------------------
 Evaluate a list from pointer to the start of the list, according
 to the definitions of the functions. Treat "#f" as empty list "()".
 Use the corresponding association list for defintions.
 ****************************************************************/
List* eval(List* list, List* assocList, List* funcList)
{
    asl = assocList;
    fcl = funcList;
    List* result = evals(list, 0);
    // return the lists back to main()
    *assocList = *asl;
    *funcList = *fcl;
    return result;
}