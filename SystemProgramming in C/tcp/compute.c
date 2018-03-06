/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */
/* Demonstration of Client side of TCP with hostnames
    John has 3 args: Mary's hostname
 Mary's port number
 How many numbers to send      */
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hwk4.h"
#include <time.h>
#include <unistd.h>
#include <pthread.h>

//clock_t begin = clock();
//
///* here, do your time-consuming job */
//
//clock_t end = clock();
//double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
pthread_t tid_signal, tid_send, tid_recv;
int num_of_thread; // store the exact thread number
sigset_t signal_set; // signal set for all threads
int signum;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; //mutex for work
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER; //mutex for signal//send//receive
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
int type;
working_info * info_working;
int s;//fd for socket
// = malloc(sizeof(pthread_t)* num_of_thread);
int rtn;
int pending;
int temp_rd;
int temp_wt;
int cnt_term;
computeRespond *compute_reply;
pthread_t * thread_ids;


void *recv_func(){
    // the thread only for receiving info from manage
    while(1){
        rtn = read(s, &temp_rd, sizeof(int));
        pthread_mutex_lock(&mtx2);///////////
        type = ntohl(temp_rd);  
        if (rtn != 4) {
            printf("Compute: No more info from Manage for now\n");
            close(s); // just close the docket//
            exit(1);
        }else if (type == RESPOND_RANGE) {
            int idx;
            // printf("A new range is received: \n");
            read(s, &temp_rd, sizeof(int));
            idx = ntohl(temp_rd);
            
            
            
            read(s, &temp_rd, sizeof(int));
            info_working[idx].start = ntohl(temp_rd);
            
            read(s, &temp_rd, sizeof(int));
            info_working[idx].range = ntohl(temp_rd);

            info_working[idx].haswork = 0;//means has work now
            
            // printf("thread %d received new range with start %d and range %d.\n", idx, info_working[idx].start, info_working[idx].range);
            // info_working[idx].done = 0;
            pthread_cond_signal(&cond2);
            
        }else if (type == MANAGE_ASK_INFO_FROM_COMPUTE){ //
            // printf("request ask for current running info:  \n");
            // printf("5 compute receives request for current from manage\n");
            while(pending) {
                pthread_mutex_unlock(&mtx2);
                pthread_mutex_lock(&mtx2);
            }
            compute_reply->idx = 0;
            // compute_reply->type = GET_PERFECT;
            compute_reply->type = CPT_SEND_CURRENT; //tell comupte to send info
            // printf("inside say hi nnn : %d\n", nn);
    
            pending = 1;
            
            pthread_cond_signal(&cond2);
            
        }else if (type == TERMINATE_FROM_MANAGE_TO_COMPUTE) {

            // printf("Compute process received request from manage to terminate.\n");
            // printf("Request ask for termination now: manage has send to compute\n");
            kill(getpid(), SIGINT);
            pthread_cond_signal(&cond2);
            
        }else if (type == UPDATE_TERMINATION) {
            // printf("Compute received a confimation from mange\n");
            cnt_term++;
            pthread_cond_signal(&cond2);
        }else {
            
            // printf("Error: Information cannot identify: %d", type);
            pthread_cond_signal(&cond);
            
        }
        pthread_mutex_unlock(&mtx2);
    }
}

void *signal_func(){
    rtn = sigwait(&signal_set,&signum);
    // printf("Signal func for compute received signal\n");
    if (rtn != 0) {
        printf("Error: Compute didn't receive the signal. \n");
        exit(1);
    }
    
    int l;
    for(l = 0; l < num_of_thread; l++){
        printf("Compute: signal received, send terminate to each thread.\n");
        info_working[l].terminate = 1;
    }
    
    while(1) {
        if (cnt_term == num_of_thread) {
            printf("compute is terminating all the threads.cnt_term == num_of_thread..\n");
            for (l = 0; l < num_of_thread; l++){
                pthread_join(((pthread_t)thread_ids[l]),NULL);
            }
            exit(0);
        }
    }
}

