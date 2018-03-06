
/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */

#include <stdio.h>
#include <stdlib.h> // atoi
#include <unistd.h>//pipe|close|execl
#include <ctype.h>//isalpha
#include <string.h>//strtok
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int BUFFER_SIZE = 1024;

void exec_sorters(int s_num,int parse_pipes[][2],int combine_pipes[][2]){
// void exec_sorters(int s_num,int** parse_pipes,int** combine_pipes){
    pid_t pid;
    int i;
    char buf[42];
    //fork s_num children process
    for(i=0;i<s_num;i++){
        pid = fork();
        if (pid<0){
            perror("sorters fork failed");
            exit(1);
        } 
        else if (pid==0){//children
            // Closing pipes of the other sorters
            int j;
            for (j=0;j<s_num;j++) {

                if (i!=j){
                    if(close(parse_pipes[j][0])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the parse_pipe[i][0] closed failed\n");
                    }
                    if(close(parse_pipes[j][1])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the parse_pipe[i][1] closed failed\n");
                    }
                    if(close(combine_pipes[j][0])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the combine_pipe[i][0] closed failed\n");
                    }
                    if(close(combine_pipes[j][1])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the combine_pipe[i][1] closed failed\n");
                    }
                }else{
                    if(close(parse_pipes[j][1])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the parse_pipe[i][0] closed failed\n");
                    }
                    if(close(combine_pipes[j][0])==-1){
                        fprintf(stderr, "i!=j,exec_sorter:the combine_pipe[i][1] closed failed\n");
                    }
                }

            }

            if (dup2(parse_pipes[i][0], STDIN_FILENO) == -1) {
                perror("dup2 failed");
            }
            if(close(parse_pipes[i][0])==-1){
                fprintf(stderr, "exec_sorter:the parse_pipe[i][1] closed failed\n");
            }
            if(dup2(combine_pipes[i][1],STDOUT_FILENO)==-1){
               perror("dup2 failed");
            }
            if(close(combine_pipes[i][1])==-1){
                fprintf(stderr, "exec_sorter:the combine_pipe[i][0] closed failed\n");
            }
            
            //after dup,close those unused
            //close the pipes that are not using
            execlp("sort","sort",NULL);
            exit(3);////////////
        } else {//parent
        }
    }
}


