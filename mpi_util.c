/*******************************************************************************/
/* Copyright (C) 2010 Doruk Bozdag                                             */
/*                                                                             */
/* This file is part of pMap.                                                  */
/* pMap is free software: you can redistribute it and/or modify                */
/* it under the terms of the Lesser GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or           */
/* (at your option) any later version.                                         */
/*                                                                             */ 
/* pMap is distributed in the hope that it will be useful,                     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of              */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               */
/* Lesser GNU General Public License for more details.                         */
/* <http://www.gnu.org/licenses/>.                                             */
/*******************************************************************************/  

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define COMM_BUFFER_SIZE (1*1024*1024*1024)
#define READ_BUFFER_SIZE (1*1024*1024*1024)

void Send_File(char *fname, int dest, int commtag, MPI_Comm comm);
void Receive_File(char *fname, int src, int commtag, MPI_Comm comm);
void Broadcast_File(char *ifname, char **ofname, int src, int myProc, MPI_Comm comm);
void Broadcast_Folder (char *idir, char *iprefix, char *odir, char *oprefix, int nProc, int myProc, MPI_Comm comm);
void Gather_Files(int np, int *proclist, char **ifname, char **ofname, int dest, int myProc, int commtag, MPI_Comm comm);
void Scatter_Files(int np, int *proclist, char **ifname, char **ofname, int src, int myProc, int commtag, MPI_Comm comm);
void Partition_Scatter_2D (int NR, char *readsfile, char *outreadsfile, char **header, int *headerlen, unsigned int *xload,
                           int nProc, int myProc, int myProcD1, int myProcD2, MPI_Comm commD1, MPI_Comm commD2);
void Partition_Scatter_1D (char *fname, char *outfname, char **header, int *headerlen,
                           unsigned int *xload, int nProc, int myProc, MPI_Comm comm);
void count_file_chars(char *fname, long *nchars, int *nlines, int lineflag);
double u_wseconds(void);
int file_exists (char *fname);
int folder_exists (char *fname);

/*##############################################################*/
/*### SEND A FILE TO A PROCESSOR ###############################*/
void Send_File(char *fname, int dest, int commtag, MPI_Comm comm) {
    long length;
    char *buffer;
    int fp;
    
    if ((fp = open(fname, O_RDONLY))==-1) {
        fprintf(stderr, "File read error %s\n", fname);
        exit(-1);
    }
    
    buffer = (char *) malloc (COMM_BUFFER_SIZE);    
    while ( (length = read(fp, buffer, COMM_BUFFER_SIZE))> 0 ) {
        MPI_Send(&length, 1, MPI_LONG, dest, commtag, comm);
        MPI_Send(buffer, length, MPI_CHAR, dest, commtag, comm);
    }
    length = 0;
    MPI_Send(&length, 1, MPI_LONG, dest, commtag, comm);

    close(fp);
    free(buffer);
}

/*##############################################################*/
/*### RECEIVE A FILE FROM A PROCESSOR ##########################*/
void Receive_File(char *fname, int src, int commtag, MPI_Comm comm) {
    long length;
    char *buffer;
    int fp;
    MPI_Status st;

    if ((fp = creat(fname, S_IRWXU))==-1) 
        if ((fp = open(fname, O_WRONLY))==-1) {
            fprintf(stderr, "File write error for %s\n", fname);
            exit (-1);
        }

    buffer = (char *) malloc (COMM_BUFFER_SIZE);    
    while (1) {        
        MPI_Recv (&length, 1, MPI_LONG, src, commtag, comm, &st);
        if (length <= 0) 
            break;
        MPI_Recv(buffer, length, MPI_CHAR, src, commtag, comm, &st);
        if (write(fp, buffer, length) <= 0) {
            fprintf(stderr, "File write error for %s\n", fname);
        }
    }
    
    close(fp);
    free(buffer);
}

/*##############################################################*/
/*### BROADCAST A FILE #########################################*/
void Broadcast_File(char *ifname, char **ofname, int src, int myProc, MPI_Comm comm) {
    long length;
    char *buffer, cmd[1000];
    int fp;
    
    buffer = (char *) malloc (COMM_BUFFER_SIZE);    
    if (myProc == src) {
        if ((fp = open(ifname, O_RDONLY))==-1) {
            fprintf(stderr, "File read error %s\n", ifname);
            exit(-1);
        }
        
        while ( (length = read(fp, buffer, COMM_BUFFER_SIZE))> 0 ) {
            MPI_Bcast (&length, 1, MPI_LONG, src, comm);
            MPI_Bcast (buffer, length, MPI_CHAR, src, comm);
        }
        length = 0;
        MPI_Bcast (&length, 1, MPI_LONG, src, comm);

        if (strcmp(ifname, ofname[myProc]) != 0) {
            sprintf(cmd, "cp -f %s %s", ifname, ofname[myProc]);
            system(cmd);
        }
        
    } else {
        if ((fp = creat(ofname[myProc], S_IRWXU))==-1)
            if ((fp = open(ofname[myProc], O_WRONLY))==-1) {
                fprintf(stderr, "File write error %s", ofname[myProc]);
                exit(-1);
            }        
        while(1) {
            MPI_Bcast (&length, 1, MPI_LONG, src, comm);
            if (length <= 0)
                break;            
            MPI_Bcast (buffer, length, MPI_CHAR, src, comm); 
            write(fp, buffer, length);
        }
    }
    close(fp);
    free(buffer);
}


