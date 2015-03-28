//
//  Created by Colin Tan on 2/25/15.
//  Copyright (c) 2015 Colin Tan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int main(int argc, const char * argv[]) {
    printf("A parser for a subset of Scheme.\n");
    printf("Type any Scheme expression and its\n");
    printf("\"parse tree\" will be printed out.\n");
    printf("Type Ctrl-c to quit.\n");
    printf("\nscheme> ");
    
    //char token[20];
    startTokens(20);
    
    while (1) {
        S_Expression();
        printf("\nscheme> ");
    }
}