#!/bin/bash

qsub QE_Config_Run.pbs -l nodes=1:ppn=1
qsub QE_Config_Run.pbs -l nodes=1:ppn=2
qsub QE_Config_Run.pbs -l nodes=1:ppn=4
qsub QE_Config_Run.pbs -l nodes=1:ppn=8
qsub QE_Config_Run.pbs -l nodes=1:ppn=16