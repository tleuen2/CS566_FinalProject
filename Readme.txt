Latest modified on 05/12/2015

This is Readme file for code of TSP with load balancing, done by Ouwen Shi and Zhengyu Hao, for ECE/CS 566 final project.

1.The source code file "tsp.cpp" is the only source code file to solve TSP with load balancing.

2.The executable file "ptsp" can only be executed under unix system environment. 

3.To compile the source code file "tsp.cpp", under the directory of the makeup file "Makefile", type in command "make".
Then the code is compiled and the executable is generated.

4.To run the executable, the script file "script" is needed. It contains all the command for executing the executable
file "ptsp" under unix system. In this file, the program with 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 processors is
executed sequentially. There is one command line argument: the name of the input adjacent matrix file. Under the directory
of the executable, type in command "sbatch script" to execute.

5.The input file contains an "n*n" size adjacent matrix. Because of the limited space, a row of the adjacent matrix may
occupies more than one line. The input file "br17.atsp" contains an asymmetry adjacent matrix which has a size of 17*17.

6.Check output file "output" provided along for an example.
Ver  : {No. of vertices}
n(P) : {No. of processors}
Sol  : {[vertice1 vertice2 ... verticeGRAPHSIZE] TOTALPATH} Best Cost Variable: {BESTPATH}
Time : {Maximum parallel time in milliseconds}
DistT: {Time taken for startup phase(each processor acquires initial load via recursive doubling initiated at P0)}