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
/*
 Manage creates the shared memory 
 manipulate compute process
 manage set up a single queue for compute and report-k
 manage only receives message with type 1-3
 1-manage find an empty slot for new compute process in the sharedd memory
 2-indicate that compute find a perfect# manage updates the perfect# array,this time manage doesn't send message back
 3-message received from report indicating kill all the process
 It calls the terminate handler
 */
shmory *shm_space;
int shm_id;
int msq_id;
int proc_index;
int manage_pid;
int numOfprocess;

void terminate_process(int signum){
    int pid;
    int i = 0;
    while( i <= 19 ){ // kill all the process include the manage
        if((pid = shm_space->process[i].pid) > 0){
            if(kill(pid,SIGINT) == -1){
                fprintf(stderr,"close compute failed:%s\n",strerror(errno));
                exit(1);
            }else{
                printf("compute: %d has been killed\n",pid);
            }
        }
        i++;
    }

    sleep(5);
    
    if(shmdt((shmory *)shm_space)==-1){
        fprintf(stderr,"close share memory failed:%s\n",strerror(errno));
        exit(1);
    }
    if(shmctl(shm_id,IPC_RMID,0)==-1){
        fprintf(stderr,"close shared memory failed:%s\n",strerror(errno));
        exit(1);
    }
    if(msgctl(msq_id,IPC_RMID,0)==-1){
        fprintf(stderr,"close message queue failed:%s\n",strerror(errno));
        exit(1);
    }
    exit(0);
}

int main(int argc, char *argv[]){

    msgque msg; //create a msgque struct msg
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = terminate_process;  
    //sigaction installed a signal-handling funtion for the SIGINT signal
    //The default action for the SIGINT signal is to terminate the process,
    // after installed the signal handler, sigaction can arrange SIGINT signals to be ignored
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
    memset(shm_space->bitmap,0,sizeof(shm_space->bitmap));
    memset(shm_space->perfectnums,0,sizeof(shm_space->perfectnums));
    memset(shm_space->process,0,sizeof(shm_space->process));
    manage_pid = getpid();
    shm_space->process[20].pid = manage_pid;
    msq_id = msgget(KEY_MSQ, IPC_CREAT|0660);
    if(msq_id<0){
        fprintf(stderr,"message queue creation failed:%s\n",strerror(errno));
        exit(1);
    }
    while(1){
        msgrcv(msq_id,&msg, sizeof(msg.data), -3, 0);
        if(msg.type==ADD_process){
            int i;
            for(i = 0;i <= 19;i++){
                if(shm_space->process[i].pid==0){
                    shm_space->process[i].pid =(int)msg.data;
                    msg.type = msg.data;
                    msg.data = i;//send index back to compute
                //location of new process in the process array
                    if(msgsnd(msq_id,&msg,sizeof(msg.data),0) == -1){
                        fprintf(stderr,"message sent to compute failed:%s\n",strerror(errno));
                        exit(1);
                    }
                    printf("Compute: %ld has been registered\n", msg.data);
                    break;
                }
                if(i==20){
                    fprintf(stderr,"allocate compute space failed:%s\n",strerror(errno));
                    break;
                }
            }
        }else if(msg.type==found_perfect){
            int i;
            for(i = 0; i < 20; i++){
                if(shm_space->perfectnums[i] == msg.data){
                    break;
                }
                if(shm_space->perfectnums[i] == 0){
                    shm_space->perfectnums[i] =(long)msg.data;
                    break;
                }
            }

        }else if(msg.type==reportInfo){
            terminate_process(0);
        }
    }
    return 0;
}
