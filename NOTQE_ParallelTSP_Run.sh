#!/bin/bash

#qsub RUN.pbs -l nodes=1:ppn=1
#qsub RUN.pbs -l nodes=1:ppn=2
#qsub RUN.pbs -l nodes=1:ppn=4
#qsub RUN.pbs -l nodes=1:ppn=8
#qsub RUN.pbs -l nodes=1:ppn=16

qsub INPUT6.pbs -F "1 1" -l nodes=1:ppn=1
qsub INPUT6.pbs -F "1 1" -l nodes=1:ppn=2
qsub INPUT6.pbs -F "1 1" -l nodes=1:ppn=4
qsub INPUT6.pbs -F "1 1" -l nodes=1:ppn=8
qsub INPUT6.pbs -F "1 1" -l nodes=1:ppn=16