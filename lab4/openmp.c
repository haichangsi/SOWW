#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>


int main(int argc,char **argv) {


  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;
  gettimeofday(&ins__tstart, NULL);
  
  // run your computations here (including OpenMP stuff)

  long max = inputArgument / 2;
  long temp = 0;

#pragma omp parallel for reduction(+ : temp)
  for (long i = 2; i <= max; i++) {
    if (inputArgument % i == 0) {
      temp = temp + 1;
      break;
    }
  }

  if (temp == 0 && inputArgument != 1) {
    printf("input=%ld is prime", inputArgument);
  } else {
    printf("input=%ld is not prime", inputArgument);
  }
  
  // synchronize/finalize your computations
  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

}
