#!/bin/bash

qsub final_project_run.pbs -l nodes=1:ppn:1
qsub final_project_run.pbs -l nodes=1:ppn:2
qsub final_project_run.pbs -l nodes=1:ppn:4
qsub final_project_run.pbs -l nodes=1:ppn:8
qsub final_project_run.pbs -l nodes=1:ppn:16
