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

#define PMAXLINE (1024*1024)

#define BOWTIE 1
#define BWA 2
#define SOAP 3
#define GSNAP 4

/*##############################################################################*/
/*### FUNCTION FOR MEASURING TIME ##############################################*/    
double u_wseconds(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL); 
    return (double) tp.tv_sec + (double) tp.tv_usec / 1000000.0;
}

/*#################################################################################*/
/*### MAIN ########################################################################*/    
int main (int argc, char *argv[]) {
    int i;
    char progargs[1000], *genomefile, *indexprefix, *indexdir;
    char cmd[1000], usage[1000], fname[1000], *genomebasename, *extension;
    FILE *pfp, *fp;
    int program;
    double times[5][2];

    /*#############################################*/
    /*### PARSE INPUT PARAMETERS ##################*/
    sprintf(usage, "Usage: %s <genomefile> <indexdir> <indexprefix> <program> [programpars]\n", argv[0]);

    if (argc < 5) {
        printf("%s", usage);
        exit(-1);
    }
    genomefile = argv[1];
    indexdir = argv[2];
    sprintf(cmd, "mkdir -p %s", indexdir);
    system(cmd);
    indexprefix = argv[3];
    if (strstr(argv[4], "bowtie") != NULL)
        program = BOWTIE;
    else if (strstr(argv[4], "bwa") != NULL)
        program = BWA;
    else if (strstr(argv[4], "soap") != NULL)
        program = SOAP;
    else {
        printf("Error: Cannot find the requested mapping program\n");
        exit(-1);
    }     
    progargs[0] = '\0'; /* additional arguments of the mapping program */    
    for (i=5; i<argc; ++i) {
        strcat(progargs, " ");
        strcat(progargs, argv[i]);
    }
        
    /*#################################################################################*/
    /*### CALL THE PROGRAM ############################################################*/
    times[1][0] = u_wseconds();
    if (program == BOWTIE) {
        sprintf(cmd, "bowtie-build %s %s %s/%s", progargs, genomefile, indexdir, indexprefix);        
        system(cmd);                
    } else if (program == BWA) {
        sprintf(cmd, "bwa index %s -p %s/%s %s", progargs, indexdir, indexprefix, genomefile);
        system(cmd);        
    } else if (program == SOAP) { 
        sprintf(cmd, "2bwt-builder %s", genomefile);        
        system(cmd);
        /* Rename created index files using indexprefix */
        genomebasename = strrchr(genomefile, '/');
        genomebasename++;        
        sprintf(cmd, "ls %s.index*", genomefile);
        fp = popen(cmd, "r" );
        while ( fgets(fname, sizeof(fname), fp) != NULL ) {
            extension = strchr(fname, '\n');
            *extension = '\0'; //replace newline
            extension = strstr(fname, genomebasename);
            for (i=0; genomebasename[i] != '\0'; i++, extension++); //skip genomename
            extension += 6; // skip ".index"
            sprintf(cmd, "mv %s %s/%s%s", fname, indexdir, indexprefix, extension);
            system(cmd);
        }
    }  
    times[1][1] = u_wseconds();        

        
    /*#################################################################################*/
    /*### PRINT TIMING INFO AND FINALIZE ##############################################*/                
    sprintf(fname, "%s/index_time.txt", indexdir);
    pfp = fopen(fname, "a");
    fprintf(pfp, "Total time    : %8.1lf\n", times[1][1]-times[1][0]);
    fclose(pfp);
    
    return 0;
}
