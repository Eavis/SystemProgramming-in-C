/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */

#ifndef hwk4_h
#define hwk4_h



#define ASK_FOR_RANGE 1
#define RESPOND_RANGE 2
#define FINISH_RANGE 3
#define GET_PERFECT 4
#define REQUEST_FOR_CURRENT 5
#define RANGE_TERMINATE 6
#define REQUEST_TERMINATE 7  //request send a termination request to manage 
#define UPDATE_TERMINATION 8
#define COMPUTE_SEND_CURRENT 9
#define SEND_REQUEST 10
#define CPT_ASK_RANGE 11
#define CPT_FINISH_RANGE 12
#define CPT_FIND_PERFECT 13
#define CPT_TERMINATE 19
#define CPT_SEND_CURRENT 14
#define TERMINATE_FROM_MANAGE_TO_COMPUTE 15 //manage send termination to compute 
#define MANAGE_WILL_SEND_PERFECT_TO_REPORT 16
#define MANAGE_ASK_INFO_FROM_COMPUTE 17
#define MANAGE_SEND_CURT_TO_REPORT 18
typedef struct _rangeinfo {
    int start;
    int range;
    struct _rangeinfo* next;
    char hostname[256];
}rangeinfo;

typedef struct _perfectinfo {
	int perfect;
	char hostname[256];
}perfectinfo;

typedef struct _working_info {
	int start;
	int range;
	int time_in_sec;
	int perfect;
	int current;
	int curt_idx;
	int haswork;
	int terminate;
}working_info;
typedef struct _computeRespond {
	int idx;
	int type;
}computeRespond;
typedef struct _hostname {
	char hostname[256];
}host_name_struct;
#endif /* hwk4_h */
