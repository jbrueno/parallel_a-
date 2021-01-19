#!/bin/bash
#SBATCH -p RM
#SBATCH -t 00:05:00
#SBATCH -N 1
#SBATCH --ntasks-per-node=28
set -x
mpirun -np $SLURM_NTASKS ./b.exec