/*##############################################################*/
/*### BROADCAST A FOLDER RECURSIVELY ###########################*/
void Broadcast_Folder (char *idir, char *iprefix, char *odir, char *oprefix, int nProc, int myProc, MPI_Comm comm)
{
    int i, type;
    char **ofname, suffix[1000];
    char cmd[1000], ifname[1000], ifnamefull[1000], *ptr, *start;
    FILE * fp;


    ofname = (char **) malloc(sizeof(char *) * nProc);    
    for (i=0; i<nProc; i++) 
        ofname[i] = malloc(sizeof(char) * 1000);
    sprintf(cmd, "mkdir -p %s", odir);
    system(cmd);

    if (myProc == 0) {        
        sprintf(cmd, "ls %s/", idir);
        fp = popen(cmd, "r" );
        while ( fgets(ifname, sizeof(ifname), fp) != NULL ) {            
            ptr = strrchr(ifname, '\n');
            *ptr = '\0'; //replace newline            
            ptr = strrchr(ifname, '/');
            start = (ptr == NULL) ? ifname : ptr+1; //find the start of iprefix            

            for (i=0; iprefix[i] != '\0'; i++) //check if file/foldername starts with the iprefix
                if (iprefix[i] != start[i]) 
                    break;
            if (iprefix[i] != '\0')
                continue;
            ptr = start + i; //skip the prefix                                

            sprintf(ifnamefull, "%s/%s", idir, start); //to make sure ifname path is correct
            type = folder_exists(ifnamefull) ? 1 : 0;
            MPI_Bcast(&type, 1, MPI_INT, 0, comm);     //let others know what will be sent 
            MPI_Bcast(ptr, 1000, MPI_CHAR, 0, comm);   //send suffix of output file/folder name to others
            sprintf(ofname[myProc], "%s/%s%s", odir, oprefix, ptr);
            if (type == 0)                                       //send file to others
                Broadcast_File(ifnamefull, ofname, 0, myProc, comm); 
            else                                                 //send folder to others
                Broadcast_Folder(ifnamefull, "", ofname[myProc], "", nProc, myProc, comm);
        }
        pclose(fp);
        type = -1; 
        MPI_Bcast(&type, 1, MPI_INT, 0, comm);         //let others know that nothing else will be sent
    } else {
        while(1) {
            MPI_Bcast(&type, 1, MPI_INT, 0, comm);
            if (type == -1) 
                break;            
            MPI_Bcast(suffix, 1000, MPI_CHAR, 0, comm);
            sprintf(ofname[myProc], "%s/%s%s", odir, oprefix, suffix);
            if (type == 0) 
                Broadcast_File(NULL, ofname, 0, myProc, comm); 
            else
                Broadcast_Folder(NULL, NULL, ofname[myProc], "", nProc, myProc, comm);
        }
    }
    for (i=0; i<nProc; i++) 
        free(ofname[i]);
    free(ofname);
}
    



/*##############################################################*/
/*### GATHER FILES #############################################*/
void Gather_Files(int np, int *proclist, char **ifname, char **ofname, int dest, int myProc, int commtag, MPI_Comm comm) {   
    int i;
    char command[200];
    
    if (myProc != dest) {
        for (i=0; i<np; i++) {
            if (proclist[i] == myProc) {
                Send_File(ifname[i], dest, commtag, comm);
                break;
            }
        }
    } else {        
        for (i=0; i<np; i++) {
            if (proclist[i] == myProc) {
                if (strcmp(ifname[i], ofname[i]) != 0) {
                    sprintf(command, "cp -f %s %s", ifname[i], ofname[i]);
                    system(command);
                }
            } else 
                Receive_File(ofname[i], proclist[i], commtag, comm);        
        }
    }    
}

/*##############################################################*/
/*### SCATTER FILES #############################################*/
void Scatter_Files(int np, int *proclist, char **ifname, char **ofname, int src, int myProc, int commtag, MPI_Comm comm) {   
    int i;
    char command[200];
    
    if (myProc != src) {
        for (i=0; i<np; i++) {
            if (proclist[i] == myProc) {
                Receive_File(ofname[i], src, commtag, comm);
                break;
            }
        }
    } else {        
        for (i=0; i<np; i++) {
            if (proclist[i] == myProc) {
                if (strcmp(ifname[i], ofname[i]) != 0) {
                    sprintf(command, "cp -f %s %s", ifname[i], ofname[i]);
                    system(command);
                }
            } else 
                Send_File(ifname[i], proclist[i], commtag, comm);
        }
    }    
}


