/*
 THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS
 TIANYU LAN
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //lseek
#include <ar.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <search.h>
#include <errno.h>


#define NAME_LIMIT 16
#define DATE_LIMIT 12
#define UID_LIMIT 6
#define GID_LIMIT 6
#define MODE_LIMIT 8
#define SIZE_LIMIT 10
#define ARFMAG_LIMIT 2
#define STR_SIZE sizeof("rwxrwxrwx")
#define FP_SPECIAL 1


//struct ar_hdr
//{
//    char ar_name[16];		/* Member file name, sometimes / terminated. */
//    char ar_date[12];		/* File date, decimal seconds since Epoch.  */
//    char ar_uid[6], ar_gid[6];	/* User and group IDs, in ASCII decimal.  */
//    char ar_mode[8];		/* File mode, in ASCII octal.  */
//    char ar_size[10];		/* File size, in ASCII decimal.  */
//    char ar_fmag[2];		/* Always contains ARFMAG.  */
//};


//
//#define	ARMAG		"!<arch>\n"	/* ar "magic number" */
//#define	SARMAG		8		/* strlen(ARMAG); */
//#define ARFMAG	"`\n"		/* String in ar_fmag at end of each header.  */


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//test and creat an archive file afile
//int test_archive(char **argv){
//    int fd;
//    //char* afile =argv[2];
//    fd=open(argv[2],O_RDWR);
//    if(fd==-1){
//        fprintf(stderr,"usage:%s1\n",strerror(errno));
//        exit(EXIT_FAILURE);
//    }else{
//         write(fd,ARMAG,SARMAG);
//    }
//    close(fd);
//    return fd;
//}

//    if(fd==-1){
//        fd=open(argv[2],O_RDWR|O_CREAT,0666);
//        if(fd==-1){
//            fd=open(argv[2],O_RDWR|O_CREAT,0666);
//        }else{
//            fprintf(stderr,"usage:%s1\n",strerror(errno));
//            exit(EXIT_FAILURE);
//        }
//
//    }
//    write(fd,ARMAG,SARMAG);
//    return fd;
//}

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//calculate size of afile and if the size if off, add 1 to the size

int IS_filesize(char *afile){
    
    struct stat size_of_file;
    stat(afile,&size_of_file);
    int file_size=size_of_file.st_size;
    if (file_size % 2 != 0)
    {
        file_size += 1;
    }
    return file_size;
}