void *send_func(){
    // printf("send_func\n");
    pthread_mutex_lock(&mtx2);
    while(1) {
        
        while(pending == 0) {
            pthread_cond_wait(&cond2, &mtx2);
        }
        if (compute_reply->type == CPT_ASK_RANGE) {
            type = ASK_FOR_RANGE;
            temp_wt=htonl(type); //send back type
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(compute_reply->idx); //send back index
            write(s,&temp_wt,sizeof(int));
            
            // printf("Sent a new range request for newly initiated thread %d\n", compute_reply->idx);
            pending=0;
            
        }else if (compute_reply->type == CPT_FINISH_RANGE) {
            
            rtn = compute_reply->idx;
            type = FINISH_RANGE;
            temp_wt=htonl(type); //send back type
            write(s,&temp_wt,sizeof(int));
            temp_wt=htonl(compute_reply->idx); //send back idx of thread
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(info_working[rtn].start); //send back start
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(info_working[rtn].time_in_sec); //send back type
            write(s,&temp_wt,sizeof(int));
            
            // printf("Sent a range request for thread %d with finished start %d and range %d and type %d.\n", compute_reply->idx,info_working[rtn].start,info_working[rtn].range,type );
            pending=0;
            
        }else if (compute_reply->type == CPT_FIND_PERFECT) {
            type = GET_PERFECT;
            // type =CPT_SEND_PERFECT_TO_MANAGE;
            temp_wt=htonl(type); //send back type
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(compute_reply->idx); //send back idx of thread
            write(s,&temp_wt,sizeof(int));
            
            // printf("Perfect Number %d is sent to manage.\n",compute_reply->idx);
            pending=0;
            
        }else if (compute_reply->type == CPT_TERMINATE){
            rtn = compute_reply->idx;
            // printf("thread %d received termination request\n", rtn);
            
            type = RANGE_TERMINATE;
            
            temp_wt=htonl(type); //send back type
            write(s,&temp_wt,sizeof(int));


            temp_wt=htonl(rtn); //send back start
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(info_working[rtn].start); //send back start
            write(s,&temp_wt,sizeof(int));
            
            // printf("start ------- %d --------- idx: %d  IS TERMINATING.\n",info_working[rtn].start,rtn);
            pending=0;
            
        }else if (compute_reply->type == CPT_SEND_CURRENT) {
            // printf("say hi: %d\n", mm);
            rtn = compute_reply->idx;
            type = COMPUTE_SEND_CURRENT;
            temp_wt=htonl(type); //send back type
            write(s,&temp_wt,sizeof(int));
            
            temp_wt=htonl(num_of_thread); 
            write(s,&temp_wt,sizeof(int));
            
            int t;
            int curt_tested = 0 , curt_start = 0 ,end = 0;
            // printf("compute side num_of_thread: $%d\n", num_of_thread);
            for(t = 0; t < num_of_thread; t++) {
                printf("start--->\n");
                curt_tested = info_working[t].current - info_working[t].start;   

                temp_wt=htonl(curt_tested); //send back type
                write(s,&temp_wt,sizeof(int));

                printf("compute side curt_tested: %d\n",curt_tested);
                temp_wt = 0;
                curt_tested = 0;

                printf("compute side origin_start: %d\n",info_working[t].start);
                curt_start = info_working[t].current;  
                temp_wt=htonl(curt_start); //send back type
                write(s,&temp_wt,sizeof(int));
                printf("compute side curt: %d\n",info_working[t].current);

                temp_wt = 0;
                curt_start = 0;

                end = info_working[t].start + info_working[t].range;
                temp_wt=htonl(end); //send back type
                write(s,&temp_wt,sizeof(int));

                end = 0;
                temp_wt = 0;

                printf("compute side end: %d\n",end);

                
               
                
                // printf("compute side origin_range: %d\n",info_working[t].range);
                
                
                printf("end--->\n");

    
                
               
               
            }
            pending = 0;
        }
         
    }
    pthread_mutex_unlock(&mtx2);
}

