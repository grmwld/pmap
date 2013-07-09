The following information can be also found at: http://bmi.osu.edu/hpc/software/pmap/pmap.html


*General information

pMap is an MPI-based tool to parallelize the alignment step of state-of-the-art sequence mapping programs. It allows transparent execution of the alignment step of a selected program in parallel on a compute-cluster. The results are the same as those from a sequential run of the selected mapping program, except for differences due to random choices within the mapping program. The parallel execution using pMap is approximately NxP times faster, where N is the number of nodes and P is the number of cores/threads per node. Currently, pMap supports BWA, SOAP, Bowtie, GSNAP, MAQ and RMAP.

pMap divides the workload by assigning approximately R/N reads to each node, where R is the number of reads provided in a formatted input file (e.g. FASTA or FASTQ). Moreover, it replicates the pre-built index file(s) associated with the reference genome on each node. The selected sequence mapping program is executed independenty to map the reads assigned to each node, and finally, the output files are combined and written to a desired location. pMap consists of three subprograms corresponding to different steps of the parallel mapping process: pmap_index, pmap_dist and pmap.

    * pmap_index corresponds to the indexing step of the sequence mapping programs. pmap_index runs sequentially on a single node and generates an index for a given mapping program and a reference genome. pmap_index is included for completeness and to provide an interface similar to pmap_dist and pmap. Any pre-built index can be used without having to use pmap_index.
    * pmap_dist is used to partition the input reads file(s) and distribute them to the nodes of the cluster. If requested, the index files and any other input files required by the selected mapping program can also be replicated using pmap_dist.
    * pmap runs the selected mapping program independently on each node to map the reads distributed by pmap_dist, and combines the generated output files. 
-------------------------------------------------------------------------------------------------------------------------
*Download

Latest release: pMap-11-25-2010.tar.gz
-------------------------------------------------------------------------------------------------------------------------

*Compiling

After extracting the archive, change the default compilers mpicc and gcc in the Makefile, if desired. Then, run make to create three executables: pmap_index, pmap_dist and pmap. Make sure that the mapping programs to be used with pMap are included in the $PATH.
Usage
-------------------------------------------------------------------------------------------------------------------------

*pmap_index

pmap_index <genomefile> <indexdir> <indexprefix> <program> [programpars]

genomefile: file that contains the genome/chromosome sequences in FASTA format.
indexdir: directory in which output index files will be written
indexprefix: the prefix of the index files in indexdir
program: the sequence mapping program to be called (bowtie, bwa or soap)
programpars: additional parameters to be passed to program

When pmap_index is called:

    * The genome/chromosome sequences in the genomefile are indexed using the selected program. When the program is chosen as bowtie, bwa and soap, respectively bowtie-build, bwa index and 2bwt-builder are executed with optional parameters specified in programpars.
    * The output index files are written in indexdir. All of the index files start with the prefix indexprefix.
    * The timing information is written in index_time.txt file in indexdir. 

* Example: Generate index files for Bowtie
#!/bin/bash
genomefile=/home/myuser/mygenome.fa
indexdir=/home/myuser/myindexdir
indexprefix=myindexprefix
pmap_index $genomefile $indexdir $indexprefix bowtie
#Sequential equivalent is:
bowtie-build $genomefile $indexdir/$indexprefix

* Example: Generate index files for BWA with "-a bwtsw" option
#!/bin/bash
genomefile=/home/myuser/mygenome.fa
indexdir=/home/myuser/myindexdir
indexprefix=myindexprefix
pmap_index $genomefile $indexdir $indexprefix bwa -a bwtsw
#Sequential equivalent is:
bwa index -a bwtsw -p $indexdir/$indexprefix $genomefile
---------------------------------------------------------------------------------------------------------------------------

*pmap_dist

pmap_dist <workdir> <outdir> <readsfile> [-r readsfile2] [-l num_lines_per_read] [-i indexdir indexprefix] [-f inpfile]

workdir: local directory in which intermediate files will be written
outdir: directory on the head node in which output files will be written
readsfile: the reads file on the head node that contains the reads to be mapped
readsfile2: the second reads file on the head node needed when mapping paired end reads
num_lines_per_read: number of lines per read in readsfile and readsfile2
indexdir: shared directory in which index files are stored
indexprefix: the prefix of the index files in indexdir
inpfile: an input file on the head node that contains additional information needed during mapping

