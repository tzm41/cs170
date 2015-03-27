//
//  main.c
//  test
//
//  Created by Colin Tan on 2/25/15.
//  Copyright (c) 2015 Colin Tan. All rights reserved.
//

#include <stdio.h>

int main() {
    void left(int *p,int n);
    int num[1000],n,i;
    printf("how many people?");
    scanf("%d",&n);
    for(i=0;i<n;i++)
        num[i]=i+1;
    left(num,n);
    for(i=0;i<n;i++)
        if(num[i]!=0)
            printf("No. which lest last if %d",num[i]);
}

void left(int *p,int n) {
    int i,out,count;
    i=0;
    out=0;
    count=0;
    while(out<n-1) {
        if(*(p+i)!=0)
            count++;
        if(count==7) {
            *(p+i)=0;
            count=0;
            out++;
        }
        i++;
        if(i==n) i=0;  
    }
}