void *working_func(void *a){
    int cnt, i;
    int end, sum;
    time_t time_begin, time_end;
    pthread_mutex_lock(&mtx);
    int index = *(int*)a;
    printf("Entered thread %d.\n", index);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    cnt = 0;
    int div;
    while(1){

        if(info_working[index].terminate == 1){
            pthread_mutex_lock(&mtx2);
            while(pending) {
                pthread_mutex_unlock(&mtx2);
                pthread_mutex_lock(&mtx2);
            }
            compute_reply->type = CPT_TERMINATE;
            compute_reply->idx = info_working[index].curt_idx;
            pending  = 1;
            pthread_cond_signal(&cond2);
            pthread_mutex_unlock(&mtx2);
            break;
        }

        if (info_working[index].haswork == -1 && cnt ==0) {
            // printf("New working thread: %d ask for a range\n", index);
            // mutxwithSig2(1,WorkMemArry[indx].curThreadNum);
            pthread_mutex_lock(&mtx2);
            while(pending) {
                pthread_mutex_unlock(&mtx2);
                pthread_mutex_lock(&mtx2);
            }
            compute_reply->type = CPT_ASK_RANGE;
            compute_reply->idx = info_working[index].curt_idx;
            pending  = 1;
            
            pthread_cond_signal(&cond2);
            pthread_mutex_unlock(&mtx2);
            
            cnt = 1;
            
        }else if (info_working[index].haswork == 0){ //should count time
            time(&time_begin);
            // printf("-----------start-------------_time:   %d \n", (int)time_begin);
            end = info_working[index].start + info_working[index].range;
            // printf("thread first recieved ")
            for (info_working[index].current = info_working[index].start; info_working[index].current <= end; info_working[index].current++){
                // printf("now->have to check current %d\n",info_working[index].current);
                if (info_working[index].terminate == 1){
                    // printf("Received termination message within calculation loop.\n");
                    // mutxwithSig2(3,WorkMemArry[indx].curThreadNum);//action type 3. terminating, with thread index
                    // printf("thread %d received termination from compute\n", info_working[index].curt_idx);
                    pthread_mutex_lock(&mtx2);
                    while(pending) {
                        pthread_mutex_unlock(&mtx2);
                        pthread_mutex_lock(&mtx2);
                    }
                    compute_reply->type = CPT_TERMINATE;
                    // printf("compute send cpt_terminate for receive\n");
                    compute_reply->idx = info_working[index].curt_idx;
                    pending  = 1;
                    pthread_cond_signal(&cond2);
                    pthread_mutex_unlock(&mtx2);
                    cnt = 1;
                    break;
                }
                
                sum=1;
                for (div = 2;  div < info_working[index].current; div++)
                    if (!(info_working[index].current % div)) sum += div;
                if (sum==info_working[index].current) {
                    // printf("%d is perfect from compute\n",sum);
                    info_working[index].perfect = sum;
                    pthread_mutex_lock(&mtx2);
                    while(pending) {
                        pthread_mutex_unlock(&mtx2);
                        pthread_mutex_lock(&mtx2);
                    }
                    compute_reply->type = CPT_FIND_PERFECT;
                    compute_reply->idx = info_working[index].perfect;
                    pending  = 1;
                    pthread_cond_signal(&cond2);
                    pthread_mutex_unlock(&mtx2);
                }
            }
            // if(info_working[index].terminate == 1 && cnt == 0){
            //     pthread_mutex_lock(&mtx2);
            //     while(pending) {
            //         pthread_mutex_unlock(&mtx2);
            //         pthread_mutex_lock(&mtx2);
            //     }
            //     compute_reply->type = CPT_TERMINATE;
            //     compute_reply->idx = info_working[index].curt_idx;
            //     pending  = 1;
            //     pthread_cond_signal(&cond2);
            //     pthread_mutex_unlock(&mtx2);
                
            //     break;
            // }
            // else if (info_working[index].terminate == 1 && cnt == 1){
            //     break;
            // }
            
            //change finished to 1, let Send know this thread needs new range in the next while loop condition 2
            info_working[index].haswork = 1;
            cnt = 0;
            // printf("Current range done for thread: %d, ask for another range\n", info_working[index].curt_idx);
            time(&time_end);
            // printf("################end###################_Time:   %d\n", (int)time_end);
            info_working[index].time_in_sec = (int)time_end - time_begin;
            // printf("~~~~~~~~~~~~compute work time ~~~~~~~~~~~~~~~: %d\n",info_working[index].time_in_sec);
        }else if (info_working[index].haswork == 1 && cnt ==0) {
            // printf("FINISHED WORK ASK FOR NEW RANGE AGAIN:\n");
            pthread_mutex_lock(&mtx2);
            while(pending) {
                pthread_mutex_unlock(&mtx2);
                pthread_mutex_lock(&mtx2);
            }
            compute_reply->type = CPT_FINISH_RANGE;
            compute_reply->idx = info_working[index].curt_idx;
            pending  = 1;
            
            pthread_cond_signal(&cond2);
            pthread_mutex_unlock(&mtx2);
            cnt = 1;
        }

        // else if (info_working[index].terminate == 1 && cnt == 1){
        //     break;
        // }
    }
}
// compute_reply->idx = 0;
// compute_reply->type = GET_PERFECT;

