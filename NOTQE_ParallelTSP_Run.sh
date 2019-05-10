#!/bin/bash

##qsubRUN.pbs -F "1" -l nodes=1:ppn=1
##qsubRUN.pbs -F "1" -l nodes=1:ppn=2
##qsubRUN.pbs -F "1" -l nodes=1:ppn=4
##qsubRUN.pbs -F "1" -l nodes=1:ppn=8
##qsubRUN.pbs -F "1" -l nodes=1:ppn=16



#qsubNOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=1
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=2
sleep 5
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=4
sleep 5
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=8
sleep 5
qsub NOTQE28INPUT.pbs -F "1" -l nodes=1:ppn=16
sleep 5

#sleep 20

qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=1
sleep 5
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=2
sleep 5
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=4
sleep 5
qsub NOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=8
sleep 5
#qsubNOTQE28INPUT.pbs -F "2" -l nodes=2:ppn=16

#sleep 20

#qsubNOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=1
#qsubNOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=2
#qsubNOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=4
#qsubNOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=8
#qsubNOTQE28INPUT.pbs -F "3" -l nodes=3:ppn=16

#sleep 20

#qsubNOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=1
#qsubNOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=2
#qsubNOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=4
#qsubNOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=8
#qsubNOTQE28INPUT.pbs -F "7" -l nodes=7:ppn=16

#sleep 20

qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=1
sleep 5
qsub NOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=2
sleep 5
#qsubNOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=4
#qsubNOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=8
#qsubNOTQE28INPUT.pbs -F "8" -l nodes=8:ppn=16

#sleep 20

#qsubNOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=1
#qsubNOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=2
#qsubNOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=4
#qsubNOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=8
#qsubNOTQE28INPUT.pbs -F "9" -l nodes=9:ppn=16

#sleep 20

qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=1
sleep 5
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=2
sleep 5
qsub NOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=4
#qsubNOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=8
#qsubNOTQE28INPUT.pbs -F "4" -l nodes=4:ppn=16

#sleep 20

#qsubNOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=1
#qsubNOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=2
#qsubNOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=4
#qsubNOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=8
#qsubNOTQE28INPUT.pbs -F "5" -l nodes=5:ppn=16

#sleep 20

#qsubNOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=1
#qsubNOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=2
#qsubNOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=4
#qsubNOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=8
#qsubNOTQE28INPUT.pbs -F "6" -l nodes=6:ppn=16