#include <stdio.h>
#include "mem.h"
#include "task.h"


/* main */
int main(void)
{
    
    
    int *init_mem = NULL;
    int index = 0;
    int *p[3];
    int status;
    status = mem_init(init_mem);
    if(status==-1)
    {
        printf("init mermory failed!\n");
        
    }else if(status == 0 ){
        printf("mermory has been inited!\n");
    }
    task_run();
   
    /*
    for(index=0;index<=2;index++)
    {
        
        p[index] = malloc_mem(6);
        printf("the memory:%d\n\n",p[index]);
    
    }
    /*
    for(index=0;index<=2;index++)
    {
        
        free_mem(p[index]);
        
    
    }*/
    
    



}