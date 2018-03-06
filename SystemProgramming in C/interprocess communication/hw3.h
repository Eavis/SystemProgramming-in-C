/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */

#ifndef hw3_h
#define hw3_h
#define KEY_MSQ (key_t) 76541  //key for message queue
#define KEY_SHM (key_t) 67541  //key for shared memory
#define ADD_process 1
#define limit 33554432
#define found_perfect 2
#define reportInfo 3

typedef struct msgque
{
    long type;   //message type
    long data;    //message data
}msgque;

typedef struct process_info
{
    int pid;
    long perfect_no;
    long tested_no;
    long skipped_no;
}process_info;

//array of process structure summarize data on the currently active compute processes
//(pid, number of perfect found, number of candidate tested, number of candidate not tested)

typedef struct shmory
{
    char bitmap[limit/8];
    long perfectnums[20]; //numbers of perfect numbers
    process_info process[21]; //information of processes
}shmory;


#endif /* hw3_h */