//// Code Refernece: Book <Linux Programming Interface> Page 296
char * file_perm_string(mode_t perm, int flags)
{
    static char str[STR_SIZE];
    snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
             (perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
             (perm & S_IXUSR) ?
             (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
             (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
             (perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
             (perm & S_IXGRP) ?
             (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
             (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
             (perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
             (perm & S_IXOTH) ?
             (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 't' : 'x') :
             (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 'T' : '-'));
    return str;
}
///////////////////////////////////////////////////////////////////////////////////////
//q--quickly append named files to archive
void q_append(int argc, char* argv[])
{
    int fd, q_fd, n, i;
    struct stat buf;
    char h_buffer[128];
    char r_buffer[1024];
    
//    for(i =0; i<argc;i++){
//        printf("arg[%d]: %s\n",i,argv[i]);
//        fflush(stdout);
//    }
    int fileexisted = access( argv[2], F_OK );
    
    fd=open(argv[2],O_RDWR|O_CREAT|O_APPEND,0666);
    if(fd==-1){
        fprintf(stderr,"usage:%s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        if(fileexisted == -1 ){
            write(fd,ARMAG,SARMAG);
        }
    }
    for(i=3;i<argc;i++){
        //        printf("argv[%d]:%s\n",i,argv[i]);
        int j;
//        for(j =0; j<argc;j++){
//            printf("arg[%d]: %s\n",j,argv[j]);
//            fflush(stdout);
//        }
        
        q_fd = open(argv[i],O_RDWR);
        
    
        if(q_fd==-1){
            fprintf(stderr,"usage:%s\n",strerror(errno));
            exit(EXIT_FAILURE);
        }else{
            stat(argv[i],&buf);
            //            snprintf(h_buffer, 59, "%-*s%-*d%-*d%-*d%-*o%-*d", 16,
            //                     strcat(argv[i], "/"), 12, (int)buf.st_mtime, 6,
            //                     buf.st_uid, 6, buf.st_gid, 8,
            //                     buf.st_mode, 10, (int)buf.st_size);
            int argvlen = strlen(argv[i]);
            char * newfilename = malloc(sizeof(char)* argvlen + 1);
            newfilename = strcpy(newfilename, argv[i]);
            
            int filesize = (int)buf.st_size;
            if(filesize%2==1){
                filesize +=1;
            }
            
            snprintf(h_buffer, 58, "%-*s%-*d%-*d%-*d%-*o%-*d", 16,
                     newfilename, 12, (int)buf.st_mtime, 6,
                     buf.st_uid, 6, buf.st_gid, 8,
                     buf.st_mode, 10, filesize);
            
            //printf("h_buffer:%s \n", h_buffer);
            //write header buffer to archive file
            write(fd, h_buffer, 58);
            //mark end of header.
            write(fd, &ARFMAG, 2);
            
            //            j  = sprintf( h_buffer,    "%s  ", argv[i] );
            //            j += sprintf( h_buffer + j, "%d  ", (int)buf.st_mtime);
            //            j += sprintf( h_buffer + j, "%d  ",  buf.st_uid);
            //            j += sprintf( h_buffer + j, "%d  ",  buf.st_gid);
            //            j += sprintf( h_buffer + j, "%o  ",  buf.st_mode);
            //            j += sprintf( h_buffer + j, "%d  ", (int)buf.st_size);
            //write header buffer to archive file
            //            write(fd, h_buffer, j);
            //mark end of header.
            //write(fd, &ARFMAG, 2);
            while((n=read(q_fd,r_buffer,1024))!=0){
                if((write(fd,r_buffer,n)==-1)){
                    fprintf(stderr,"usage:%s3\n",strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            if(((int)buf.st_size%2)==1){
                 write(fd,"\n",1);
            }
            
        }
        close(q_fd);
    }
    close(fd);
    
    
}


//Output the file names from the archive file
void t_concise(int argc,char **argv){
    int fd;
    //char* q_Hdr;
    struct ar_hdr q_Hdr;
    char *new_file_name;
    fd=open(argv[2],O_RDWR);
    if(fd!=-1){//
        //open archive and skip the magic number
        if(lseek(fd,SARMAG,SEEK_SET)!=-1){
            //read header of the file[60]
            while(read(fd,&q_Hdr,60)!=0){
                //print out the verbose info
                new_file_name=q_Hdr.ar_name;
                printf("%.*s\n",16,new_file_name);
                //                printf("%s %d/%d %6d %s %.*s\n",
                //
                //                       file_perm_string(strtol(myHdr.ar_mode, NULL, 8), 1),
                //                       atoi(myHdr.ar_uid),atoi(myHdr.ar_gid),
                //                       atoi(myHdr.ar_size), header, 16, new_name);
                
                lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
                
            }
        }else{
            fprintf(stderr,"usage:%s3\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }//
    else{
        fprintf(stderr,"usage:%s3\n",strerror(errno));
        exit(EXIT_FAILURE);
    }//
}

//Output the complete information of the archive file
void tv_verbose(int argc, char **argv){
    int fd;
    //char* q_Hdr;
    struct ar_hdr q_Hdr;
    char *new_file_name;
    fd=open(argv[2],O_RDWR);
    if(fd!=-1){//
        //open archive and skip the magic number
        if(lseek(fd,SARMAG,SEEK_SET)!=-1){
            //read header of the file[60]
            while(read(fd,&q_Hdr,60)!=0){
                //print out the verbose info
                new_file_name=q_Hdr.ar_name;
                //                printf("%s\n",new_file_name);
                
                
                // Reference: http://man7.org/linux/man-pages/man3/strftime.3.html
                // Reference: Command line help
                //When used with the -t option, ar displays an ``ls -l'' style
                //                listing of information about the members of the archive.  This
                //                listing consists of eight, white-space separated fields: the file
                //                permissions (see strmode(3) ), the decimal user and group ID's,
                //                separated by a single slash (``/''), the file size (in bytes),
                //                the file modification time (in the date(1) format ``%b %e %H:%M
                //                                            %Y''), and the name of the file.
                
                //Generate the Permissions
                
                
                //Generate the decimal and Group IDs
                
                
                //Output the file size
                
                
                // Generate the file modificaiton
                time_t curtime = atoi (q_Hdr.ar_date);
                //                printf("curtime:%d",curtime);
                fflush(stdout);
                
                struct tm *curtm = localtime(&curtime);
                char curDate[30] ;
                strftime(curDate, 30, "%b %e %H:%M %Y", curtm);
                
                
                printf("%s %d/%d %d %s %.*s\n", file_perm_string(strtol(q_Hdr.ar_mode, NULL, 8), 1),
                       atoi(q_Hdr.ar_uid),atoi(q_Hdr.ar_gid), atoi(q_Hdr.ar_size), curDate, 16, q_Hdr.ar_name);
                
                
                //                printf("curDate:%s",curDate);
                fflush(stdout);
                
                
                //                printf("%s %d/%d %6d %s %.*s\n",
                //
                //                       file_perm_string(strtol(myHdr.ar_mode, NULL, 8), 1),
                //                       atoi(myHdr.ar_uid),atoi(myHdr.ar_gid),
                //                       atoi(myHdr.ar_size), header, 16, new_name);
                
                lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
                
            }
        }else{
            fprintf(stderr,"usage:%s3\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }//
    else{
        fprintf(stderr,"usage:%s3\n",strerror(errno));
        exit(EXIT_FAILURE);
    }//
}

// Append file to the Archive file
void A_append(int argc, char **argv){
    
    int fd, q_fd, n,i;
    struct stat buf;
    char h_buffer[60];
    char r_buffer[1024];
    DIR *mydir=opendir(".");
    struct dirent *hp;
    
    //creat an archive
    fd=open(argv[2],O_RDWR|O_CREAT,0666);
    if(fd==-1){
        fprintf(stderr,"usage:%s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        write(fd,ARMAG,SARMAG);
    }
    
    while((hp=readdir(mydir))!=NULL){
//        printf("curFileName:%s\n",hp->d_name);
//        fflush(stdout);
        if(strcmp(hp->d_name,".")==0){
            //DOING NOTHING
        }else if(strcmp(hp->d_name,"..")==0){
            //Doing nothing
        }else if(strcmp(hp->d_name,"myar.c")==0){
            //doing nothing
        }else if(strcmp(hp->d_name,"myar")==0){
            //doing nothing
        }else if(strstr(hp->d_name, "DS_Store")!=NULL){
            //doing nothing
        }else if(strcmp(hp->d_name,"main.c")==0){
            //doing nothing
        }else if(strstr(hp->d_name, "FileIO")!=NULL){
            
        }else if(strcmp(hp->d_name,argv[2])==0){
            //doing nothing
            
        }else{
            lstat(hp->d_name,&buf);
//            printf("symbolic file:%d\n", buf.st_mode);
            fflush(stdout);

            if((S_ISLNK(buf.st_mode)==1)){
                struct stat sy_buf;
                lstat(hp->d_name,&sy_buf);
                
                int filesize = (int)sy_buf.st_size;
                
                if(filesize%2==1){
                    filesize+=1;
                }
                
                snprintf(h_buffer, 58, "%-*s%-*d%-*d%-*d%-*o%-*d", 16,
                         hp->d_name, 12, (int)sy_buf.st_mtime, 6,
                         sy_buf.st_uid, 6, sy_buf.st_gid, 8,
                         sy_buf.st_mode, 10, filesize);
                write(fd, h_buffer, 58);
                //mark end of header.
                write(fd, &ARFMAG, 2);
                int linksize = readlink(hp->d_name,r_buffer,1024);
//                printf("linksize:%d\n",linksize);
                fflush(stdout);
                write(fd,r_buffer,linksize);
                
                if(((int)sy_buf.st_size%2)==1){
                    write(fd,"\n",1);
                }
                
            }else{
                q_fd=open(hp->d_name,O_RDWR);
                int filesize = (int)buf.st_size;
                
                if(filesize%2==1){
                    filesize+=1;
                }
                
                snprintf(h_buffer, 58, "%-*s%-*d%-*d%-*d%-*o%-*d", 16,
                         hp->d_name, 12, (int)buf.st_mtime, 6,
                         buf.st_uid, 6, buf.st_gid, 8,
                         buf.st_mode, 10, filesize);
                
                //write header buffer to archive file
                write(fd, h_buffer, 58);
                //mark end of header.
                write(fd, &ARFMAG, 2);
                
                while((n=read(q_fd,r_buffer,1024))!=0){
                    if((write(fd,r_buffer,n)==-1)){
                        fprintf(stderr,"usage:%s3\n",strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                
                if(((int)buf.st_size%2)==1){
                    write(fd,"\n",1);
                }
                
                close(q_fd);
            }
        }
    }
    close(fd);
}

//Extract all file from the archive files.
// ***** BONUS: If no name is specified, I extract all files from the archive
void extract(int argc, char **argv){
    int fd,i;
    //char* q_Hdr;
    struct ar_hdr q_Hdr;
    char new_file_name[18];
    char r_buffer[1024];
    
    int numFiles = argc - 3;
    int extracted[numFiles];
    for(i=0; i<numFiles;++i){
        extracted[i] = 0;
    }
    
    fd=open(argv[2],O_RDWR);
    if(fd!=-1){
        
        //open archive and skip the magic number
        if(lseek(fd,SARMAG,SEEK_SET)!=-1){
            
            //read header of the file[60]
            while(read(fd,&q_Hdr,60)!=0){
                //print out the verbose info
                //                strcpy(new_file_name, q_Hdr.ar_name);
                sprintf(new_file_name, "%.*s", 16, q_Hdr.ar_name);
                int foundfile = 0;
                int extractedcurFile = 0;
                for(i= 3; i<argc;++i){
                    //                    prtintf("argv[%d]:%s",i,argv[i]);
                    if(strstr(new_file_name, argv[i])){
                        foundfile = 1;
                        if(extracted[i-3]!=0){
                            extractedcurFile = 1;
                            break;
                        }else{
                            extracted[i-3]=1;
                            break;
                        }
                    }
                }
                
                //                if(foundfile==0){
                //                    continue;
                //                }
                //                if(foundfile==1&&extractedcurFile==1){
                //                    continue;
                //                }
                
                // Generate the file modificaiton
                time_t curtime = atoi (q_Hdr.ar_date);
                //                printf("curtime:%d",curtime);
                fflush(stdout);
                
                struct tm *curtm = localtime(&curtime);
                char curDate[30] ;
                strftime(curDate, 30, "%b %e %H:%M %Y", curtm);
//                printf("%s %d/%d %d %s %.*s\n", file_perm_string(strtol(q_Hdr.ar_mode, NULL, 8), 1),
//                       atoi(q_Hdr.ar_uid),atoi(q_Hdr.ar_gid), atoi(q_Hdr.ar_size), curDate, 16, q_Hdr.ar_name);
                fflush(stdout);
            
                // Set the mode of the file
                int filemode;
                sscanf(q_Hdr.ar_mode, "%o", &filemode);
                
                // If there are not file names specified, extracted all files from the archive
                if((foundfile==1&&extractedcurFile==0)||(argc==3)){
                    
                    int n_fd = open(new_file_name, O_CREAT|O_WRONLY,0666,filemode);
                    
                    if(n_fd==-1){
                        perror("Can't open new output file , program aborted");
                        exit(EXIT_FAILURE);
                    }
                    
                    int filesize = atoi(q_Hdr.ar_size);
                    int readsize = 0;
                    // Reference: http://man7.org/linux/man-pages/man3/strftime.3.html
                    // Reference: Command line help
                    //When used with the -t option, ar displays an ``ls -l'' style
                    //                listing of information about the members of the archive.  This
                    //                listing consists of eight, white-space separated fields: the file
                    //                permissions (see strmode(3) ), the decimal user and group ID's,
                    //                separated by a single slash (``/''), the file size (in bytes),
                    //                the file modification time (in the date(1) format ``%b %e %H:%M
                    //                                            %Y''), and the name of the file.
                    
                    
                    //
                    //                if((readsize = read(fd, r_buffer, filesize))!=0){
                    //                    write(n_fd, r_buffer, readsize);
                    //                }
                    //
                    if((readsize = read(fd, r_buffer, filesize))!=0){
//                        printf("readsize: %d", readsize);
//                        fflush(stdout);
                        write(n_fd, r_buffer, readsize);
                    }
                    
                    close(n_fd);
                    
                    if(chown(new_file_name, atoi(q_Hdr.ar_uid), atoi(q_Hdr.ar_gid))==-1){
                        perror("Can't change owner of the file , program aborted");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
                }
                
                
                
                //Move the pointer to the later part
                //                lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
            }
        }else{
            fprintf(stderr,"usage:%s3\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }//
    else{
        fprintf(stderr,"usage:%s3\n",strerror(errno));
        exit(EXIT_FAILURE);
    }//
    close(fd);
}


//Extract files with correct file timestamp
void extractXO(int argc, char* argv[]){
    int fd,i;
    //char* q_Hdr;
    struct ar_hdr q_Hdr;
    char new_file_name[18];
    char r_buffer[1024];
    
    int numFiles = argc - 3;
    int extracted[numFiles];
    for(i=0; i<numFiles;++i){
        extracted[i] = 0;
    }
    
    fd=open(argv[2],O_RDWR);
    if(fd!=-1){
        
        //open archive and skip the magic number
        if(lseek(fd,SARMAG,SEEK_SET)!=-1){
            
            //read header of the file[60]
            while(read(fd,&q_Hdr,60)!=0){
                //print out the verbose info
                //                strcpy(new_file_name, q_Hdr.ar_name);
                sprintf(new_file_name, "%.*s", 16, q_Hdr.ar_name);
                int foundfile = 0;
                int extractedcurFile = 0;
                for(i= 3; i<argc;++i){
                    //                    prtintf("argv[%d]:%s",i,argv[i]);
                    if(strstr(new_file_name, argv[i])){
                        foundfile = 1;
                        if(extracted[i-3]!=0){
                            extractedcurFile = 1;
                            break;
                        }else{
                            extracted[i-3]=1;
                            break;
                        }
                    }
                }
                
                //                if(foundfile==0){
                //                    continue;
                //                }
                //                if(foundfile==1&&extractedcurFile==1){
                //                    continue;
                //                }
                
                // Generate the file modificaiton
                time_t curtime = atoi (q_Hdr.ar_date);
                //                printf("curtime:%d",curtime);
                fflush(stdout);
                
                struct tm *curtm = localtime(&curtime);
                char curDate[30] ;
                strftime(curDate, 30, "%b %e %H:%M %Y", curtm);
//                printf("%s %d/%d %d %s %.*s\n", file_perm_string(strtol(q_Hdr.ar_mode, NULL, 8), 1),
//                       atoi(q_Hdr.ar_uid),atoi(q_Hdr.ar_gid), atoi(q_Hdr.ar_size), curDate, 16, q_Hdr.ar_name);
//                fflush(stdout);
                
                // Set the mode of the file
                int filemode;
                sscanf(q_Hdr.ar_mode, "%o", &filemode);
                // create a new file
                
                
                
                if(foundfile==1&&extractedcurFile==0){
                    
                    int n_fd = open(new_file_name, O_CREAT|O_WRONLY,0666,filemode);
                    
                    if(n_fd==-1){
                        perror("Can't open new output file , program aborted");
                        exit(EXIT_FAILURE);
                    }
                    
                    int filesize = atoi(q_Hdr.ar_size);
                    int readsize = 0;
                    // Reference: http://man7.org/linux/man-pages/man3/strftime.3.html
                    // Reference: Command line help
                    //When used with the -t option, ar displays an ``ls -l'' style
                    //                listing of information about the members of the archive.  This
                    //                listing consists of eight, white-space separated fields: the file
                    //                permissions (see strmode(3) ), the decimal user and group ID's,
                    //                separated by a single slash (``/''), the file size (in bytes),
                    //                the file modification time (in the date(1) format ``%b %e %H:%M
                    //                                            %Y''), and the name of the file.
                    
                    
                    //
                    //                if((readsize = read(fd, r_buffer, filesize))!=0){
                    //                    write(n_fd, r_buffer, readsize);
                    //                }
                    //
                    if((readsize = read(fd, r_buffer, filesize))!=0){
//                        printf("readsize: %d", readsize);
//                        fflush(stdout);
                        write(n_fd, r_buffer, readsize);
                    }
                    
                    close(n_fd);
                    
                    if(chown(new_file_name, atoi(q_Hdr.ar_uid), atoi(q_Hdr.ar_gid))==-1){
                        perror("Can't change owner of the file , program aborted");
                        exit(EXIT_FAILURE);
                    }
                    
                    struct utimbuf newtimbuf;
                    newtimbuf.actime = atoi (q_Hdr.ar_date);
                    newtimbuf.modtime = atoi (q_Hdr.ar_date);
                    
                    if(utime(new_file_name, &newtimbuf)==-1){
                        perror("Can't change the access time of the file , program aborted");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
                }
                
                
                
                //Move the pointer to the later part
                //                lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
            }
        }else{
            fprintf(stderr,"usage:%s3\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }//
    else{
        fprintf(stderr,"usage:%s3\n",strerror(errno));
        exit(EXIT_FAILURE);
    }//
    close(fd);
}
///////////////////////////////////////////////////////////////////////////////////////
//d--delete named files from archive
void delete(int argc, char **argv){
    int fd,i;
    //char* q_Hdr;
    struct ar_hdr q_Hdr;
    char new_file_name[18];
    char r_buffer[1024];
    char* tmpFilename = "tmpfile.txt";
    
    // Create an array that tracks the status of the files in the archive
    int numFiles = argc - 3;
    int deleted[numFiles];
    for(i=0; i<numFiles;++i){
        deleted[i] = 0;
    }
    
    // Open the orignial Archive File
    fd=open(argv[2],O_RDWR);
    
    if(fd!=-1){
        
        // Create a temp file to write all the files
        int tmptxt = open(tmpFilename, O_CREAT|O_WRONLY,0666);
        if(tmptxt ==-1){
            perror("Can't create tmptxt file , program aborted");
            exit(EXIT_FAILURE);
        }else{
            write(tmptxt,ARMAG,SARMAG);
        }
        
        //open archive and skip the magic number
        if(lseek(fd,SARMAG,SEEK_SET)!=-1){
            
            //read header of the file[60]
            while(read(fd,&q_Hdr,60)!=0){
                
                sprintf(new_file_name, "%.*s", 16, q_Hdr.ar_name);
                int foundfile = 0;
                int extractedcurFile = 0;
                
                for(i= 3; i<argc;++i){
                    //                    prtintf("argv[%d]:%s",i,argv[i]);
                    if(strstr(new_file_name, argv[i])){
                        foundfile = 1;
                        if(deleted[i-3]!=0){
                            extractedcurFile = 1;
                            break;
                        }else{
                            deleted[i-3]=1;
                            break;
                        }
                    }
                }
                
                // Generate the file modificaiton
                time_t curtime = atoi (q_Hdr.ar_date);
                //                printf("curtime:%d",curtime);
                fflush(stdout);
                
                struct tm *curtm = localtime(&curtime);
                char curDate[30] ;
                strftime(curDate, 30, "%b %e %H:%M %Y", curtm);
//                printf("%s %d/%d %d %s %.*s\n", file_perm_string(strtol(q_Hdr.ar_mode, NULL, 8), 1),
//                       atoi(q_Hdr.ar_uid),atoi(q_Hdr.ar_gid), atoi(q_Hdr.ar_size), curDate, 16, q_Hdr.ar_name);
//                fflush(stdout);
                
                // Set the mode of the file
                int filemode;
                sscanf(q_Hdr.ar_mode, "%o", &filemode);
                // create a new file
                
                if(foundfile==1&&extractedcurFile==0){
                    
                    lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
                    
                }else{
                    
                    int filesize = atoi(q_Hdr.ar_size);
                    int readsize = 0;
                    
                    char * h_buffer[60];
                    snprintf(h_buffer, 58, "%-*s%-*d%-*d%-*d%-*o%-*d", 16,
                             new_file_name, 12, curtime, 6,
                             atoi(q_Hdr.ar_uid), 6, atoi(q_Hdr.ar_gid), 8,
                             strtol(q_Hdr.ar_mode, NULL, 8), 10, atoi(q_Hdr.ar_size));
                    
                    
//                    printf("h_buffer:%s \n", h_buffer);
                    //write header buffer to archive file
                    write(tmptxt, h_buffer, 58);
                    //mark end of header.
                    write(tmptxt, &ARFMAG, 2);
                    
                    
                    
                    if((readsize = read(fd, r_buffer, filesize))!=0){
//                        printf("readsize: %d", readsize);
//                        fflush(stdout);
                        write(tmptxt, r_buffer, readsize);
                    }
                }
                //Move the pointer to the later part
                //                lseek(fd,atoi(q_Hdr.ar_size),SEEK_CUR);
            }
        }else{
            fprintf(stderr,"usage:%s3\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }//
    else{
        fprintf(stderr,"usage:%s3\n",strerror(errno));
        exit(EXIT_FAILURE);
    }//
    close(fd);
    remove(argv[2]);
    rename(tmpFilename, argv[2]);
    
}




///////////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char * argv[]) {
    char *afile = argv[2];
    
    //    for(i =0; i<argc;i++){
    //        printf("arg[%d]: %s\n",i,argv[i]);
    //        fflush(stdout);
    //    }
    //    printf("arg0%s, arg1%s\n",argv[0], argv[1]);
    //    printf("arg2%s, arg3%s\n",argv[2], argv[3]);
    
    
    if(argc<3){
        fprintf(stderr,"usage4:%s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"-q") == 0 || strcmp(argv[1],"q") == 0){
        q_append(argc,argv);
    }else if(strcmp(argv[1],"-t") == 0 || strcmp(argv[1],"t") == 0){
        t_concise(argc, argv);
    }else if(strcmp(argv[1],"-tv") == 0 || strcmp(argv[1],"tv") == 0){
        tv_verbose(argc, argv);
    }else if(strcmp(argv[1],"-A") == 0 || strcmp(argv[1],"A") == 0){
        A_append(argc,argv);
    }else if(strcmp(argv[1],"-x") == 0 || strcmp(argv[1],"x") == 0){
        extract(argc, argv);
    }else if(strcmp(argv[1],"-xo") == 0 || strcmp(argv[1],"xo") == 0){
        extractXO(argc,argv);
    }else if(strcmp(argv[1],"-d") == 0 || strcmp(argv[1],"d") == 0){
        delete(argc,argv);
    }
    
    else{
//        printf("Error:The key is invalid\n");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}




