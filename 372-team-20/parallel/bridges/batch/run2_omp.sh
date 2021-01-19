#!/bin/bash
#SBATCH -p RM
#SBATCH -t 00:05:00
#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=28
set -x
OMP_NUM_THREADS=28 mpirun -np $SLURM_NTASKS ./a.exec
