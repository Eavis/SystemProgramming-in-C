/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>
#include "hw3.h"

shmory *shm_space;  //pointer to the shared segment
int shm_id;         //shared memory segment id
pid_t msq_id;         //message queue id
int proc_index;     //process slot index
int start_NO;       //start number of compute
int pid_sent;
msgque msg; //create a msgque struct msg


void terminate_process(int signum){
    shm_space->process[20].perfect_no +=shm_space->process[proc_index].perfect_no;
    shm_space->process[20].tested_no +=shm_space->process[proc_index].tested_no;
    shm_space->process[20].skipped_no +=shm_space->process[proc_index].skipped_no;
//reset the current table
    shm_space->process[proc_index].pid = 0;
    shm_space->process[proc_index].perfect_no = 0;
    shm_space->process[proc_index].tested_no = 0;
    shm_space->process[proc_index].skipped_no = 0;
    
//    if(kill(shm_space->process[proc_index].pid,SIGINT)!=0){
//        fprintf(stderr,"process termination failed:%s\n",strerror(errno));
//        exit(1);
//    }
    exit(0);
}


int testBitmap( char B[ ],  int m )
{
    return ((B[m/8] & (1 << (m%8))) != 0);
}


void  setBit( char B[ ],  int m )
{
    B[m/8] |= (1 << (m%8));
}

/*
 the main function takes an argument from the command line and the format is "./compute startNO &" &means run in the background.
 main method attaches itself to the shared memory
 send message to manage to register allocating a new process
 compute perfect numbers
 and send numbers (perfect#, tested# skipped #)back to shared memory
 */
int main(int argc, char *argv[]){
    
    
    int compute_no = start_NO;
    
   
    if(argc < 2){
        fprintf(stderr,"initializaiton failed:%s\n",strerror(errno));
        exit(1);
    }else{
        start_NO= atoi(argv[1]);//begin from this number
    }
    if (compute_no > limit) {
        printf("The number is out of compute boundary\n");
        exit(3);
    }
    
    struct sigaction sigact;
    sigact.sa_handler = terminate_process;
    if(sigaction(SIGINT,&sigact,NULL)!=0){
        fprintf(stderr,"INTR call failed:%s\n",strerror(errno));
        exit(1);
    }
    if(sigaction(SIGQUIT,&sigact,NULL)!=0){
        fprintf(stderr,"SIGQUIT call failed:%s\n",strerror(errno));
        exit(1);
    }
    if(sigaction(SIGHUP,&sigact,NULL)!=0){
        fprintf(stderr,"SIGHUP call failed:%s\n",strerror(errno));
        exit(1);
    }
    
    /*create shared memory in case compute run before manage*/
    //first use shmget to create shared memory
    shm_id=shmget(KEY_SHM,sizeof(shmory), IPC_CREAT|0660);
    if(shm_id<0){
        fprintf(stderr,"shared memory creation failed:%s\n",strerror(errno));
        exit(1);
    }
    //second map it to the address space
    shm_space = shmat(shm_id,0,0);
    if(shm_space == (void*)-1){
        fprintf(stderr,"share memory address match failed: %s\n",strerror(errno));
        exit(1);
        
    }
    
    //set message queque
    msq_id = msgget(KEY_MSQ,IPC_CREAT|0660);
    if(msq_id<0){
        fprintf(stderr,"message queue creation failed:%s\n",strerror(errno));
        exit(1);
    }
    
    
    //    int pid_sent;
    //get your pid to manage to register a new process
    pid_sent = getpid();
    printf("pid_sent: %d\n",pid_sent);
    //send info to manage with "type 1 =Add_process"
    msg.type = ADD_process;
    msg.data = pid_sent;//store the pid of the register process
    if(msgsnd(msq_id,&msg,sizeof(msg.data),0)==-1){
        fprintf(stderr,"message queue info sent failed:%s\n",strerror(errno));
        exit(1);
    }
    /* wait for response from manage,which is the index of your process in the slot */
    /* only receive the message if the type matches your pid*/
    if(msgrcv(msq_id, &msg,sizeof(msg.data),pid_sent, 0)==-1){
        fprintf(stderr,"message queue info receive failed:%s\n",strerror(errno));
        exit(1);
    }
    
    proc_index = (int)msg.data;//data from manage which is an index
    //initialize the memory slot for new process
    shm_space->process[proc_index].pid = pid_sent;
    shm_space->process[proc_index].perfect_no=0;
    shm_space->process[proc_index].skipped_no=0;
    shm_space->process[proc_index].tested_no=0;
    
    
    int j;		/*loop counter */
    for (j=compute_no; j<=limit; j++) {
        
        if(testBitmap(shm_space->bitmap, j) == 0) { //Test the bit
            setBit(shm_space->bitmap, j); //set the bit
            if (j==1) {
                continue;
            }
            shm_space->process[proc_index].tested_no++;
            //Test the number for perfect Number

            int i;
            int sum=1;
            for (i=2;i<j;i++) {
                if (!(j%i)) sum+=i;
            }
            if (sum==j && j!=1) {
                shm_space->process[proc_index].perfect_no++;
                msg.type = 2;
                msg.data = (long)j;
                msgsnd(msq_id,&msg,sizeof(msg.data),0);
            }
        }else{
            shm_space->process[proc_index].skipped_no++;
        }
        
    }
    for (j= 1 ; j < compute_no; j++) { //wrap around
        if(testBitmap(shm_space->bitmap, j) == 0) { //Test the bit
            setBit(shm_space->bitmap, j); //set the bit
            //Test the number for perfect Number
            if (j==1) {
                continue;
            }
            shm_space->process[proc_index].tested_no++;
            int i;
            int sum=1;
            for (i=2;i<j;i++) {
                if (!(j%i)) sum+=i;
            }
            if (sum==j && j!=1) {
                shm_space->process[proc_index].perfect_no++;
                msg.type = 2;
                msg.data = (long)j;
                msgsnd(msq_id,&msg,sizeof(msg.data),0);
            }
        }else{
            shm_space->process[proc_index].skipped_no++;
        }
        
    }
    kill(getpid(), SIGINT);
    return 0;
}
