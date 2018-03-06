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
shmory *shm_space;
int shm_id;
int msq_id;
int main(int argc, char *argv[]){
    int i;
    int j;
    int pid;
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
    if((msq_id = msgget(KEY_MSQ,IPC_CREAT|0660))<0){
        fprintf(stderr,"message queue creation failed:%s\n",strerror(errno));
        exit(1);
    }
    printf("The perfect numbers are: ");
    for(i=0;i<=19;i++){
        if(shm_space->perfectnums[i]>0){
            printf(" %ld",shm_space->perfectnums[i]);
        }
    }
    printf("\n");

    long totalTestednum =0;
    long totalSkipednum =0;
    long totalPerfectnum =0;
    for (j=0;j <= 19;j++){
        pid = shm_space->process[j].pid;
        if(pid > 0){
            if(argc == 1) {
                printf("Active Pid %d: ",pid);
                printf("Tested: %ld ", shm_space->process[j].tested_no);
                printf("Found: %ld perfect# , ",shm_space->process[j].perfect_no);
                printf("Skipped: %ld.\n",shm_space->process[j].skipped_no);

            }
            totalTestednum += shm_space->process[j].tested_no;
            totalSkipednum += shm_space->process[j].skipped_no;
            totalPerfectnum += shm_space->process[j].perfect_no;
        }
    }
    totalTestednum += shm_space->process[20].tested_no;
    totalSkipednum += shm_space->process[20].skipped_no;
    totalPerfectnum += shm_space->process[20].perfect_no;
    printf("Total perfect number: %ld",totalPerfectnum);
    printf(" Total tested: %ld",totalTestednum);
    printf(" Total skipped: %ld\n",totalSkipednum);
    if(argc > 1){
        if(strcmp(argv[1],"-k")==0){
            kill(shm_space->process[20].pid, SIGINT);//kill manage itself
        }else {
            fprintf(stderr,"process termination  -k failed: %s\n",strerror(errno));
            exit(1);
        }
    }
    return 0;
}