When pmap_dist is called:

    * A local workdir is created on each node. On the head node, outdir is also created.
    * The readsfile (as well as readsfile2 if -r option is used) on the head node is partitioned into as many parts as the number of nodes. Parts are distributed and written to local workdir on respective nodes. The granularity of partitioning is num_lines_per_read. If -l option is not used, num_lines_per_read is automatically set to 2 for FASTA files and 4 for FASTQ files.
    * If -i option is used, all index files and folders starting with the indexprefix in indexdir on the head node are replicated in workdir on each node.
    * If -f option is used, inpfile on the head node is replicated in workdir on each node. Multiple input files can be replicated by specifying each file with a separate -f.
    * The timing information is written in dist_time.txt file in outdir. 

Please also see pmap examples below.
----------------------------------------------------------------------------------------------------------------------------

*pmap

pmap [options] <workdir> <outdir> <program> [programpars] [-conversion <convpars>]
options: pmap has the following optional parameters:
       -pe: paired end mapping. readsfile2 must have been distributed by pmap_dist if needed by the program.
-dc: don't cleanup intermediate files in workdir after the execution of pmap completes.
-i <indexdir> <indexprefix>: use index files from a shared directory. The index files are in indexdir and file names start with indexprefix. If -i is not specified, index files must have been replicated using -i option of pmap_dist

workdir: local work directory created by pmap_dist
outdir: directory on the head node in which output files will be written
program: the sequence mapping program to be called (bowtie, bwa, soap, gsnap, maq or rmap)
programpars: additional parameters to be passed to program. If programpars includes an output file, the output file name should be specified in outfile:filename format. If programpars includes an input file which is not to be read from a shared location, the input file should be replicated using the -f <inpfile> option of pmap_dist. Then, in programpars, the input file name should be specified in infile:filename format. For both input and output files, filename is just the name of the file, not a path.
convpars: additional paramaters of the conversion program, if any, that is used to convert intermediate output files from program-native format to human-readable format. [-conversion <convpars>] should be the last parameters of pmap

When pmap is called:

    * outdir is created on the head node if it doesn't already exist.
    * program with optional parameters in programpars is called independently on each node to map reads in workdir that were distributed by pmap_dist. If -i option is used, the index files in indexdir that start with indexprefix are used for mapping. Otherwise, the index files in the workdir that were replicated using -i option of pmap_dist are used.
    * The mapping results from all nodes are concatanated and written in out.txt file in outdir. Additional output files specified as outfile:filename in programpars are also concatanated and written in filename in outdir.
    * The timing information is written in map_time.txt file in outdir.
    * All files in workdir of each node are deleted unless -dc option is used. 

* Examples for running of pmap_dist and pmap in a PBS environment are given below. Commands for executing equivalent runs sequentially are also given in program specific examples. In all of the following examples assume that 4 nodes with 8 processors each are used for pMap and the following variables are set:
workdir=/temp
outdir=/home/myuser
readsfile=/home/myuser/reads.fq
readsfile2=/home/myuser/reads2.fq #if needed
indexdir=/home/myuser/myindexdir
indexprefix=myindexprefix

* Example: Map single end reads using Bowtie. Index files are at a shared location.
#PBS -l nodes=4:ppn=8
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile
mpiexec -n 32 pmap -i $indexdir $indexprefix $workdir $outdir bowtie

* Example: Map paired end reads using BWA. Index files are replicated.
#PBS -l nodes=4:ppn=8
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -r $readsfile2 -i $indexdir $indexprefix
mpiexec -n 32 pmap -pe $workdir $outdir bwa

* Example: Map paired end reads using SOAP. Index files are replicated. Use -p 8 option of soap to use 8 threads per node.
#PBS -l nodes=4:ppn=8
uniq $PBS_NODEFILE > $ids # Get ids of the nodes and run a single instance with 8 threads on each node
mpiexec -n 4 -machinefile $ids pmap_dist $workdir $outdir $readsfile -r $readsfile2 -i $indexdir $indexprefix
mpiexec -n 4 -machinefile $ids pmap -pe $workdir $outdir soap -p 8 -2 outfile:unpaired.txt

Please also note the following for pmap_dist and pmap:

    * All directory and file names should be absolute paths unless stated otherwise.
    * All output files should be text files. Binary files cannot be concatanated.
    * Replicating the index files has copying overhead. However, it is still likely to be slightly faster than using shared index files, since reading files from a shared location may overload the file system and degrade the performance.
    * Files and folders requested to be on the head node can also be at a shared location.
    * workdir can also be at a shared location. But it may overload the file system and degrade the performance.
    * For multi-core machines, the selected mapping program can either be run in multi-threaded mode (if supported) as in the third example above, or each core can be treated as a separate node as in the first two examples above. The latter is likely to be slightly faster, however, it requires more memory since multiple instances of the mapping program are executed on the same node. 
