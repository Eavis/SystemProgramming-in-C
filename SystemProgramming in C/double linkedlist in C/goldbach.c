/*
 THIS CODE IS MY OWN WORK, 
 IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> /* memset */
typedef struct _seg {  /* definition of new type "seg" */
    int  bits[256];
    struct _seg  *next,*prev;
}seg  ;

#define BITSPERSEG  (8*256*sizeof(int))

float numberN = 0;
seg* head;
seg* lastseg;
double last_pos = 0;

int numOfseg(){
    int numSeg = 0;
    float seg = 0;
    float numforodd = 0;
    numforodd = numberN / 2;
    seg = numforodd / BITSPERSEG;
//    numSeg = ceil(seg);
    numSeg = (int)seg + 1;
    return numSeg;
}
seg* whichseg(int j) {
    if(lastseg == NULL){
        lastseg = head;
    }
//    double pos_seg = 0;
//    pos_seg = floor(((double)(j-3)/2)/BITSPERSEG);
//    pos_seg = floor(((double)(j-3)/2)/BITSPERSEG);
    int pos_seg = ((double)(j-3)/2)/BITSPERSEG;
    seg* whichSeg;
    whichSeg = lastseg;
    
    if(last_pos == pos_seg) {
        whichSeg = lastseg;
    }else if (last_pos < pos_seg){
        while(last_pos < pos_seg) {
            whichSeg = whichSeg -> next;
            last_pos++;
        }
    }else if (last_pos > pos_seg){
        while(last_pos > pos_seg) {
            whichSeg = whichSeg -> prev;
            last_pos--;
        }
    }
    lastseg = whichSeg;
    last_pos = pos_seg;
    return whichSeg;
}
int whichint(int j) {
    int whichInt;
//    whichInt = floor((double)(j - 3) /64);
    whichInt = (double)(j - 3) /64;
    whichInt = whichInt % 256;
    return whichInt;
}

int whichbit(int j) {
    int whichBit;
    whichBit = ((j - 3) / 2) % 32;
    return whichBit;
}

int test(seg* j_seg,int j_int,int j_bit){
    
    return ((j_seg->bits[j_int]&(1<<j_bit))==0);
}

int set(int j){
    seg *j_seg = whichseg(j);
    int j_int = whichint(j);
    int j_bit = whichbit(j);
    if(test(j_seg,j_int,j_bit)){
        j_seg->bits[j_int] |= 1<< j_bit;
    }
    return 1;
}

int setUpBitmap(double stop){
    int i,j;
    for (i = 3; i <= stop; i+=2) {
        for(j = 2; i*j <= numberN; j++) {
            if( (i*j) % 2 != 0) {
                if(set(i*j)!=1){
                    printf("Error in setting the bitmap");
                }
            }
        }
    }
    int numOfprime = 0;
    for(i = 3; i <= numberN; i+=2) {
        seg *i_seg = whichseg(i);
        int i_int = whichint(i);
        int i_bit = whichbit(i);
        if(test(i_seg,i_int,i_bit)){
            numOfprime++;
        }
    }
    return numOfprime;
}

int main(int argc, const char * argv[]) {
    int even_num;
    int num_seg;
    int cnt_pairs = 0;
//    printf("Please Enter Number N: \n");
//    scanf("%f",&numberN);
    if (argc == 2){
        sscanf(argv[1],"%f",&numberN);
    }else{
        printf("Please Enter Number N: \n");
        scanf("%f",&numberN);
    }
    printf("Calculating odd primes up to %d ...\n",(int) numberN);
    num_seg = numOfseg(); //calculate the number of segment needed
    seg *pt;
    head= (seg *) malloc(sizeof(seg));
    pt=head;
    int i;
    for (i=1; i<num_seg; i++) {  //double linked list
        pt->next = (seg *) malloc(sizeof (seg));
        pt->next->prev = pt;
        pt = pt->next;
    }
    double stop = sqrt(numberN);
    int numOfprime = setUpBitmap(stop);
    printf("Found %d odd primes.\n",numOfprime);
    printf("Enter Even Numbers >5 for Goldbach Tests: \n");
    while( scanf("%d",&even_num) != EOF ){
        
        if(even_num % 2 != 0){
            printf("Please enter an even number:\n");
            continue;
        }
        if(even_num > numberN ){
            printf("Please enter an even number less than number N\n");
            printf("Enter Even Numbers >5 for Goldbach Tests: \n");
            continue;
        }
        seg *start_seg = whichseg(3);
        int start_int = whichint(3);
        int start_bit = whichbit(3);
        seg *end_seg = whichseg(even_num - 3);
        int end_int = whichint(even_num - 3);
        int end_bit = whichbit(even_num - 3);
        int high = 0;
        int pair =0;
        for(int i = 3; i <= even_num / 2; i = i+2){
            if(test(start_seg,start_int,start_bit) & test(end_seg,end_int, end_bit)){
                if(high < i) {
                    high = i;
                    pair = even_num - high;
                }
                cnt_pairs++;
            }
            start_bit++;
            end_bit--;
            if(start_bit > 31){
                start_bit = 0;
                start_int++;
            }
            if(end_bit < 0){
                end_bit = 31;
                end_int--;
            }
            if(start_int > 255) {
                start_int = 0;
                start_seg = start_seg->next;
            }
            if(end_int < 0) {
                end_int = 255;
                end_seg = end_seg->prev;
            }
            
        }
        printf("Largest %d =  %d + %d out of %d solutions\n",even_num,high,pair,cnt_pairs);
        
    }
    return 0;
}
