#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "mem.h"

int isInit = 0;
big_block_manage *big_manage;
small_block_manage *small_manage;
struct heap_add_node *p_head = NULL;


SEM_ID sem_small;
SEM_ID sem_big;
SEM_ID sem_heap;

/* calculate the begining address of all the structs*/
void cal_mem(int *base_mem)
{
    int index = 0;
    small_block *small_temp_block;
    big_block *big_temp_block;

    small_manage = (small_block_manage*)base_mem; /* base memory address is small_manage's address */
    big_manage = (big_block_manage*)(small_manage + 1); /* to get big_manage's address  */
    small_temp_block = (small_block*)(big_manage + 1); /* the first small block's address*/
    big_temp_block = (big_block*)(small_temp_block + SMALLBLOCKNUM); /* the first big block's address*/

    /* init small block management */
    small_manage->base_add = small_temp_block;
    small_manage->used_num = 0;
    for(index=0;index<SMALLBLOCKNUM;index++)
    {
        small_manage->status[index] = BLOCKAVAILABLE;
    }

    /* init smabigll block management */
    big_manage->base_add = big_temp_block;
    big_manage->used_num = 0;
    for(index=0;index<BIGBLOCKNUM;index++)
    {
        big_manage->status[index] = BLOCKAVAILABLE;
    }

    /* print the memory information */
    printf("--------------------------\n");
    printf("small_manage begining address:%d\n",small_manage);
    printf("big_manage begining address:%d\n",big_manage);
    printf("small_block begining address:%d\n",small_manage->base_add);
    printf("big_block begining address:%d\n",big_manage->base_add);
    printf("--------------------------\n");
}





/* ...
    init memory
    return 1 or 0(success or fail)
    init_mem mean the allocated meomery begining address
... */
int mem_init(int *init_mem) /* has inited:return 0; fail:-1;success:1*/
{
    int small_size;
    int big_size;
    int manage_size;
    int pre_alloc_size;
    /* if memory is inited or not*/
    if(isInit==1)
    {
        
        return 0;
    }

    /* calculate the memory size need */
    small_size = sizeof(small_block) * SMALLBLOCKNUM; /* the memory size of all small sizes */
    big_size = sizeof(big_block) * BIGBLOCKNUM; /* the memory size of all big sizes */
    manage_size = sizeof(small_block_manage) + sizeof(big_block_manage); /* the memory size of all manage structs*/
    pre_alloc_size = small_size + big_size + manage_size; /* all memory need */

    /* init the semaphore */
    sem_small = semBCreate(SEM_Q_FIFO,SEM_FULL);   
    sem_big = semBCreate(SEM_Q_FIFO,SEM_FULL);
    sem_heap = semBCreate(SEM_Q_FIFO,SEM_FULL);
    
    printf("----------------------\n");
    printf("small_size:%dbytes small_num:%d.\n",SMALLBLOCKSIZE,SMALLBLOCKNUM);	
    printf("big_size:%dbytes big_num:%d.\n",BIGBLOCKSIZE,BIGBLOCKNUM);	
    printf("small blocks use %d bytes.\n",small_size);
    printf("big blocks use %d bytes.\n",big_size);
    printf("manage struct use %d bytes.\n",manage_size);
    printf("small manage struct use %d bytes.\n",sizeof(small_block_manage));

    printf("pre_alloc_size is %d bytes.\n",pre_alloc_size);
    printf("----------------------\n");
    /* init the link to manage the memory(>BIGBLOCKSIZE)*/
    p_head = (struct heap_add_node *)malloc(sizeof(heap_add_node));
    p_head->next_node = NULL;
    p_head->size = 0;
    p_head->heap_add = NULL;
    
    init_mem = (int *)malloc(pre_alloc_size);
    if(init_mem == NULL)
    {
        
        return -1;
    }else{
        isInit = 1;
        printf("init memory succeed!\n");
        printf("the inited memoryed is begining at %d.\n",init_mem);
        cal_mem(init_mem);
        return 1;
    }
} 

/* malloc memory return the allocated memory begining address*/
int* malloc_mem_type(int size,int type)
{
    int index = 0;
    int *heap_add;

    heap_add_node *new_node;

    small_block *first_small = small_manage->base_add ;

    big_block *first_big = big_manage->base_add;

    switch(type){
        case SMALLBLOCK: 
            printf("requesting a small_block......\n");
            /* if all small blocks are busy or not*/
            if(small_manage->used_num==SMALLBLOCKNUM){
                printf("request small_block failed,no available small_block,retry to request a big_block.\n"); 
                return malloc_mem_type(size,BIGBLOCK); /* request a big block*/
            }

            semTake(sem_small,WAIT_FOREVER);
            small_manage->used_num++;
            /* find a block to use*/
            for(index=0;index<SMALLBLOCKNUM;index++)
            {
                if(small_manage->status[index]==BLOCKAVAILABLE){
                    
                    small_manage->status[index] = BLOCKBUSY;
                    semGive(sem_small);
                    printf("request a small_block succeeed!\n");
                    return (int*)(first_small + index);
                    
                }
            }
            semGive(sem_small);
            /* printf("request small_block failed,retry to request a big_block"); */
            /* return malloc_mem_type(size,BIGBLOCK);*/
            break;

        case BIGBLOCK:

            printf("requesting a big_block......\n");
            /* if all big blocks are busy or not*/
            if(big_manage->used_num==BIGBLOCKNUM){
                    printf("request big_block failed,no available big_block,retry to request heap memory.\n"); 
                    return malloc_mem_type(size,HEAP); /* request heap memory */
            }
            semTake(sem_big,WAIT_FOREVER);
            big_manage->used_num++;
            /* find a block to use*/
            for(index=0;index<BIGBLOCKNUM;index++)
            {
                
                if(big_manage->status[index]==BLOCKAVAILABLE){
                    
                    big_manage->status[index] = BLOCKBUSY;
                    semGive(sem_big);
                    printf("request a big_block succeed!\n");
                    return (int*)(first_big + index);
                }
            }
            semGive(sem_big);
            /* printf("request big_block failed,retry to request heap memory"); */
            /* return malloc_mem_type(size,HEAP);*/
            break;

        case HEAP:

            printf("requesting heap memory......\n");
            semTake(sem_heap,WAIT_FOREVER);

            new_node = (heap_add_node*)malloc(sizeof(heap_add_node)); /* init a heap manage block*/
            if(new_node == NULL){
                printf("request heap memory failed!\n");
                semGive(sem_heap);
                return NULL;
            }

            heap_add = (int*)malloc(size); /* request heap memory */

            if(heap_add==NULL){
                printf("request heap memory failed!\n");
                semGive(sem_heap);
                free(new_node); 
                return NULL;
            }
            new_node->heap_add = heap_add;
            new_node->size = size;

            new_node->next_node = p_head->next_node; /* insert into the head of the link*/
            p_head->next_node = new_node;

            semGive(sem_heap);
            printf("request heap memory succeed!\n");
            return heap_add;
        default:
            break;
    }


}

