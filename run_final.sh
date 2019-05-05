#!/bin/bash

qsub finalprojectrun.pbs -l nodes=1:ppn=1
qsub finalprojectrun.pbs -l nodes=1:ppn=2
qsub finalprojectrun.pbs -l nodes=1:ppn=4
qsub finalprojectrun.pbs -l nodes=1:ppn=8
qsub finalprojectrun.pbs -l nodes=1:ppn=16