/*###############################################################################*/
/* PARTITION A FILE, DISTRIBUTE IN DIM-1, THEN REPLICATE IN DIM-2 (MULTICAST) ###*/
void Partition_Scatter_2D (int NX, char *fname, char *outfname, char **header, int *headerlen, unsigned int *xload,
                           int nProc, int myProc, int myProcD1, int myProcD2, MPI_Comm commD1, MPI_Comm commD2) {
    int rfp, wfp;
    unsigned int cnt;
    unsigned int length, blength;
    char *buffer;
    unsigned int i, j;
    unsigned int start;
    MPI_Status status;
    int mpi_tag = 2010;
    
    buffer = (char *) malloc(COMM_BUFFER_SIZE); //dbdb

    /* Open output file */
    if ((wfp = creat(outfname, S_IRWXU))==-1) {
        if ((wfp = open(outfname, O_WRONLY))==-1) {
            fprintf(stderr, "[%d] Error opening write file %s\n", myProc, outfname);
            exit (-1);
        }
    }

    /* Distribute and print headers if they exist */
    if (header != NULL) {
        MPI_Bcast(headerlen, NX, MPI_INT, 0, commD1);
        MPI_Bcast(headerlen, NX, MPI_INT, 0, commD2);
        for (i=0; i<NX; i++) {
            MPI_Bcast(header[i], headerlen[i], MPI_CHAR, 0, commD1);
            if (i == myProcD1)
                MPI_Bcast(header[i], headerlen[i], MPI_CHAR, 0, commD2);
        }
        write(wfp, header[myProcD1], headerlen[myProcD1]);        
    }
        
    /* Partition and scatter the file */
    if (myProc == 0) {
        rfp = open(fname, O_RDONLY);        
        for (j=0, cnt=0; (length = read(rfp, buffer, COMM_BUFFER_SIZE))> 0; ) {
            start = 0;
            for (i=0; i<length; i++) {
                if (buffer[i] == '\n') {
                    cnt++;                    
                    if (cnt == xload[j+1] - xload[j]) {
                        blength = i-start+1;
                        if (j >= NX) {
                            printf("Error: There's a problem with the input file. Attempt to use more than NX partitions\n");
                            exit(-1);                        
                        } else if (j != 0) {
                            MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, commD1);
                            MPI_Send(buffer+start, blength, MPI_CHAR, j, mpi_tag, commD1);
                            blength = 0;
                            MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, commD1); /* Mark the end of the part to be sent to proc p */
                        } else {
                            MPI_Bcast (&blength, 1, MPI_LONG, 0, commD2);
                            MPI_Bcast (buffer+start, blength, MPI_CHAR, 0, commD2);
                            if (write(wfp, buffer+start, blength) <= 0) {
                                fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                                exit(-1);
                            }
                            blength = 0;
                            MPI_Bcast (&blength, 1, MPI_LONG, 0, commD2);
                        }
                        j++;
                        cnt = 0;
                        start = i+1;
                    }
                }
            }
            if (start < length) {
                blength = length-start;
                if (j!=0) {
                    MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, commD1);
                    MPI_Send(buffer+start, blength, MPI_CHAR, j, mpi_tag, commD1);
                } else {
                    MPI_Bcast (&blength, 1, MPI_LONG, 0, commD2);
                    MPI_Bcast (buffer+start, blength, MPI_CHAR, 0, commD2);
                    if (write(wfp, buffer+start, blength) <= 0) {
                        fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                        exit(-1);
                    }
                }
            }            
        }
        close(rfp);
    } else if (myProcD2 == 0) {        
        while (1) {        
            MPI_Recv (&blength, 1, MPI_LONG, 0, mpi_tag, commD1, &status);
            MPI_Bcast (&blength, 1, MPI_LONG, 0, commD2);
            if (blength <= 0) 
                break;
            MPI_Recv(buffer, blength, MPI_CHAR, 0, mpi_tag, commD1, &status);
            MPI_Bcast (buffer, blength, MPI_CHAR, 0, commD2);
            if (write(wfp, buffer, blength) <= 0) {
                fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                exit(-1);
            }
        }
    } else {
        while (1) {
            MPI_Bcast (&blength, 1, MPI_LONG, 0, commD2);
            if (blength <= 0)
                break;            
            MPI_Bcast (buffer, blength, MPI_CHAR, 0, commD2);
            if (write(wfp, buffer, blength) <= 0) {
                fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                exit(-1);
            }
        }
    }
    free(buffer);
    close(wfp);
}



