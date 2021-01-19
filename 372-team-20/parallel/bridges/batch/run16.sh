#!/bin/bash
#SBATCH -p RM-shared
#SBATCH -t 00:05:00
#SBATCH -N 1
#SBATCH --ntasks-per-node=16
set -x
mpirun -np $SLURM_NTASKS ./b.exec
