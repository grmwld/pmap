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

#define BOWTIE 1
#define BWA 2
#define SOAP 3
#define GSNAP 4
#define MAQ 5
#define RMAP 6

/*#################################################################################*/
/*### MAIN ########################################################################*/    
int main (int argc, char *argv[]) {
    int i, ncmd;
    char progargs[1000], workdir[1000], outfile[50]="pmapoutfile.out.txt", *mainworkdir, *outdir, indexprefix[1000]="index", indexdir[1000]="", convargs[1000];
    char cmd[10][1000], usage[1000], fname[1000];
    int cleanup=1, paired_end=0, shared_index=0, dry=0;
    FILE *pfp, *fp;
    int program;
    MPI_Comm comm=MPI_COMM_WORLD;
    int myProc, nProc;
    int *proclist;
    char **ifname, **ofname, *ptr;
    double times[5][2];
    double *alltimes, maxtime, avgtime;

    /*#############################################*/
    /*### PARSE INPUT PARAMETERS ##################*/
    sprintf(usage, "Usage: %s [-pe] [-dc] [-dry] [-i <indexdir> <indexprefix>] <workdir> <outdir> <program> [programpars] [-conversion <convpars>]\n", argv[0]);
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &nProc);
    MPI_Comm_rank(comm, &myProc);
    if (argc < 4) {
        if (myProc == 0)
            printf("%s", usage);
        exit(-1);
    }
    for (i=1; ; i++) {
        if (!strcmp(argv[i], "-dc")) 
            cleanup = 0;
        else if (!strcmp(argv[i], "-dry"))
            dry = 1;
        else if (!strcmp(argv[i], "-pe")) 
            paired_end = 1;
        else if (!strcmp(argv[i], "-i")) {
            shared_index = 1;
            strcpy(indexdir, argv[++i]);
            strcpy(indexprefix, argv[++i]);
        }
        else
            break;        
    }    
    mainworkdir = argv[i++];
    sprintf(workdir, "%s/pmap_node%d", mainworkdir, myProc);
    if (!folder_exists(workdir)) {
        fprintf(stderr, "[Node %d] pmap error: Cannot find local workdir\n", myProc);
        exit(-1);
    }    
    if (!strcmp(indexdir, "")) {
        sprintf(indexdir, "%s/index", workdir);
        strcpy(indexprefix, "index");
    } else if (!folder_exists(indexdir)) {
        if (indexdir[0] != '/')
            fprintf(stderr, "[Node %d] pmap error: Cannot find indexdir: %s. Make sure the path is an absolute path.\n", myProc, indexdir);
        else
            fprintf(stderr, "[Node %d] pmap error: Cannot find indexdir: %s\n", myProc, indexdir);
        exit(-1);
    }    
    outdir = argv[i++];
    sprintf(cmd[0], "mkdir -p %s", outdir);
    system(cmd[0]);
    if (strstr(argv[i], "bowtie") != NULL) {
        program = BOWTIE;
        if (!shared_index) {
            printf("[Node %d] pmap error: Bowtie program cannot be executed without using -i option of pmap.\n", myProc);
            exit(-1);
        }
    }
    else if (strstr(argv[i], "bwa") != NULL)
        program = BWA;    
    else if (strstr(argv[i], "soap") != NULL)
        program = SOAP;
    else if (strstr(argv[i], "gsnap") != NULL)
        program = GSNAP;
    else if (strstr(argv[i], "maq") != NULL)
        program = MAQ;
    else if (strstr(argv[i], "rmap") != NULL)
        program = RMAP;
    else {
        printf("pmap error: Cannot find the requested mapping program: %s\n", argv[i]);
        exit(-1);
    }    
    progargs[0] = '\0'; /* additional arguments of the mapping program */        
    for (++i ; i<argc; ++i) {
        if (!strcmp(argv[i], "-conversion"))
            break;
        strcat(progargs, " ");
        if (strstr(argv[i], "outfile:") == argv[i]) { //Check if an output file is requested
            sprintf(fname, "%s/pmapoutfile.%s", workdir, argv[i]+8); //Replace prefix "outfile:" so that the file will be gathered
            strcat(progargs, fname);        
        } else if (strstr(argv[i], "infile:") == argv[i]) { //Check if an input file is specified
            sprintf(fname, "%s/%s", workdir, argv[i]+7); //Remove prefix "infile:"
            strcat(progargs, fname);      
        } else 
            strcat(progargs, argv[i]);        
    }
    convargs[0] = '\0'; /* additional arguments of the output conversion program */        
    for (++i ; i<argc; ++i) {
        strcat(convargs, " ");
        strcat(convargs, argv[i]);
    }
    
        
    /*#################################################################################*/
    /*### CALL THE MAPPING PROGRAM ####################################################*/
    times[1][0] = u_wseconds();
    ncmd = 0;
    if (program == BOWTIE) {
        if (paired_end)   
            sprintf(cmd[ncmd++], "bowtie %s %s/%s -1 %s/pmap.reads -2 %s/pmap.reads2 %s/%s", progargs, indexdir, indexprefix, workdir, workdir, workdir, outfile);
        else              
            sprintf(cmd[ncmd++], "bowtie %s %s/%s %s/pmap.reads %s/%s", progargs, indexdir, indexprefix, workdir, workdir, outfile);
    } else if (program == BWA) {
        if (paired_end) { 
            sprintf(cmd[ncmd++], "bwa aln %s %s/%s %s/pmap.reads > %s/pmap.out.sai", progargs, indexdir, indexprefix, workdir, workdir);
            sprintf(cmd[ncmd++], "bwa aln %s %s/%s %s/pmap.reads2 > %s/pmap.out2.sai", progargs, indexdir, indexprefix, workdir, workdir);            
            sprintf(cmd[ncmd++], "bwa sampe %s %s/%s %s/pmap.out.sai %s/pmap.out2.sai %s/pmap.reads %s/pmap.reads2 > %s/pmapoutfile.out.txt", convargs, indexdir, indexprefix, workdir, workdir, workdir, workdir, workdir);
	} else {          
            sprintf(cmd[ncmd++], "bwa aln %s %s/%s %s/pmap.reads > %s/pmap.out.sai", progargs, indexdir, indexprefix, workdir, workdir);
            sprintf(cmd[ncmd++], "bwa samse %s %s/%s %s/pmap.out.sai %s/pmap.reads > %s/pmapoutfile.out.txt", convargs, indexdir, indexprefix, workdir, workdir, workdir);
        }
    } else if (program == SOAP) {
        if (paired_end)   
            sprintf(cmd[ncmd++], "soap -D %s/%s -a %s/pmap.reads -b %s/pmap.reads2 -o %s/pmapoutfile.out.txt %s", indexdir, indexprefix, workdir, workdir, workdir, progargs);
        else              
            sprintf(cmd[ncmd++], "soap -D %s/%s -a %s/pmap.reads -o %s/pmapoutfile.out.txt %s", indexdir, indexprefix, workdir, workdir, progargs);        
    } else if (program == GSNAP) { /* Single file and the same call for paired end and single end reads */
        ptr = strrchr(indexdir, '/'); /* Remove the directory name, that is the same as the indexprefix */
        ptr = '\0';
        sprintf(cmd[ncmd++], "gsnap %s -D %s -d %s %s/pmap.reads > %s/pmapoutfile.out.txt", progargs, indexdir, indexprefix, workdir, workdir);
    } else if (program == MAQ) {
        sprintf(cmd[ncmd++], "maq fasta2bfa %s/%s %s/index/index.bfa", indexdir, indexprefix, workdir); /* Genome conversion */
        sprintf(indexdir, "%s/index", workdir); /* In case the genome (current value of indexdir) is at a shared location */
        strcpy(indexprefix, "index.bfa");   
        if (paired_end) {
            sprintf(cmd[ncmd++], "maq fastq2bfq %s/pmap.reads %s/pmap.reads.bfq", workdir, workdir);
            sprintf(cmd[ncmd++], "maq fastq2bfq %s/pmap.reads2 %s/pmap.reads2.bfq", workdir, workdir);
            sprintf(cmd[ncmd++], "maq map %s %s/pmap.map %s/%s %s/pmap.reads.bfq %s/pmap.reads2.bfq", progargs, workdir, indexdir, indexprefix, workdir, workdir);
            sprintf(cmd[ncmd++], "maq mapview %s %s/pmap.map > %s/pmapoutfile.out.txt", convargs, workdir, workdir);
        } else {               
            sprintf(cmd[ncmd++], "maq fastq2bfq %s/pmap.reads %s/pmap.reads.bfq", workdir, workdir);
            sprintf(cmd[ncmd++], "maq map %s %s/pmap.map %s/%s %s/pmap.reads.bfq", progargs, workdir, indexdir, indexprefix, workdir);
            sprintf(cmd[ncmd++], "maq mapview %s %s/pmap.map > %s/pmapoutfile.out.txt", convargs, workdir, workdir);
        }
    } else if (program == RMAP) {
        if (paired_end)        
            sprintf(cmd[ncmd++], "rmappe %s -c %s/%s -o %s/pmapoutfile.out.txt -Q %s/pmap.reads", progargs, indexdir, indexprefix, workdir, workdir);
        else                   
            sprintf(cmd[ncmd++], "rmap %s -c %s/%s -o %s/pmapoutfile.out.txt -Q %s/pmap.reads", progargs, indexdir, indexprefix, workdir, workdir);
    }

    /* Execute the commands */
    for (i=0; i<ncmd; i++) { 
        if (myProc == 0) 
            printf("pmap: executing in parallel: %s\n", cmd[i]);
        system(cmd[i]);
    }

    times[1][1] = u_wseconds();        


    /*#################################################################################*/
    /*### GATHER OUTPUT ###############################################################*/                
    times[2][0] = u_wseconds();        
    proclist = (int *) malloc (sizeof (int) * nProc); 
    ifname = (char **) malloc(sizeof(char *) * nProc);
    ofname = (char **) malloc(sizeof(char *) * nProc);
    for (i=0; i<nProc; i++) {
        ifname[i] = malloc(sizeof(char) * 1000);
        ofname[i] = malloc(sizeof(char) * 1000);
    }
    
    if (!dry) {
        sprintf(cmd[0], "ls %s/pmapoutfile.*", workdir);
        fp = popen(cmd[0], "r" );
        while ( fgets(fname, sizeof(fname), fp) != NULL ) {
            ptr = strchr(fname, '\n');
            *ptr = '\0'; //replace newline
            ptr = strrchr(fname, '/');
            ptr++;       //start of prefix

            sprintf(cmd[0], "cat ");                
            for (i=0; i<nProc; i++) {
                sprintf(ifname[i], "%s/%s", workdir, ptr);
                sprintf(ofname[i], "%s/%s.%d", mainworkdir, ptr, i);
                proclist[i] = i;
                sprintf(cmd[0], "%s %s", cmd[0], ofname[i]);
            }
            Gather_Files(nProc, proclist, ifname, ofname, 0, myProc, MPI_TAG, comm);
            sprintf(cmd[0], "%s > %s/%s", cmd[0], outdir, ptr+12); // Ignore "pmapoutfile." prefix
            if (myProc == 0)
                system(cmd[0]);
        }
        pclose(fp);
    }

    /* Remove all temporary files */
    if (cleanup) { 
        sprintf(cmd[0], "/bin/rm -rf %s/*; /bin/rmdir %s", workdir, workdir);        
        system(cmd[0]);
        if (myProc == 0) {
            sprintf(cmd[0], "/bin/rm -f %s/pmapoutfile*", mainworkdir);
            system(cmd[0]);
        }            
    }
    times[2][1] = u_wseconds();

    
    /*#################################################################################*/
    /*### PRINT TIMING INFO AND FINALIZE ##############################################*/                
    times[1][1] -= times[1][0];
    alltimes = (double *) malloc(sizeof(double) * nProc);
    MPI_Gather (&times[1][1], 1, MPI_DOUBLE, alltimes, 1, MPI_DOUBLE, 0, comm);
    if (myProc == 0) {
        sprintf(fname, "%s/map_time.txt", outdir);
        pfp = fopen(fname, "wt");
        maxtime = avgtime = 0;
        for (i=0; i<nProc; i++) {
            if (maxtime < alltimes[i])
                maxtime = alltimes[i];
            avgtime += alltimes[i];
        }
        avgtime /= nProc;
        fprintf(pfp, "Mapping time     : %8.1lf\n", maxtime);
        fprintf(pfp, "Load imbalance   : %8.1lf%%\n", 100*(maxtime-avgtime)/avgtime);
        for (i=0; i<nProc; i++) 
            fprintf(pfp, "[%2d]: %8.1lf\n", i, alltimes[i]);
        fprintf(pfp, "Output merge time: %8.1lf\n", times[2][1]-times[2][0]);
        fclose(pfp);
    }
    
    MPI_Finalize();
    free(alltimes);
    free(proclist);
    for (i=0; i<nProc; i++) {
        free(ifname[i]);
        free(ofname[i]);
    }
    free(ifname);
    free(ofname);

    
    return 0;
}









