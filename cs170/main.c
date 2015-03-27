//
//  Created by Colin Tan on 2/25/15.
//  Copyright (c) 2015 Colin Tan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int main(int argc, const char* argv[])
{
    startTokens(20);
    printf("A prototype evaluator for Scheme.\nType Scheme expressions using quote,\ncar, cdr, cons and symbol?.\nThe function call (exit) quits.\n");
    printf("\nscheme> ");
    
    while (1) {
        printList(eval(s_expr()));
        fflush(stdin);
        printf("\n\nscheme> ");
    }
}