void exec_parser(int s_num, int parse_pipes[][2]){
// void exec_parser(int s_num, int** parse_pipes){
    //void exec_parser(int s_num, char * path){
    char buffer[BUFFER_SIZE];
    char *out = (char*)malloc(BUFFER_SIZE);
//    FILE * input;
//    input = fopen("test.txt", "r");
    FILE *parse_out[s_num];
    for(int i=0;i<s_num;i++){
        parse_out[i]=fdopen(parse_pipes[i][1],"w");
        if(parse_out[i]==NULL){
            perror("parse_pipe open failed");
            exit(1);
        }
    }
       // parse_out[0]=fopen("test0.txt","w");
       // parse_out[1]=fopen("test1.txt","w");
       // parse_out[2]=fopen("test2.txt","w");
    int i;
    int count=0;
    while(fgets(buffer,BUFFER_SIZE,stdin)!=NULL){
        for(i=0;i<strlen(buffer);i++){
            if(isalpha(buffer[i])==0){
                buffer[i]=' ';
            }else{
                buffer[i]=tolower(buffer[i]);
            }
        }
        char* buf=strtok(buffer," ");
        while(buf!=NULL){
            if(strlen(buf)>=3){
                if(strlen(buf)>40){
                    buf[40]='\0';
                    sprintf(out, "%s\n",  buf);
                }else{
                    sprintf(out, "%s\n",  buf);
                }
                fputs(out, parse_out[count%s_num]);
                // printf("Sent %s to %d\n", buf, count%s_num);
                count++;
            }
            buf = strtok(NULL," ");
        }
    }
    //    fclose(parse_out[0]);
    //    fclose(parse_out[1]);
    //    fclose(parse_out[2]);
    
    for(int i=0;i<s_num;i++){
        if(fclose(parse_out[i])==-1){
            perror("Parser: close pipe error parse_out[i][1]\n");
            
        }
//        else{
//            fprintf(stderr,"Parser:close parse_out[i][1] good\n");
//        }
    }
    free(out);
//    fclose(input);
}
void exec_combine(int s_num, int combine_pipes[][2], int parse_pipes[][2]){
    char min_index[42];//point to the minimum alphabetic word
    //here we use the buffer to store the minimum word
    char buf_combine[s_num][42];
    int pipe_ctrl[s_num];
    int freq=0;
    pid_t c_pid;//fork suppresser
    c_pid = fork();
    if(c_pid<0) {
        perror("combine_exec failed");
        exit(1);
    } else if(c_pid==0){
        FILE *combine_input[s_num];//read from the pipe and output to combine_input
        int empty;
        for(int i=0;i<s_num;i++){
            combine_input[i]=fdopen(combine_pipes[i][0], "r");
            if( close(parse_pipes[i][0])==-1){
                printf("exec_combine:close parse_pipe[i][0]failed: %d\n", parse_pipes[i][0]);
                perror("exec_combine:close parse_pipe[i][0]");
            }
            if( close(parse_pipes[i][1])==-1){
                printf("exec_combine:close parse_pipe[i][1]failed %d\n", parse_pipes[i][1]);
                perror("exec_combine:close parse_pipe[i][1]");
            }
            if( close(combine_pipes[i][1])==-1){
                printf("exec_combine:close combine_pipe[i][1]failed\n");
                perror("exec_combine:close combine_pipe[i][1]");
            }
            // combine_input[i]=fdopen(combine_pipes[i][0], "r");
        }
        for (int i=0;i<s_num;i++){
            //read one line from the combine_input to the buf_combine
            if(fgets(buf_combine[i],BUFFER_SIZE,combine_input[i])!=NULL){
                // printf("Combine: %s from index %d", buf_combine[i], i);
                pipe_ctrl[i] = 1;
            }
            else {
                pipe_ctrl[i] = 0;
            }
        }
        empty = s_num;
        do {
            freq = 0;
            for(int j=0; j<s_num; j++){
                if (pipe_ctrl[j]==1) {
                    strcpy(min_index, buf_combine[j]);
                    break;
                }
            }
            for(int j=0; j<s_num; j++){
                if (pipe_ctrl[j]==1 && strcmp(buf_combine[j], min_index)<0){
                    strcpy(min_index, buf_combine[j]);
                }
            }
            for (int j=0;j<s_num;j++){
                while (strcmp(min_index, buf_combine[j])==0){
                    freq++;
                    if (pipe_ctrl[j]==1 && fgets(buf_combine[j], BUFFER_SIZE, combine_input[j]) == NULL) {
                        pipe_ctrl[j] = 0;
                        empty--;
                        break;
                    } else {
                        //printf("Combine: %s from index %d", buf_combine[j], j);
                        int kk = 0;
                    }
                }
            }
            // printf("result: \n");
            printf("%5d %s",freq, min_index);

        } while(empty!=0);
        for(int i=0;i<s_num; i++){
            if(fclose(combine_input[i])==-1){
                printf("exec_combine close all the combine_input[i]failed\n");
                //use fclose instead of close to flush out all the information in the buffer
            }
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    int s_num = atoi(argv[1]);
    int parse_pipes[s_num][2];
    int combine_pipes[s_num][2];
    int i;
    //form pipes
    for( i=0;i<s_num;i++){
        if(pipe(parse_pipes[i])==-1){
            perror("error:parse_pipes creation failed\n");
            exit(1);
        }
        if (pipe(combine_pipes[i])==-1){
            perror("error:combine_pipe creation failed\n");
            exit(1);
        }
    }
    //1st form the pipes
    //2nd fork sorters
    //3rd fork suppresser
    //4th execute the parent process--parser
    exec_sorters(s_num, parse_pipes, combine_pipes);
    exec_combine(s_num, combine_pipes, parse_pipes);
    for ( i=0 ; i<s_num; i++) {
        if(close(parse_pipes[i][0])==-1){
            printf("before parser: parse_pipe[i][0] close failed\n");
        }
        if(close(combine_pipes[i][1])==-1){
            printf("before parser: combine_pipe[i][1] close failed\n");
        }
        if(close(combine_pipes[i][0])==-1){
            printf("before parser: combine_pipe[i][0] close failed\n");
        }
    }
    //only left the port that parse uses 
    exec_parser(s_num, parse_pipes);
    //  loop waiting for all children
    int status, pid;
    //printf("Started waiting...\n");
    for (i=0; i<s_num; i++) {
        pid = wait(&status); //wait for first child
        // printf("pid %d finished\n", pid);
    }
    return 0;
}
