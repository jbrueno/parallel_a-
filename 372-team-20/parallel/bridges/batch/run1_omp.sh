#!/bin/bash
#SBATCH -p RM
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=28
set -x
OMP_NUM_THREADS=28 mpirun -np $SLURM_NTASKS ./a.exec
