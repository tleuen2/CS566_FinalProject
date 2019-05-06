#!/bin/bash

#qsub RUN.pbs -l nodes=1:ppn=1
#qsub RUN.pbs -l nodes=1:ppn=2
#qsub RUN.pbs -l nodes=1:ppn=4
#qsub RUN.pbs -l nodes=1:ppn=8
#qsub RUN.pbs -l nodes=1:ppn=16



qsub NOTQE28INPUT.pbs -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -l nodes=1:ppn=16