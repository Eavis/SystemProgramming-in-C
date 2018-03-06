/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */
/* Program to demonstrate server side of TCP communication */
/* There is one argument, the TCP port number to bind to */
/* This deemo illustrates using the POLL system call to monitor
 several fd's at one time in a single process */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "hwk4.h"
#include <time.h>
#include <pthread.h>


struct pollfd pollarray[21];     /* up to 21 simultaneous connections*/
//rangeinfo* rangelist[21];
// host_name_struct *host_name_for_send;


int tt;
int temp;

int thread_idx;
// uint32_t temp_rd;
int type;
int kill_manage;
void terminate_process();
int rept_fd;
int rept_idx;
int mark_terminate;
int cnt_compute;
int test_manage;
int temp_rd;
int temp_wt;
int main (int argc, char* argv[]){

    host_name_struct host_name_ss;
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = terminate_process;
    char send_hostname[256];
    int spec_for_send = 0;
    // host_name_for_send = malloc(sizeof(host_name_struct));
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
    
    int s; //socket descriptor
    int  fd, num, count;
    socklen_t len;
    struct hostent *hostentp;
    struct sockaddr_in sin; /* structure for socket address */
    int i;
    int prev_start;
    prev_start = 2;
    perfectinfo perfectinfo[20];
    host_name_struct host_name_for_send[1];
    // rangeinfo *temp_head;
    int str_len = 0;
    rangeinfo* temp_head;
    temp_head = NULL;
    rangeinfo* ptr;
    int perfect_idx = 0;
    /* set up IP addr and port number for bind */
    sin.sin_addr.s_addr= INADDR_ANY;
    sin.sin_family= AF_INET;
    sin.sin_port = htons(atoi(argv[1]));
    /* Get an internet socket for stream connections */
    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("Socket");
        exit(1);
    }
    
    /* Do the actual bind */
    if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
        perror("bind");
        exit(2);
    }
    
    /* Allow a connection queue for up to 21 Computes */
    listen(s,21); // use the first one for manage 
    //the other 20 for compute
    
    /* Initialize the pollarray */
    pollarray[0].fd=s;     /* Accept Socket*/
    pollarray[0].events=POLLIN;
    /* 20 possible compute's */
    for (i=1; i <= 20 ; i++) {pollarray[i].fd=-1;pollarray[i].events=POLLIN;}
    char computeName[21][256]; // write name from 1 - 20
