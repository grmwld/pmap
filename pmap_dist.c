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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "mpi.h"
#include "mpi_util.c"
#define PMAXLINE (1024*1024)
#define MPI_TAG 2009
void Partition_Reads (char *readsfile, char *outreadsfile, int nline_per_read, int nProc, int myProc, MPI_Comm comm);
int get_nline_per_read(char *readsfile);

/*#################################################################################*/
/*### MAIN ########################################################################*/    
int main (int argc, char *argv[]) {
    int i, j;
    char *readsfile, *readsfile2 = NULL, *indexdir=NULL, *indexprefix=NULL, *mainworkdir, *outdir, workdir[1000];
    char command[1000], usage[1000];
    int ninpfile = 0;
    char inpfile[10][1000], **ofname;    
    char fname1[500];
    int nline_per_read = -1;
    MPI_Comm comm=MPI_COMM_WORLD;
    int myProc, nProc;
    FILE *pfp;
    double times[5][2];

    //#############################################
    //### PARSE INPUT PARAMETERS ##################
    sprintf(usage, "Usage: %s <work directory> <output directory> <reads file> [-r <reads file2>] [-l <num.of lines per read>] [-i <index directory> <index prefix>] [-f <inpfile>]\n", argv[0]); 

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &nProc);
    MPI_Comm_rank(comm, &myProc);
    if (argc < 4) {
        if (myProc == 0)
            printf("%s", usage);
        exit(-1);
    }
    i=1;
    mainworkdir = argv[i++];
    outdir = argv[i++];
    readsfile = argv[i++];
    for ( ; i<argc ; i++) {
        if (!strcmp(argv[i], "-r")) {
            readsfile2 = argv[++i];                        
        } else if (!strcmp(argv[i], "-i")) {
            indexdir = argv[++i];
            indexprefix = argv[++i];
        } else if (!strcmp(argv[i], "-f")) {
            if (ninpfile > 10 && myProc == 0) {
                fprintf (stderr, "pmap_dist error: At most 10 inpfiles can be specified\n");
                exit(-1);
            }
            strcpy(inpfile[ninpfile++], argv[++i]);
        } else if (!strcmp(argv[i], "-l")) {
            nline_per_read = atoi(argv[++i]);
            if (nline_per_read <= 0 && myProc == 0) {
                fprintf(stderr, "pmap_dist error: Number of lines per read should be greater than 0\n");
                exit(-1);
            }
        } else if (myProc == 0) {
            fprintf (stderr, "pmap_dist error: Unknown option %s\n", argv[i]);
            exit(-1);
        }
    }    
    
    // Create a workdir, indexdir and outdir 
    sprintf(workdir, "%s/pmap_node%d", mainworkdir, myProc); 
    sprintf(command, "mkdir -p %s/index", workdir);
    system(command);
    if (myProc == 0) {
        sprintf(command, "mkdir -p %s", outdir);
        system(command);
    }

    // Check if files/folders exist
    if (myProc == 0) {
        if (indexdir && !folder_exists(indexdir)) {
            if (indexdir[0] != '/')
                fprintf(stderr, "pmap_dist error: Cannot find indexdir: %s. Make sure the path is an absolute path.\n", indexdir);
            else
                fprintf(stderr, "pmap_dist error: Cannot find indexdir: %s\n", indexdir);
            exit(-1);
        }
        if (!file_exists(readsfile)) {
            if (readsfile[0] != '/')
                fprintf(stderr, "pmap_dist error: Cannot find readsfile: %s. Make sure the path is an absolute path.\n", readsfile);
            else
                fprintf(stderr, "pmap_dist error: Cannot find readsfile: %s\n", readsfile);
            exit(-1);
        }
        if (readsfile2 && !file_exists(readsfile2)) {
            if (readsfile2[0] != '/')
                fprintf(stderr, "pmap_dist error: Cannot find readsfile2: %s. Make sure the path is an absolute path.\n", readsfile2);
            else
                fprintf(stderr, "pmap_dist error: Cannot find readsfile2: %s\n", readsfile2);
            exit(-1);
        }
        for (i=0; i<ninpfile; i++) {
            if (!file_exists(inpfile[i])) {
                if (inpfile[i][0] != '/')
                    fprintf(stderr, "pmap_dist error: Cannot find inpfile: %s. Make sure the path is an absolute path.\n", inpfile[i]);
                else
                    fprintf(stderr, "pmap_dist error: Cannot find inpfile: %s\n", inpfile[i]);
                exit(-1);
            }
        }        
        if (nline_per_read == -1) {
            nline_per_read = get_nline_per_read(readsfile);
        }
    }

    // Memory allocation
    ofname = (char **) malloc(sizeof(char *) * nProc);    
    for (i=0; i<nProc; i++) 
        ofname[i] = malloc(sizeof(char) * 1000);
            
    
    
    //#########################################################
    //### PARTITION/DISTRIBUTE THE DATA #######################
    times[1][0] = u_wseconds();
    sprintf(fname1, "%s/pmap.reads", workdir);
    Partition_Reads (readsfile, fname1, nline_per_read, nProc, myProc, comm);
    if (readsfile2) {
        sprintf(fname1, "%s/pmap.reads2", workdir);
        Partition_Reads (readsfile2, fname1, nline_per_read, nProc, myProc, comm);
    }
    times[1][1] = u_wseconds();    

    times[2][0] = u_wseconds();
    if (indexdir) {
        sprintf(fname1, "%s/index", workdir);
        Broadcast_Folder(indexdir, indexprefix, fname1, "index", nProc, myProc, comm);
    }
    times[2][1] = u_wseconds();    

    times[3][0] = u_wseconds();
    for (i=0; i<ninpfile; i++) {
        char *ptr = strrchr(inpfile[i], '/');
        for (j=0; j<nProc; j++) 
            sprintf(ofname[j], "%s/%s", workdir, ptr+1);
        Broadcast_File(inpfile[i], ofname, 0, myProc, comm);
    }
    times[3][1] = u_wseconds();    
    
    //#########################################################
    //### PRINT TIMING INFO AND FINALIZE ######################
    if (myProc == 0) {
        sprintf(fname1, "%s/dist_time.txt", outdir); 
        pfp = fopen(fname1, "w");           
        fprintf(pfp, "Partition reads     : %8.1lf\n", times[1][1]-times[1][0]);
        fprintf(pfp, "Replicate index     : %8.1lf\n", times[2][1]-times[2][0]);
        fprintf(pfp, "Replicate inpfile(s): %8.1lf\n", times[3][1]-times[3][0]);
        fclose(pfp);
    }            
    for (i=0; i<nProc; i++) 
        free(ofname[i]);
    free(ofname);    
    MPI_Finalize();
    return 0;    
}



