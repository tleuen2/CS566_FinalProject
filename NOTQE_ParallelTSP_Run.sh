#!/bin/bash

#qsub RUN.pbs -l nodes=1:ppn=1
#qsub RUN.pbs -l nodes=1:ppn=2
#qsub RUN.pbs -l nodes=1:ppn=4
#qsub RUN.pbs -l nodes=1:ppn=8
#qsub RUN.pbs -l nodes=1:ppn=16

qsub NOTQE28INPUT.pbs -F "1 1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "1 1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "1 1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "1 1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "1 1" -l nodes=1:ppn=16


qsub NOTQE28INPUT.pbs -F "2 1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "2 1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "2 1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "2 1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "2 1" -l nodes=1:ppn=16


qsub NOTQE28INPUT.pbs -F "3 1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "3 1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "3 1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "3 1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "3 1" -l nodes=1:ppn=16


qsub NOTQE28INPUT.pbs -F "4 1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "4 1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "4 1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "4 1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "4 1" -l nodes=1:ppn=16


qsub NOTQE28INPUT.pbs -F "5 1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "5 1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "5 1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "5 1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "5 1" -l nodes=1:ppn=16