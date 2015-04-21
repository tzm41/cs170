//
//  evaler.h
//
//  Created by Colin Tan on 4/20/15.
//  Copyright (c) 2015 Colin Tan. All rights reserved.
//

#ifndef EVALER
#define EVALER

#include <stdio.h>

struct Cell;

typedef struct Cell List;

List* eval(List* list, List* assocList, List* funcList);

#endif /* defined(__cs170__evaler__) */
