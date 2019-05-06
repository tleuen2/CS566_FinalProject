#!/bin/bash

#qsub RUN.pbs -F "1" -l nodes=1:ppn=1
#qsub RUN.pbs -F "1" -l nodes=1:ppn=2
#qsub RUN.pbs -F "1" -l nodes=1:ppn=4
#qsub RUN.pbs -F "1" -l nodes=1:ppn=8
#qsub RUN.pbs -F "1" -l nodes=1:ppn=16



qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=2
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=4
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=8
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=16

qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=1
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=2
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=4
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=8
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=16

qsub NOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=1
qsub NOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=2
qsub NOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=4
qsub NOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=8
qsub NOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=16

qsub NOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=1
qsub NOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=2
qsub NOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=4
qsub NOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=8
qsub NOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=16

qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=1
qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=2
qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=4
qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=8
qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=16

qsub NOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=1
qsub NOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=2
qsub NOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=4
qsub NOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=8
qsub NOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=16

qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=1
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=2
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=4
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=8
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=16

qsub NOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=1
qsub NOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=2
qsub NOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=4
qsub NOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=8
qsub NOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=16

qsub NOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=1
qsub NOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=2
qsub NOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=4
qsub NOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=8
qsub NOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=16