int main (int argc, char* argv[]){
    int l;
    if (argc != 4) {
        printf("Error: Please enter correct command: \n");
        printf("hostname, port number, thread number\n");
        exit(1);
    }
    // printf("Client side request %d threads\n", atoi(argv[3]));
    num_of_thread = atoi(argv[3]);
    thread_ids = malloc(sizeof(pthread_t)* num_of_thread);
    compute_reply = malloc(sizeof(computeRespond));
    
    info_working = malloc(sizeof(working_info)*num_of_thread);//setting up the working structure for each thread
    
    for (l = 0; l < num_of_thread; l++) {
        info_working[l].start = 0;
        info_working[l].range = 0;
        info_working[l].time_in_sec = 0;
        info_working[l].perfect = 0;
        info_working[l].current = 0;
        info_working[l].curt_idx = 0;
        info_working[l].haswork = -1;
        info_working[l].terminate = 0;
    }
    //set up a signal thread which handles all signals for all the threads
    int m;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGHUP);
    sigaddset(&signal_set, SIGQUIT);
    m = pthread_sigmask(SIG_BLOCK, &signal_set, NULL); //block all these 3 signals
    //let the signal thread to deal with signals
    if (m != 0) {
        printf("Error: Thread signal mask failed\n");
        exit(1);
    }
    m = pthread_create(&tid_signal, NULL, signal_func, (void *) &signal_set);
    if (m != 0) {
        printf("Error: Signal thread creation failed\n");
        exit(1);
    }else {
        // printf("thread_create for signal succed\n");
    }
   
    struct sockaddr_in sin; /* socket address for destination */
    
    long address;
    /* Fill in Mary's Address */
    address = *(long *) gethostbyname(argv[1])->h_addr;
    sin.sin_addr.s_addr= address;
    sin.sin_family= AF_INET;
    sin.sin_port = htons(atoi(argv[2]));
    
    // printf("Compute :Hostname: %s and port number: %d has been added to address adder\n", argv[1], atoi(argv[2]));

    while(1) { /*loop waiting for Mary if Necessary */      
        /* create the socket */
        if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
            perror("Socket");
            exit(1);
        }
        
        /* try to connect to Mary */
        if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
            printf("Where is that Mary!\n");
            close(s);
            sleep(10);
            continue;
        }
        break; /* connection successful */
    }
    /* Now send Mary the Numbers */
    
    //set up thread on send
    //set up thread on receive
 
    m = pthread_create(&tid_recv, NULL, recv_func, NULL);
    if (m != 0) {
        printf("Error: Receive thread creation failed\n");
        exit(1);
    }else {
        // printf("thread_create for recv succed\n");
    }
    m = pthread_create(&tid_send, NULL, send_func, NULL);
    if (m != 0) {
        printf("Error: Send thread creation failed\n");
        exit(1);
    }else{
        // printf("thread_create for send succed\n");
    }

  

    //set up working process
    pthread_mutex_lock(&mtx);
    int tt;
    for(m = 0; m < num_of_thread; m++){
        info_working[m].haswork = -1;//new Work
        info_working[m].curt_idx = m;
        tt = pthread_create(&thread_ids[m], NULL, working_func,  (void *) &m);
        if (tt != 0){
            printf("Error: compute thread creation failed\n");
            exit(1);
        }else{
            // printf("thread_create for compute %d succed\n", m);
        }
        pthread_cond_wait(&cond,&mtx);
    }
    pthread_mutex_unlock(&mtx);
    pause();
}