---------------------------------------------------------------------------------------------------------------------------

*Using pMap with BWA

When pmap is called for BWA program, bwa aln is executed with appropriate parameters on each node to map the group of reads assigned to the node by pmap_dist. Subsequently, the generated output files in BWA-native ".sai" format are converted to SAM format by executing bwa samse for single end reads and bwa sampe for paired end reads. The optional parameters of bwa sampe or bwa samse can be provided to pmap by using the -conversion option of pmap.
* Example: Map paired end reads. Index files are replicated. Use -l 28 option with bwa aln and -a 400 option with bwa sampe.
#PBS -l nodes=4:ppn=8
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -r $readsfile2 -i $indexdir $indexprefix
mpiexec -n 32 pmap -pe $workdir $outdir bwa -l 28 -conversion -a 400
#Sequential equivalent is:
bwa aln -l 28 $indexdir/$indexprefix $readsfile > $workdir/out.sai
bwa aln -l 28 $indexdir/$indexprefix $readsfile2 > $workdir/out2.sai
bwa sampe -a 400 $indexdir/$indexprefix $workdir/out.sai $workdir/out2.sai $readsfile $readsfile2 > $outdir/out.txt

For more information on BWA: http://bio-bwa.sourceforge.net/
(pMap was tested using version 0.5.0 of BWA)
----------------------------------------------------------------------------------------------------------------------------

*Using pMap with SOAP

When pmap is called for SOAP program, soap is executed with -D option and appropriate parameters on each node to map the group of reads assigned to the node by pmap_dist.

-a,-b,-D,-o options of SOAP are automatically used within pmap and therefore should not be specified in programpars.
* Example: Map paired end reads. Index files are replicated. Write unpaired reads into unpaired.txt using -2 option of soap.
#PBS -l nodes=4:ppn=8
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -r $readsfile2 -i $indexdir $indexprefix
mpiexec -n 32 pmap -pe $workdir $outdir soap -2 outfile:unpaired.txt
#Sequential equivalent is:
soap -D $indexdir/$indexprefix -a $readsfile -b $readsfile2 -o $outdir/out.txt -2 $outdir/unpaired.txt

For more information on SOAP: http://soap.genomics.org.cn/
(pMap was tested using version 2.20 of SOAP)
Note: Not using "-2 <str>" option when mapping paired end reads generates an error in SOAP.
----------------------------------------------------------------------------------------------------------------------------

*Using pMap with Bowtie

When pmap is called for Bowtie program, bowtie is executed with appropriate parameters on each node to map the group of reads assigned to the node by pmap_dist. The BOWTIE_INDEXES environment variable must be set to the folder name that contains the index files (e.g. the following line should be present in .bashrc file: BOWTIE_INDEXES=/home/myuser/myindexdir; export BOWTIE_INDEXES. Also see Bowtie manual). Currently, using Bowtie with replicated index files (i.e. -i option of pmap_dist) is not supported by pMap. Therefore it should be used with shared index files (i.e. using -i option of pmap).

--12,-c,-s,-u,-b,--refout,--mm,-f (with ".mfa" files) options of Bowtie are not supported by pmap.
-1,-2 options of Bowtie are automatically used within pmap and therefore should not be specified in programpars.
* Example: Map paired end reads first by using default options, then by using the --best option of bowtie. Using the -dc option of pmap in the first run allows reusing the files distributed/replicated by pmap_dist in the second run. Index files are at a shared location.
#PBS -l nodes=4:ppn=8
outdir1=/home/myuser/myoutdir1 #Write output of first run here
outdir2=/home/myuser/myoutdir2 #Write output of second run here
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -r $readsfile2
mpiexec -n 32 pmap -pe -dc -i $indexdir $indexprefix $workdir $outdir1 bowtie
mpiexec -n 32 pmap -pe -i $indexdir $indexprefix $workdir $outdir2 bowtie --best
#Sequential equivalent is:
outdir1=/home/myuser/myoutdir1 #Write output of first run here
outdir2=/home/myuser/myoutdir2 #Write output of second run here
bowtie $indexprefix -1 $readsfile -2 $readsfile2 $outdir1/out.txt
bowtie --best $indexprefix -1 $readsfile -2 $readsfile2 $outdir2/out.txt