/*#################################################################################*/
/*### PARTITION AND DISTRIBUTE READS FILE(S) ######################################*/
void Partition_Reads (char *readsfile, char *outreadsfile, int nline_per_read, int nProc, int myProc, MPI_Comm comm)
{    
    int i;
    int nlines;
    long nchars;
    unsigned int *xload, load;
    
    /* Allocation */
    xload = (unsigned int *) malloc(sizeof(unsigned int) * (nProc+1));
    
    /* Determine load distribution */
    if (myProc == 0) {        
        /* Compute load for each part */
        count_file_chars (readsfile, &nchars, &nlines, 1); //nchars is not reliable - file may be too big
        load = nlines/nProc;
        while (load % nline_per_read != 0) 
            load--;
        for (i=0; i<nProc; i++) 
            xload[i] = i*load;
        xload[nProc] = nlines;
    }
    
    /* Partition and scatter reads file */
    Partition_Scatter_1D (readsfile, outreadsfile, NULL, NULL, xload, nProc, myProc, comm);

    free(xload);
}

/* Determine reads file format and return the number of lines per read */
int get_nline_per_read(char *readsfile)
{
    FILE *ifp;
    char line[PMAXLINE];
    int nline_per_read;
    
    ifp = fopen(readsfile, "r");
    fgets(line, PMAXLINE, ifp);
    if (line[0] == '>')      /* Fasta */
        nline_per_read = 2;
    else if (line[0] == '@') /* Fastq */
        nline_per_read = 4;
    else {
        fprintf(stderr, "pmap_dist error: Could not understand reads file format in %s\n", readsfile);
        exit(-1);
    }            
    fclose(ifp);
    return nline_per_read;
}