/* malloc the memory according to the size */
int* malloc_mem(int size)
{
    
    if(size>0&&size<=SMALLBLOCKSIZE){
        return malloc_mem_type(size,SMALLBLOCK);
    }else if(size<=BIGBLOCKSIZE){
        return malloc_mem_type(size,BIGBLOCK);
    }else{
        return malloc_mem_type(size,HEAP);
    }

}

/* from the memory*/
void free_mem(int *free_add)
{
    /* fugure out the memory belongs to which block*/
    if(free_add >= (int*)(small_manage->base_add) && free_add <= (int*)(small_manage->base_add + SMALLBLOCKNUM - 1)){ /* small block */
        small_block *first_block = small_manage->base_add;
        int index = 0;
        for(index = 0;index<SMALLBLOCKNUM;index++){ /* find the small blcok */
            if((int*)(small_manage->base_add + index) == free_add){
                small_manage->used_num--;
                small_manage->status[index] = BLOCKAVAILABLE;
                printf("free a small block success!\n");
                return;
            }
        }

    }else if(free_add >= (int*)(big_manage->base_add) && free_add <= (int*)(big_manage->base_add + BIGBLOCKNUM - 1)){ /* big block */

        big_block *first_block = big_manage->base_add;
        int index = 0;
        for(index = 0;index<BIGBLOCKNUM;index++){ /* find the big blcok */
            if((int*)(big_manage->base_add) + index == free_add){
                big_manage->used_num--;
                big_manage->status[index] = BLOCKAVAILABLE;
                printf("free a big block success!\n");
                return;
            }
        }
    }else if(p_head->next_node!=NULL){ /* heap */
        /* double node to traverse the link */
        heap_add_node *p = p_head; 
        heap_add_node *temp_p = NULL;
        /* if heap mermory is allocated */
        while(p->next_node!=NULL){
            temp_p = p->next_node;

            if(temp_p->heap_add == free_add){ /* if the heap_add equals the free_add */
                free(free_add); /* free the heap*/
                p->next_node = temp_p->next_node;
                free(temp_p); /* free the node */
                printf("free a block memory of heap success!\n");
                return;
            }else{
                p = p->next_node;
            }
        }
    }
    
}

/* traverse the heap management list*/
void traverse_list()
{
    heap_add_node *p = p_head->next_node;
    if(p==NULL){
        printf("heap management list is empty!\n");
    }else{
        printf("the memory mallocd from heap:\n");
        while(p!=NULL){
            printf("allocated memory begin at address:%d,size%d.\n",p->heap_add,p->size);
            p = p->next_node;
        }
    }

}

/* free the heap management list*/
void free_list(){
    heap_add_node *p = p_head;
    heap_add_node *temp_p = p_head->next_node;
    if(temp_p==NULL){
        printf("heap management list is empty!\n");
    }else{
        while(temp_p!=NULL){
            free(temp_p->heap_add);
            p->next_node = temp_p->next_node;
            free(temp_p);
            temp_p = p->next_node;
            printf("free a block of heap!\n");
        }
    }
}

void show_mem()
{
    double f1,f2;
    int index;
    if(isInit == 0){
        printf("memory is not inited!\n");
        return;
    }else{
        printf("\n ===========the memory status==========\n");
        f1 = (double)small_manage->used_num/(double)SMALLBLOCKNUM*100;
        printf("\n small block:%d,used:%d,rate of use:%.2f\n",SMALLBLOCKNUM,small_manage->used_num,f1);
        for(index = 0;index<SMALLBLOCKNUM;index++){
            printf("small block%d:",index);
            if(small_manage->status[index] == BLOCKBUSY){
                printf("busy\n");
            }
            if(small_manage->status[index]== BLOCKAVAILABLE){
                printf("available\n");
            }
        }

        f2 = (double)big_manage->used_num/(double)BIGBLOCKNUM*100;
        printf("\n big block:%d,used:%d,rate of use:%.2f\n",BIGBLOCKNUM,big_manage->used_num,f2);
        for(index = 0;index<BIGBLOCKNUM;index++){
            printf("big block%d:",index);
            if(big_manage->status[index] == BLOCKBUSY){
                printf("busy\n");
            }
            if(big_manage->status[index]== BLOCKAVAILABLE){
                printf("available\n");
            }
        }
        printf("\n");
        traverse_list();
        printf("================================================\n");
    }



}