For more information on Bowtie: http://bowtie-bio.sourceforge.net/
(pMap was tested using version 0.10.1 of Bowtie)
---------------------------------------------------------------------------------------------------------------------------

*Using pMap with GSNAP

When pmap is called for GSNAP program, gsnap is executed with appropriate parameters on each node to map the group of reads assigned to the node by pmap_dist. When using pMap with GSNAP, the indexdir should be set to the database directory that corresponds to the genome being considered. Also, indexprefix should be set to the name of the database. GSNAP uses a single input file when mapping paired end reads, therefore readsfile2 paramater is not needed.

-D,-d options of GSNAP are automatically used within pmap and therefore should not be specified in programpars.
* Example: Map paired end reads. The readsfile is in a special format that contains 3 lines per read. Use known SNP information in /home/myuser/snp.iit file using -V option of gsnap. Replicate the SNP information file as well as the index files (database files).
#PBS -l nodes=4:ppn=8
indexdir=/home/myuser/mygenomedb
indexprefix=mygenomedb
inpfile=/home/myuser/snp.iit
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -l 3 -i $indexdir $indexprefix -f $inpfile
mpiexec -n 32 pmap -pe $workdir $outdir gsnap -V infile:snp.iit
#Sequential equivalent is:
indexdir=/home/myuser/mygenomedb
indexprefix=mygenomedb
inpfile=/home/myuser/snp.iit
gsnap -V $inpfile -D $indexdir -d $indexprefix $readsfile > $outdir/out.txt

For more information on GSNAP: http://research-pub.gene.com/gmap/
(pMap was tested using version 2010-07-27 of GMAP/GSNAP)
---------------------------------------------------------------------------------------------------------------------------

*Using pMap with MAQ

When pmap is called for MAQ program, first maq fastq2bfa and maq fastq2bfq are called to convert the input genome and the group of reads assigned to the node to an maq-internal format (optional [-n nreads] parameter of maq fastq2bfa is ignored). Then, maq map is executed with appropriate parameters on each node to map the reads. Subsequently, the generated output files in MAQ-native ".map" format are converted to a human-readable format by executing maq mapview. The optional parameters of maq mapview can be provided to pmap by using the -conversion option of pmap. Since there's no distinct indexing step in MAQ, the indexdir and indexprefix paramaters of pmap_dist and pmap should be set to the directory of the genome file and the genome file name respectively.
* Example: Map paired end reads. Index file (genome file) is replicated.
#PBS -l nodes=4:ppn=8
indexdir=/home/myuser
indexprefix=mygenome.fa
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -i $indexdir $indexprefix
mpiexec -n 32 pmap -pe $workdir $outdir maq
#Sequential equivalent is:
indexdir=/home/myuser
indexprefix=mygenome.fa
maq fasta2bfa $indexdir/$indexprefix $workdir/index.bfa
maq fastq2bfq $readsfile $workdir/reads.bfq
maq fastq2bfq $readsfile2 $workdir/reads2.bfq
maq map $workdir/out.map $workdir/index.bfa $workdir/reads.bfq $workdir/reads2.bfq
maq mapview $workdir/out.map > $outdir/out.txt

For more information on MAQ: http://maq.sourceforge.net/
(pMap was tested using version 0.7.1 of MAQ)
----------------------------------------------------------------------------------------------------------------------------

*Using pMap with RMAP

When pmap is called for RMAP program, rmap is executed with -Q option and appropriate parameters on each node to map the group of reads assigned to the node by pmap_dist. Since there's no distinct indexing step in RMAP, the indexdir and indexprefix paramaters of pmap_dist and pmap should be set to the directory of the genome file and the genome file name respectively. The genome should be stored in a file in FASTA format, which may contain one or multiple chromosomes.

-s,-F,-p options of RMAP are not supported by pmap.
-c,-Q,-o options of RMAP are automatically used within pmap and therefore should not be specified in programpars.
* Example: Map paired end reads. Index file (genome file) is replicated.
#PBS -l nodes=4:ppn=8
indexdir=/home/myuser
indexprefix=mygenome.fa
mpiexec -n 32 pmap_dist $workdir $outdir $readsfile -i $indexdir $indexprefix
mpiexec -n 32 pmap -pe $workdir $outdir rmap
#Sequential equivalent is:
indexdir=/home/myuser
indexprefix=mygenome.fa
rmappe -c $indexdir/$indexprefix -Q $readsfile -o $outdir/out.txt

For more information on RMAP: http://rulai.cshl.edu/rmap/
(pMap was tested using version 2.05 of RMAP)
