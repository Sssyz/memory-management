#include <msgQLib.h>
#include "task.h"
#include <taskLib.h>
#include "mem.h"
#include <semaphore.h>
MSG_Q_ID msg;
MSG_Q_ID msg_client1;
MSG_Q_ID msg_client2;
MSG_Q_ID msg_client3;
SEM_ID sem_urgent_msg; /* make urgent msg FIFO*/
/* define the request msg*/
typedef struct{
    char pri; /* msg type 'u' urgent 'n' normal */
    int pid;  /* task identity */
    int size; /* memory need */
    int *mem_add; 
} REQ;
/* define the request msg*/
typedef struct{
    int ack; /* 1 success 0 failure*/
    int *mem_add;  
} RES;

/* server task ptr */
void server()
{

    REQ req;
    int *add;
    RES res;
    add = NULL;
    while(1){
        
            
        msgQReceive(msg,(char*)&req,sizeof(REQ),WAIT_FOREVER);
        /* recognize the msg */
        switch(req.pid){
            case 1:
                if(req.pri=='n'){
                    add = malloc_mem(req.size);
                    /* client1 malloc memory succeed*/
                    if(add!=NULL){
                        res.ack=1;
                        res.mem_add = add;
                        msgQSend(msg_client1,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);
                    }else{ /* fail */
                        res.ack=0;
                        res.mem_add = NULL;
                        msgQSend(msg_client1,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);
                    } 
                }
                break;
            case 2:
                if(req.pri=='u'){
                    add = malloc_mem(req.size);
                    /* client2 malloc memory succeed*/
                    if(add!=NULL){
                        res.ack=1;
                        res.mem_add = add;
                        msgQSend(msg_client2,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);
                    }else{ /* fail */
                        res.ack=0;
                        res.mem_add = NULL;
                        msgQSend(msg_client2,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);

                    }
                    semGive(sem_urgent_msg);
                }
                break;
            case 3:
                if(req.pri=='u'){
                    add = malloc_mem(req.size);
                    /* client2 malloc memory succeed*/
                    if(add!=NULL){
                        res.ack=1;
                        res.mem_add = add;
                        msgQSend(msg_client3,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);
                    }else{ /* fail */
                        res.ack=0;
                        res.mem_add = NULL;
                        msgQSend(msg_client3,(char*)&res,sizeof(RES),WAIT_FOREVER,MSG_PRI_NORMAL);

                    }
                    semGive(sem_urgent_msg); /* give the sem */
                }
                break;
            default:
                break;

        }
    }
    
    
    
}

/* send nomoral msg*/
void client1()
{
    
    REQ req;
    RES res;
    int* add;
    int i;
    i=1;
    add = NULL;
    
    while(i--){
        req.pri = 'n';
        req.pid = 1;
        req.size = 6;
        msgQSend(msg,(char*)&req,sizeof(REQ),WAIT_FOREVER,MSG_PRI_NORMAL);
        msgQReceive(msg_client1,(char*)&res,sizeof(RES),WAIT_FOREVER);
    
        if(res.ack==1){
            add = res.mem_add;
        }else{
            printf("client1 request a memory block failed!");
        }
        
        
       
        
    }
    
    

    
       
}
/* send urgent msg*/
void client2()
{
    REQ req;
    RES res;
    int* add;
    int i;
    i=1;
    add = NULL;
    /* make request msg */
    req.pri = 'u';
    req.pid = 2;
    req.size = 256;
    
    while(i--){
        semTake(sem_urgent_msg,WAIT_FOREVER); /* take the sem */
        msgQSend(msg,(char*)&req,sizeof(REQ),WAIT_FOREVER,MSG_PRI_NORMAL);
        msgQReceive(msg_client2,(char*)&res,sizeof(RES),WAIT_FOREVER);
        if(res.ack==1){
            add = res.mem_add;
        }else{
            printf("client2 request a memory block failed!");
        }
        
        
        
    }
    
}

void client3()
{
    REQ req;
    RES res;
    int* add;
    int i;
    i=1;
    add = NULL;
    req.pri = 'u';
    req.pid = 3;
    req.size = 256;
    
    while(i--){
        semTake(sem_urgent_msg,WAIT_FOREVER); /* take the sem */
        msgQSend(msg,(char*)&req,sizeof(REQ),WAIT_FOREVER,MSG_PRI_NORMAL);
        msgQReceive(msg_client1,(char*)&res,sizeof(RES),WAIT_FOREVER);
        if(res.ack==1){
            add = res.mem_add;
        }else{
            printf("client3 request a memory block failed!");
        }
       
        
        
    }
    
}

/* run all the tasks */
void task_run(){
    int task_server;
    int task_client1;
    int task_client2;
    int task_client3;
    msg = msgQCreate(10,sizeof(REQ),MSG_Q_FIFO); /* create msg for server  */
    msg_client1 = msgQCreate(10,sizeof(RES),MSG_Q_FIFO); /* create msg for client1*/
    msg_client2 = msgQCreate(10,sizeof(RES),MSG_Q_FIFO); /* create msg for client2*/
    msg_client3 = msgQCreate(10,sizeof(RES),MSG_Q_FIFO); /* create msg for client3*/
    sem_urgent_msg= semBCreate(SEM_Q_FIFO,SEM_FULL); /* create the sem for urgent_msg */
    /* create and run task */
    task_client3= taskSpawn("client3",102,0,2000,(FUNCPTR)client3,0,0,0,0,0,0,0,0,0,0);
    task_client2= taskSpawn("client2",101,0,2000,(FUNCPTR)client2,0,0,0,0,0,0,0,0,0,0);
    task_client1= taskSpawn("client1",100,0,2000,(FUNCPTR)client1,0,0,0,0,0,0,0,0,0,0);
    task_server= taskSpawn("server",99,0,2000,(FUNCPTR)server,0,0,0,0,0,0,0,0,0,0);
    printf("client1_pid:%d client2_pid:%d client3_pid:%d server_pid:%d\n",task_client1,task_client2,task_client3,task_server);
    
    
    

}

