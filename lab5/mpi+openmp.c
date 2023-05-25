#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>

#define THREADNUM 4

int main(int argc,char **argv) {

  omp_set_num_threads(THREADNUM);
  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc,thread_support;
  
  MPI_Init_thread(&argc,&argv, MPI_THREAD_MULTIPLE, &thread_support);

  if (thread_support != MPI_THREAD_MULTIPLE)
  {
    printf ("\nThe implementation does not support MPI_THREAD_MULTIPLE, it supports level %d\n",
            thread_support);
    MPI_Finalize();
    exit (-1);
  }

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank)
      gettimeofday(&ins__tstart, NULL);


  // run your computations here (including MPI communication)
  long max = inputArgument / 2;
  long step = max / nproc;
  long start = 2 + myrank * step;
  long mine = start + (myrank+1) * step;
  long temp = 0;
  long temp_final = 0;

  #pragma omp parallel for reduction(+:temp)
  for (long i=start; i<mine; i++) {
	if (inputArgument % i ==0) {
		temp = temp + 1;
	}
  }

  MPI_Reduce(&temp, &temp_final, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if (myrank==0) {
	  if(temp_final==0 && inputArgument != 1) {
		  printf("input=%ld is prime", inputArgument);
	 }
	  else {
		  printf("input=%ld is not prime", inputArgument);
	}
  }	  



  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
  
  MPI_Finalize();

}
