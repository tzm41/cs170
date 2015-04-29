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
#include <ctype.h>
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

// implementation of all the evaluation methods
List* evals(List* list, int lvl);

// implementation of function evaluation
List* evalFunc(List* augEnv, List* funcCell, int lvl);

// implementation of equal function
int equals(List* list1, List* list2);

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

// implementation of car function
List* car(List* list)
{
    return evals(list->cellunion.conscell.first, 0)->cellunion.conscell.first;
}

// implementation of cdr function
List* cdr(List* list)
{
    if (equals(list, empty()) == 1) {
        return empty();
    }
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
    // kick out empty list at the head
    if (head->cellunion.conscell.first->cellunion.conscell.first == NULL)
        return head->cellunion.conscell.first->cellunion.conscell.rest;
    else
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
    if (list->cellunion.conscell.first == NULL) {
        return t();
    }
    if (list->isCons == 0) {
        return f();
    }
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
    List* rest = list2;
    int add = atoi(list1->cellunion.symbol) + atoi(rest->cellunion.conscell.first->cellunion.symbol);
    // add the whole list
    while (rest->cellunion.conscell.rest != NULL) {
        rest = rest->cellunion.conscell.rest;
        add = add + atoi(rest->cellunion.conscell.first->cellunion.symbol);
    }
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
    int sub = atoi(list1->cellunion.symbol) - atoi(list2->cellunion.symbol);
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", sub);
    return result;
}

// implementation of the numeric * function
List* nummul(List* list1, List* list2)
{
    List* rest = list2;
    int mul = atoi(list1->cellunion.symbol) * atoi(list2->cellunion.conscell.first->cellunion.symbol);
    // multiply the whole list
    while (rest->cellunion.conscell.rest != NULL) {
        rest = rest->cellunion.conscell.rest;
        mul = mul * atoi(rest->cellunion.conscell.first->cellunion.symbol);
    }
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", mul);
    return result;
}