//    memset(computeName,0, sizeof(computeName));
    
    while(1) {
        poll(pollarray,21,-1);   /* no timeout, blocks until some activity*/
        
        /* Check first for new connection */
        if (pollarray[0].revents & POLLIN) {
            len=sizeof(sin);
            if ((fd= accept (s, (struct sockaddr *) &sin, &len)) <0) {
                perror ("accept");
                exit(3);
            }
            /* Find first available slot for new john */
            for (i=1;i<=20;i++) if (pollarray[i].fd == -1) break;
            pollarray[i].fd=fd; //use the new fd to communicate
            printf("Manage ready for listening process %d\n", i);
            hostentp=gethostbyaddr((char *)&sin.sin_addr.s_addr, sizeof(sin.sin_addr.s_addr),AF_INET);

            strcpy((char *)&computeName[i][0], hostentp->h_name);
           

        }
        
        /* If there are no new connections, process waiting john's */


        else for (i=1;i<=20;i++) {
            if ((pollarray[i].fd !=-1) && pollarray[i].revents) {
                count = read(pollarray[i].fd, &num, 4);
                type = ntohl(num);
                if (count != 4) {
                    // printf("compute request for manage is done for now\n");
                    close(pollarray[i].fd);
                    pollarray[i].fd = -1;
                    memset(computeName[i], 0, sizeof(computeName[i]));
                }
                else if (type == ASK_FOR_RANGE){
                    //the first time compute ask for a range
                    //so give a small range like 5000
                    int range = 5000;
                    read(pollarray[i].fd, &temp, sizeof(int));
                    thread_idx = ntohl(temp);
                    printf("Threads for compute %d with %d threads is going to get range\n",i, thread_idx);
                    type = RESPOND_RANGE;
                    ptr = (rangeinfo*)malloc(sizeof(rangeinfo));
                    temp_wt=htonl(type); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    temp_wt=htonl(thread_idx); //send back idx of thread
                    write(pollarray[i].fd,&temp_wt,sizeof(int));

                    temp_wt=htonl(prev_start); //send back type
                    printf("11111111111111111111111manage side prev_start\n: %d", prev_start);



                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    temp_wt=htonl(range); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    // printf("Manage send start %d and range %d sent back to compute %s, thread %d.\n",prev_start, range, computeName[i], thread_idx);
                    ptr->start = prev_start; //ptr point to new start
                    ptr->range = range; 
                    strcpy(ptr->hostname, (char *)&computeName[i][0]);

                

                    ptr->next = temp_head;
                    temp_head = ptr;
                    prev_start = prev_start + range + 1;
                    // printf("ptr->start = %d, ptr->range = %d. \n", ptr->start, ptr->range);
                }else if (type == FINISH_RANGE){
                    // read(readfd, netNum, sizeof(int));
                    // *netNum = ntohl(*netNum);
                    int idx, time_in_sec, start_finish, range_finish, new_range;
                    read(pollarray[i].fd, &idx, sizeof(int)); //read index of the thread
                    idx = ntohl(idx); //read index
                    read(pollarray[i].fd, &start_finish, sizeof(int));
                    start_finish = ntohl(start_finish);//read start_finish
                    read(pollarray[i].fd, &time_in_sec, sizeof(int));

                    time_in_sec = ntohl(time_in_sec);//read time
                    // printf("!@!!!!!!!!!!!!!!!!!!manage received time!!!!!!!!!!!!!!!!!!!!!: %d\n", time_in_sec);
                    // printf("Thread %d of host %s finished the range starting at %d with %d seconds.\n", idx,computeName[i],start_finish,time_in_sec);
                    rangeinfo* prev;
                    ptr = temp_head;
                    prev = NULL;
                    while (ptr!= NULL){
                        // printf("ptr->start = %d, ptr->range = %d\n", ptr->start, ptr->range);
                        if (ptr->start == start_finish) {  /* Found it. */
                            range_finish = ptr->range;
                            if (prev == NULL) {
                                temp_head = ptr->next;
                            } else {
                                prev->next = ptr->next;
                            }
                            free(ptr);
                            ptr = NULL;
                        }else {
                            prev = ptr;
                            ptr= ptr->next;
                        }
                    }
                    double multi;
                    if (time_in_sec == 0) {
                        int tp = 1;
                        multi = ((double)15)/((double)tp);
                    }else{
                        multi = ((double)15)/((double)time_in_sec);
                    }

                    // printf("15 seconds is %lf times of the last calculation.\n", multi);
                    // printf("range_finish: %d.\n", range_finish);
                    new_range =(int)((double)range_finish * multi);

                    int new_time = (new_range * time_in_sec/range_finish);
                    // printf("Estimated calculation time for new range %d is %d seconds.\n",new_range, new_time);
                    
                    //4. send the new range
                    type = RESPOND_RANGE;
                    temp_wt=htonl(type); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    temp_wt=htonl(idx); //send back numofthread
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    temp_wt=htonl(prev_start); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                    temp_wt=htonl(new_range); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int));
                   
                    //store the new range to the linked list
                    ptr = (rangeinfo*)malloc(sizeof(rangeinfo));
                    ptr-> start = prev_start;
                    ptr-> range = new_range;
                    strcpy(ptr->hostname, (char *)&computeName[i][0]);
                    ptr->next = temp_head;
                    // printf("Add: pt->start = %d, pt->range = %d. \n", pt-> start, pt-> range);
                    temp_head = ptr;
                    
                    prev_start = prev_start + new_range + 1;
                    printf("New range with start %d and range %d was sent back.\n",prev_start,new_range);
                    
                }else if  (type == GET_PERFECT){
                    read(pollarray[i].fd, &temp_rd, sizeof(int));
                    perfectinfo[perfect_idx].perfect = ntohl(temp_rd); // in idx
                    strcpy(perfectinfo[perfect_idx].hostname, (char *)&computeName[i][0]);
                    // printf("Perfect number %d found in host %s.\n",perfectinfo[perfect_idx].perfect, perfectinfo[perfect_idx].hostname);
                    perfect_idx++;
                }else if (type == COMPUTE_SEND_CURRENT){
                    
                    int thread_cnt = 0 , tested = 0, curt_head = 0 , curt_end = 0;


                    // memset(temp_rd, 0, sizeof(int));
                    // memset(temp_wt, 0, sizeof(int));

                    temp_rd = 0;
                    temp_wt = 0;

                    read(pollarray[i].fd, &temp_rd, sizeof(int));
                    thread_cnt = ntohl(temp_rd);
                    type = MANAGE_SEND_CURT_TO_REPORT;
                    temp_wt=htonl(type); //send back type
                    write(rept_fd,&temp_wt,sizeof(int));
                    
                    write(rept_fd,(computeName+i), 256*sizeof(char));
                    printf("Host name %s sent.\n", *(computeName+i));
                    // strcpy(send_hostname, (char *)&computeName[i][0]);
                    // write(rept_fd, send_hostname, 256 *sizeof(char));
                    temp_rd = 0;
                    temp_wt = 0;

                    temp_wt=htonl(thread_cnt); //send back thread_cnt
                    write(rept_fd,&temp_wt,sizeof(int));

                    temp_rd = 0;
                    temp_wt = 0;
                    printf("check!!!!Manage !!!Sent the number of threads = %d in this host \n", thread_cnt);
                    int l;

                    for (l = 0; l < thread_cnt; l++) {
                    	// printf("thread_cont:: %d\n", thread_cnt);
                        read(pollarray[i].fd, &temp_rd, sizeof(int));
                        tested = ntohl(temp_rd);
                        printf("manage side curt_tested: %d\n",tested);

                        temp_wt=htonl(tested); //send back type
                        write(rept_fd,&temp_wt,sizeof(tested));

                        tested = 0;
                        temp_rd = 0;
                        temp_wt = 0;



                        read(pollarray[i].fd, &temp_rd, sizeof(int));
                        curt_head = ntohl(temp_rd);
                        printf("manage side curt_start: %d\n",curt_head);

                        temp_wt=htonl(curt_head); //send back type
                        write(rept_fd,&temp_wt,sizeof(curt_head));

                        curt_head = 0;
                        temp_rd = 0;
                        temp_wt = 0;




                        read(pollarray[i].fd, &temp_rd, sizeof(int));
                        curt_end = ntohl(temp_rd);

                        temp_wt=htonl(curt_end); //send back type
                        write(rept_fd,&temp_wt,sizeof(curt_end));

                        temp_rd = 0;
                        temp_wt = 0;
                        curt_end = 0;

                        
                        
                        printf("manage side end: %d\n",curt_end);
                        // printf("manage receives these numbers from compute check:: tested: %d.  curt_head:  %d curt_end : %d", tested, curt_head,curt_end);
                    }
                }else if (type == RANGE_TERMINATE) { //compute's send send a specific thread termiantion
                    printf("manage received compute range terminate message \n");
                    int tm_start; 
                    int tm_range;
                    int iidx;
                    read(pollarray[i].fd, &temp_rd, sizeof(int));
                    iidx = ntohl(temp_rd);
                    read(pollarray[i].fd, &temp_rd, sizeof(int));
                    tm_start = ntohl(temp_rd);
                    rangeinfo* prev;
                    ptr = temp_head;
                    prev = NULL;
                    while (ptr!= NULL){
                        // printf("ptr->start = %d, ptr->range = %d\n", ptr->start, ptr->range);
                        if (ptr->start == tm_start) {  /* Found it. */
                            tm_range = ptr->range;
                            if (prev == NULL) {
                                temp_head = ptr->next;
                            } else {
                                prev->next = ptr->next;
                            }
                            free(ptr);
                            ptr = NULL;
                        }else {
                            prev = ptr;
                            ptr= ptr->next;
                        }
                    }
                    type = UPDATE_TERMINATION;
                    temp_wt=htonl(type); //send back type
                    write(pollarray[i].fd,&temp_wt,sizeof(int)); 
                    // if(mark_terminate == 1 && temp_head == NULL){
                    if(mark_terminate == 1 && temp_head == NULL){
                        printf("Fianl Attention::All threads has terminated , manage will wait 5 seconds to terminate...\n");
                        sleep(5);//make sure the last msg is sent
                        int nn;
                        for (nn =1;nn <=20;nn++) {

                            if (pollarray[nn].fd != -1) {
                                printf("l: ::::::: %d\n", nn);
                                close(pollarray[nn].fd);
                                pollarray[nn].fd = -1;
                            }
                        }
                        close(pollarray[0].fd);
                        pollarray[0].fd = -1;
                        printf("EXIT NOW.\n");
                        close(s);
                        exit(0);
                         // _exit(1);
                    }
 
                }else if (type == REQUEST_TERMINATE){
                    //manage receive the termination from request and then 
                    //send 
                    printf("Manage received request from report to terminate: Preparing: \n");
                    mark_terminate = 1;
                    type = TERMINATE_FROM_MANAGE_TO_COMPUTE;
                    int l;
                    for (l = 1; l < rept_idx; l++) {
                        if (pollarray[l].fd != -1) {
                            printf("manage send terminate to %d compute\n", l);
                            temp_wt=htonl(type); //send back type
                            write(pollarray[l].fd,&temp_wt,sizeof(int)); 
                        }
                    }
                }else if (type == SEND_REQUEST) {//1manage receive request from report
                    // printf("really?????no send_request info????\n");
                    cnt_compute = 0;
                    // printf("1. manage receives request \n");
                    rept_idx = i;
                    rept_fd = pollarray[i].fd;
                    printf("manage first set_ rept_fd: %d\n",rept_fd);
                    int l;

                    for (l = 1; l <= 20; l++) {
                        if((pollarray[l].fd!= -1) && l != i){
                           
                            cnt_compute++;
                            
                        } 
                    }
                    printf("!!!! :::::::manage knows how many compute alive %d\n", cnt_compute);
                    type = MANAGE_WILL_SEND_PERFECT_TO_REPORT;//manage 

                    temp_wt=htonl(type); //send back type
                    write(rept_fd,&temp_wt,sizeof(int)); 

                    temp_wt=htonl(cnt_compute); //cnt_comupte
                    write(rept_fd,&temp_wt,sizeof(int)); 
                    // test_manage = 22;
                    // temp_wt=ntohl(test_manage); //cnt_comupte
                    // write(rept_fd,&temp_wt,sizeof(int)); 

                    temp_wt = htonl(perfect_idx);
                    write(rept_fd,&temp_wt,sizeof(int));
                    printf("manage kows how many perfect numbers: %d", perfect_idx);

                    for (l = 0; l < perfect_idx; l++) {
                        write(rept_fd,(perfectinfo+l), sizeof(perfectinfo));

                    }

                    //send perfect number and then !!
                    for (l = 1; l <= 20; l++) {
                        if((pollarray[l].fd!= -1) && l != i){
                            // type = REQUEST_FOR_CURRENT;//manage tell compute to send info on working
                            type = MANAGE_ASK_INFO_FROM_COMPUTE;
                            temp_wt=htonl(type); //send back type
                            write(pollarray[l].fd,&temp_wt,sizeof(int)); 
                        }
                    }
                }
            }
        }
    }
}

void terminate_process(int signum) {
    printf("Terminating process:  \n");
    mark_terminate = 1;
    int i;
    for (i = 1; i <= 20; i++) {
        if(pollarray[i].fd!=-1){
            type = TERMINATE_FROM_MANAGE_TO_COMPUTE;
            temp_wt=(type); //send back type
            write(pollarray[i].fd,&temp_wt,sizeof(int));
            // printf("manage send termination info to pollarr %d\n", pollarray[i].fd);
        }
    }//after killing all computes sleep 5 sec and exit;
    kill_manage = 1;
}