/*###############################################################################*/
/* PARTITION A FILE, THEN SCATTER ###*/
void Partition_Scatter_1D (char *fname, char *outfname, char **header, int *headerlen,
                           unsigned int *xload, int nProc, int myProc, MPI_Comm comm)                           
{
    int rfp, wfp;
    unsigned int cnt;
    unsigned int length, blength;
    char *buffer;
    unsigned int i, j;
    unsigned int start;
    MPI_Status status;
    int mpi_tag = 2010;
    
    buffer = (char *) malloc(COMM_BUFFER_SIZE);

    /* Open output file */
    if ((wfp = creat(outfname, S_IRWXU))==-1) {
        if ((wfp = open(outfname, O_WRONLY))==-1) {
            fprintf(stderr, "[%d] Error opening write file %s\n", myProc, outfname);
            exit (-1);
        }
    }

    /* Distribute and print headers if they exist */
    if (header != NULL) {
        MPI_Bcast(headerlen, nProc, MPI_INT, 0, comm);
        for (i=0; i<nProc; i++) 
            MPI_Bcast(header[i], headerlen[i], MPI_CHAR, 0, comm);
        write(wfp, header[myProc], headerlen[myProc]);        
    }
    
    /* Partition and scatter the file */
    if (myProc == 0) {
        rfp = open(fname, O_RDONLY);        
        for (j=0, cnt=0; (length = read(rfp, buffer, COMM_BUFFER_SIZE))> 0; ) {
            start = 0;
            for (i=0; i<length; i++) {
                if (buffer[i] == '\n') {
                    cnt++;                    
                    if (cnt == xload[j+1] - xload[j]) {
                        blength = i-start+1;
                        if (j >= nProc) {
                            printf("Error: There's a problem with the input file. Attempt to use more than nProc partitions\n");
                            exit(-1);                        
                        } else if (j != 0) {
                            MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, comm);
                            MPI_Send(buffer+start, blength, MPI_CHAR, j, mpi_tag, comm);
                            blength = 0;
                            MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, comm); /* Mark the end of the part to be sent to proc p */
                        } else {
                            if (write(wfp, buffer+start, blength) <= 0) {
                                fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                                exit(-1);
                            }
                        }
                        j++;
                        cnt = 0;
                        start = i+1;
                    }
                }
            }
            if (start < length) {
                blength = length-start;
                if (j != 0) {
                    MPI_Send(&blength, 1, MPI_LONG, j, mpi_tag, comm);
                    MPI_Send(buffer+start, blength, MPI_CHAR, j, mpi_tag, comm);
                } else {
                    if (write(wfp, buffer+start, blength) <= 0) {
                        fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                        exit(-1);
                    }
                }
            }          
        }
        close(rfp);
    } else {
        while (1) {        
            MPI_Recv (&blength, 1, MPI_LONG, 0, mpi_tag, comm, &status);
            if (blength <= 0) 
                break;
            MPI_Recv(buffer, blength, MPI_CHAR, 0, mpi_tag, comm, &status);
            if (write(wfp, buffer, blength) <= 0) {
                fprintf(stderr, "[%d] File write error for %s\n", myProc, outfname);
                exit(-1);
            }
        }
    }
    free(buffer);
    close(wfp);
}








/*#################################################################################*/
/*### UTILITY FUNCTIONS ###########################################################*/
void count_file_chars(char *fname, long *nchars, int *nlines, int lineflag) { /* Similar to wc but faster */
    int fp;
    int length, i;
    char *buffer;
    
    if ((fp = open(fname, O_RDONLY))==-1) {
        fprintf(stderr, "File read error %s\n", fname);
        exit(-1);
    }    
    buffer = (char *) malloc (READ_BUFFER_SIZE);
    for (*nlines=0, *nchars=0; (length = read(fp, buffer, READ_BUFFER_SIZE))> 0; ) {        
        if (lineflag) {
            for (i=0; i<length; i++) {
                if (buffer[i] == '\n')
                    (*nlines)++;
            }
        }
        *nchars += length;
    }
    close(fp);
    free(buffer);
}
double u_wseconds(void) { /* Measure time */
    struct timeval tp;
    gettimeofday(&tp, NULL); 
    return (double) tp.tv_sec + (double) tp.tv_usec / 1000000.0;
}
int file_exists (char *fname) { /* Check existance of a file */
    FILE *pfp;    
    if ((pfp = fopen(fname, "r"))) {        
        fclose(pfp);
        return 1;
    } else 
        return 0;
}
int folder_exists (char *fname) { /* Check existance of a folder */
    DIR *pfp;    
    if ((pfp = opendir(fname))) {        
        closedir(pfp);
        return 1;
    } else 
        return 0;
}