// implementation of the numeric / function
List* numdiv(List* list1, List* list2)
{
    int div = atoi(list1->cellunion.symbol) / atoi(list2->cellunion.symbol);
    // construct result cell
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 0;
    result->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(result->cellunion.symbol, "%d", div);
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

// implementation of the logical and function
List* and(List* list)
{
    List* ptr = list;
    while (ptr->cellunion.conscell.rest != NULL) {
        if (equals(evals(ptr->cellunion.conscell.first, 0), f()) == 1) {
            return f();
        }
        ptr = ptr->cellunion.conscell.rest;
    }
    return ptr->cellunion.conscell.first;
}

// implementation of the logical or function
List* or(List* list)
{
    List* ptr = list;
    while (ptr != NULL) {
        if (equals(evals(ptr->cellunion.conscell.first, 0), f()) == 0) {
            return evals(ptr->cellunion.conscell.first, 0);
        }
        ptr = ptr->cellunion.conscell.rest;
    }
    return f();
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

// implementation of list function
List* listFunc(List* list) {
    List* result = (List*)malloc(sizeof(List));
    result->isCons = 1;
    result->cellunion.conscell.first = NULL;
    result->cellunion.conscell.rest = NULL;
    
    List* ptr = list;
    while (ptr != NULL) {
        List* evalResult = evals(ptr->cellunion.conscell.first, 0);
        
        List* evalCell = (List*)malloc(sizeof(List));
        evalCell->isCons = 1;
        evalCell->cellunion.conscell.first = evalResult;
        evalCell->cellunion.conscell.rest = NULL;
        
        result = append(result, evalCell);
        ptr = ptr->cellunion.conscell.rest;
    }
    
    return result;
}

// implementation of number? function
List* numberQ(List* list)
{
    List* evalResult = evals(list, 0);
    if (evalResult->cellunion.conscell.first->isCons == 1) {
        return f();
    }
    char content[20] = "";
    strcpy(content, evalResult->cellunion.conscell.first->cellunion.symbol);
    for (int i = 0; i < strlen(content); i++) {
        if (!isdigit(content[i])) {
            return f();
        }
    }
    return t();
}

// implementation of the last function
List* last(List* list) {
    List* ptr = evals(list, 0);
    while (ptr->cellunion.conscell.rest != NULL) {
        ptr = ptr->cellunion.conscell.rest;
    }
    return ptr->cellunion.conscell.first;
}

// implementation of the length function
List* length(List* list) {
    List* ptr = evals(list->cellunion.conscell.first, 1);
    int length = 0;
    if (equals(ptr->cellunion.conscell.first, empty()) == 0) {
        length += 1;
    }
    while (ptr->cellunion.conscell.rest != NULL) {
        ptr = ptr->cellunion.conscell.rest;
        length += 1;
    }
    List* cell = (List*)malloc(sizeof(List));
    cell->isCons = 0;
    cell->cellunion.symbol = (char*)malloc(sizeof(char));
    sprintf(cell->cellunion.symbol, "%d", length);
    return cell;
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
                List* funcParam = funcResult->cellunion.conscell.rest->cellunion.conscell.first->cellunion.conscell.rest->cellunion.conscell.first->cellunion.conscell.rest;
                List* funcDef = funcResult->cellunion.conscell.rest->cellunion.conscell.first->cellunion.conscell.rest->cellunion.conscell.rest;
                
                // make function cell
                List* funcCell = (List*)malloc(sizeof(List));
                funcCell->isCons = 1;
                funcCell->cellunion.conscell.first = nameCell;
                funcCell->cellunion.conscell.rest = funcDef;
                
                // make augmented environment cell
                List* augmentEnv = (List*)malloc(sizeof(List));
                augmentEnv->isCons = 1;
                augmentEnv->cellunion.conscell.first = empty();
                augmentEnv->cellunion.conscell.rest = NULL;
                
                
                // construct parameter-variable pairs
                List* paramPtr = funcParam;
                List* argPtr = list->cellunion.conscell.rest;
                int paramCount = 0;
                while (paramPtr != NULL) {
                    List* thisParamSymbol = paramPtr->cellunion.conscell.first;
                    List* thisArgument = argPtr->cellunion.conscell.first;
                    
                    List* thisArgumentCell = (List*)malloc(sizeof(List));
                    thisArgumentCell->isCons = 1;
                    thisArgumentCell->cellunion.conscell.first = evals(thisArgument, lvl);
                    thisArgumentCell->cellunion.conscell.rest = NULL;
                    
                    List* thisPair = (List*)malloc(sizeof(List));
                    thisPair->isCons = 1;
                    thisPair->cellunion.conscell.first = thisParamSymbol;
                    thisPair->cellunion.conscell.rest = thisArgumentCell;
                    
                    List* pairCell = (List*)malloc(sizeof(List));
                    pairCell->isCons = 1;
                    pairCell->cellunion.conscell.first = thisPair;
                    pairCell->cellunion.conscell.rest = NULL;
                    
                    augmentEnv = append(pairCell, augmentEnv);
                    
                    paramPtr = paramPtr->cellunion.conscell.rest;
                    argPtr = argPtr->cellunion.conscell.rest;
                    paramCount += 1;
                }
                list = evalFunc(augmentEnv, funcCell, lvl);
                // restore definition environment
                for (int i = paramCount; i >= 0; i--) {
                    asl = cdr(asl);
                }
            } else if (equals(assocResult, f()) != 1) {
                // evaluate symbol defintion
                if (assocResult->isCons == 1) {
                    return assocResult->cellunion.conscell.rest;
                }
            } else {
                // switch for each function
                if (strcmp(name, "quote") == 0) {
                    return quote(list);
                }
                if (strcmp(name, "car") == 0) {
                    return car(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "cdr") == 0) {
                    return cdr(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "cadr") == 0) {
                    return cdr(list->cellunion.conscell.rest)->cellunion.conscell.first;
                }
                if (strcmp(name, "caddr") == 0) {
                    return cdr(list->cellunion.conscell.rest)->cellunion.conscell.rest->cellunion.conscell.first;
                }
                if (strcmp(name, "cadddr") == 0) {
                    return cdr(list->cellunion.conscell.rest)->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first;
                }
                if (strcmp(name, "caddddr") == 0) {
                    return cdr(list->cellunion.conscell.rest)->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first;
                }
                if (strcmp(name, "list") == 0) {
                    return listFunc(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "symbol?") == 0) {
                    return evals(symbolQ(evals(list->cellunion.conscell.rest, lvl)), lvl + 1);
                }
                if (strcmp(name, "function?") == 0) {
                    return evals(funcQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "number?") == 0) {
                    return evals(numberQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "list?") == 0) {
                    return evals(listQ(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "null?") == 0 || strcmp(name, "not") == 0) {
                    return evals(nullQ(evals(list->cellunion.conscell.rest, lvl)), lvl + 1);
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
                    return numadd(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest, lvl + 2));
                }
                if (strcmp(name, "-") == 0) {
                    return numsub(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2));
                }
                if (strcmp(name, "*") == 0) {
                    return nummul(evals(list->cellunion.conscell.rest->cellunion.conscell.first, lvl + 2), evals(list->cellunion.conscell.rest->cellunion.conscell.rest, lvl + 2));
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
                if (strcmp(name, "and") == 0) {
                    return evals(and(list->cellunion.conscell.rest), lvl + 1);
                }
                if (strcmp(name, "or") == 0) {
                    return or(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "last") == 0) {
                    return last(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "length") == 0) {
                    return length(list->cellunion.conscell.rest);
                }
                if (strcmp(name, "define") == 0) {
                    if (equals(symbolQ(list->cellunion.conscell.rest), t()) == 1 || equals(nullQ(list->cellunion.conscell.rest), t()) == 1) {
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
                return empty();
            }
            // evaluate symbol definitions
            List* nameCell = (List*)malloc(sizeof(List));
            nameCell->isCons = 0;
            nameCell->cellunion.symbol = (char*)malloc(sizeof(char));
            strcpy(nameCell->cellunion.symbol, name);
            
            List* assocResult = assoc(nameCell, asl);
            if (assocResult != NULL) {
                if (assocResult->isCons == 1) {
                    return assocResult->cellunion.conscell.rest->cellunion.conscell.first;
                }
            }
        }
    }
    return list;
}

// recursive function evalution method
List* evalFunc(List* augEnv, List* funcCell, int lvl) {
    List* localFunc = funcCell->cellunion.conscell.rest->cellunion.conscell.first;
    return eval(localFunc, augEnv, fcl, lvl + 1);
}

/****************************************************************
 Function: eval(List* list, List* assocList)
 --------------------
 Evaluate a list from pointer to the start of the list, according
 to the definitions of the functions. Treat "#f" as empty list "()".
 Use the corresponding association list for defintions.
 ****************************************************************/
List* eval(List* list, List* assocList, List* funcList, int lvl)
{
    asl = assocList;
    fcl = funcList;
    List* result = evals(list, lvl);
    // return the lists back to main()
    *assocList = *asl;
    *funcList = *fcl;
    return result;
}