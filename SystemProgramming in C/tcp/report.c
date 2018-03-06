/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */

#include <errno.h>
#include <stdio.h>
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
struct pollfd pollarray[1];  
int main (int argc, char* argv[]){
	struct sockaddr_in sin; /* socket address for destination */
	long address;
	int s;//fd for socket
	int type;
	int temp_wt;
	int temp_rd;
	address = *(long *) gethostbyname(argv[1])->h_addr;
    sin.sin_addr.s_addr= address;
    sin.sin_family= AF_INET;
    sin.sin_port = htons(atoi(argv[2]));

 
    int cnt_perfect;
    int cnt_compute;
    int cnt_threads = 0;
    int i;
    int j;
    int count;
    perfectinfo info_of_perfect;
    host_name_struct hostname_recv;
    char hostname[256];
    uint32_t tested;
    uint32_t start;
    uint32_t end;
    int test_recv = 0;
    // char recv_hostname[256];
    // recv_hostname = '';
        // memset(recv_hostname, 0, sizeof(recv_hostname));
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
        /* Allow a connection queue for up to 21 Computes */
    // listen(s,1); // use the first one for manage 
    // //the other 20 for compute
    
    /* Initialize the pollarray */
    pollarray[0].fd=s;     /* Accept Socket*/
    pollarray[0].events=POLLIN;
    

    type = SEND_REQUEST;
    temp_wt=htonl(type); //send back type
    int l = 0;
    int idx = 0;
    write(s,&temp_wt,sizeof(int));
    // if (argc >3 && strcmp("-k", argv[3]) == 0) {
    //     printf("Receive is sending termination to manage after printing all the information\n");
    // }
   
    while(1) {
        poll(pollarray,1,-1);   /* no timeout, blocks until some activity*/
        if (pollarray[0].revents & POLLIN){
            // printf("have received info for request\n");
            count = read(pollarray[0].fd, &temp_rd, 4);
            type = ntohl(temp_rd);
            if (count == 4) {

                if (type == MANAGE_WILL_SEND_PERFECT_TO_REPORT ) {
                    read(s,  &temp_rd, sizeof(int));
                    cnt_compute = ntohl(temp_rd); //number of perfect
                     // printf("3'%d of compute are still alive\n", cnt_compute);
                    read(s, &temp_rd, sizeof(int));
                    cnt_perfect = ntohl(temp_rd); //number of perfect
                    printf("report receive number of perfect : %d\n",cnt_perfect);
                    for (i = 0; i < cnt_perfect; i++) {
                        read(s,&info_of_perfect,sizeof(perfectinfo));
                        printf("%d :PERFECT NUMBER %d FOUND IN HOST: %s\n", i, info_of_perfect.perfect, info_of_perfect.hostname);
                    }
                }
                if (type == MANAGE_SEND_CURT_TO_REPORT){
                    l++;
                    idx = l;
                    
                    read(s,&hostname,256*sizeof(char));
                    
                    read(s,  &temp_rd, sizeof(int));
                    cnt_threads = ntohl(temp_rd); 
                    printf("Printing Working Compute: %d Hostname: %s for total %d threads info:... \n", idx, hostname, cnt_threads);
                    memset(hostname, 0, sizeof(hostname));
                    for (j = 0; j < cnt_threads; j++) {
                        read(s, &temp_rd, sizeof(int));
                        tested = ntohl(temp_rd); //numbe
                        read(s, &temp_rd, sizeof(int));
                        start = ntohl(temp_rd); //numbe
                        read(s, &temp_rd, sizeof(int));
                        end = ntohl(temp_rd); //numbe
                        printf("Thread %d: Total Tested: %d  Current Working on %d -- %d\n", j,tested,start,end);
                    } 
                }
                if (l == cnt_compute){
                    if (argc >3 && strcmp("-k", argv[3]) == 0) {
                        type = REQUEST_TERMINATE;
                        printf("Report has sent termination to manage.\n");
                        temp_wt=ntohl(type); //send back type
                        write(s,&temp_wt,sizeof(int));
                    }            
                    exit(1);
                }
                
            } 
        }
    }
}