#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

int main(int argc, char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  long inputArgument = ins__args.arg;

  struct timeval ins__tstart, ins__tstop;

  gettimeofday(&ins__tstart, NULL);

  long max = inputArgument / 2;
  long temp = 0;

  int num_threads = omp_get_num_procs();
  omp_set_num_threads(num_threads);

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

  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

  return 0;
}
