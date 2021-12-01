#ifndef _MEM_H_
#define _MEM_H_
#define SMALLBLOCKSIZE 16 /* define the number of bytes used by a smallblock */
#define BIGBLOCKSIZE 256 /* define the number of bytes used by a bigblock */
#define SMALLBLOCKNUM 1 /* define the number of the smallblock */
#define BIGBLOCKNUM 1 /* define the number of the bigblock */

#define BLOCKBUSY 0  /* the status of being used*/
#define BLOCKAVAILABLE 1 /* the status of being available */

#define SMALLBLOCK 1
#define BIGBLOCK 2
#define HEAP 3
/* define small memory block */
typedef struct
{
    char data[SMALLBLOCKSIZE]; /* define the data area(small_block) */
}small_block;

/* define big memory block */
typedef struct
{
    char data[BIGBLOCKSIZE]; /* define the data area(big_block) */
}big_block;

/* define small block management struct */
typedef struct
{
    small_block *base_add; /* small blocks' base address */
    int used_num; /* the number of used small blocks */
    char status[SMALLBLOCKNUM]; /* the status of small block */
}small_block_manage;

/* define big block management struct */
typedef struct
{
    big_block *base_add; /* big blocks' base address */
    int used_num; /* the number of used big blocks */
    char status[BIGBLOCKNUM]; /* the status of big block */
}big_block_manage;

/* 
    if the memory size need bigger than BIGBLOCKSIZE, need get it from heap. 
    define a link list to manage the memory(>BIGBLOCKSIZE) allocated from the heap
*/

/* define the link node */
typedef struct heap_add_node
{
    int * heap_add; /* the address in heap of the mermory */
    int size; /* memory size */
    struct heap_add_node *next_node; /* link to next heap_add_node */
}heap_add_node;

void cal_mem(int* base_mem);
int mem_init(int* init_mem);
void free_mem(int *free_add);
int* malloc_mem_type(int size,int type);
int* malloc_mem(int size);
void free_list();
void traverse_list();
void show_mem();
#endif
