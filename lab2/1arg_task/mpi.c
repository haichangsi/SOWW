#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#define PACKAGE_SIZE 4

#define DATA 0
#define RESULT 1
#define FINISH 2

// used by a slave to count how many specific numbers are in the package
void calc_sum(int *numbers_package, int *numbers_count, int inputArgument) {
  for (int i=0; i<PACKAGE_SIZE; i++) {
    if (numbers_package[i] >= inputArgument)
      printf("Max number is %d", inputArgument-1);
    int curr_number = numbers_package[i];
    numbers_count[curr_number] = numbers_count[curr_number] + 1;
  }
}
// used by a master to sum all 
void sum_cor_el(int *arr1, int *sum_arr, int inputArgument) {
  // int sum_arr[inputArgument];
  for (int i=0; i<inputArgument; i++) {
    sum_arr[i] = arr1[i] + sum_arr[i];
  }
}

void print_result(int *result, int inputArgument, int* counter) {
  for (int i =0; i< inputArgument; i++) {
    printf("Number of %d values= %d \n", i, result[i]);
    *counter += result[i];
  }
}

void print_rand_values(int *values, int inputArgument) {
  printf("rand values: \n");
  for (int i =0; i< inputArgument; i++) {
    printf("%d \n",values[i]);
  }
}

void print_arr(int *arr, int n, int inputArgument) {
    printf("Printing package: \n");
    for (int i =0; i< n; i++) {
    printf("%d ", arr[i]);
  }
}
int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  int counter = 0;

  int *arr, *package, *temp_result, *result;
  MPI_Status status;

  arr = (int*)calloc(inputArgument, sizeof(int));
  package = (int*)calloc(PACKAGE_SIZE, sizeof(int));

  result = (int*)calloc(inputArgument, sizeof(int));
  int i;
  
  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank)
      gettimeofday(&ins__tstart, NULL);


  // run your computations here (including MPI communication)
  if (myrank == 0) {
    time_t t;
    srand((unsigned) time(&t));

    for (i = 0;i<inputArgument;i++) {
      arr[i] = rand() % 1000;
    }

    print_rand_values(arr, inputArgument);
    // distribute packages to slaves
    for (i = 1; i < nproc; i++) {
      // prevent a segfault when the number of processes is greater than the number of packages
      if (PACKAGE_SIZE*(i) -1 > inputArgument)
        break;
      printf("Master sending range %d,%d to process %d \n",*(arr+PACKAGE_SIZE*(i-1)),*(arr+PACKAGE_SIZE*(i)-1),i); fflush(stdout);
      // send it to process i
      MPI_Send (arr+PACKAGE_SIZE*(i-1), PACKAGE_SIZE, MPI_INT, i, DATA, MPI_COMM_WORLD);
    }

    do
    {
      temp_result = (int*)calloc(inputArgument, sizeof(int));
      // distribute remaining subranges to the processes which have completed their parts
      MPI_Recv (temp_result, inputArgument, MPI_INT, MPI_ANY_SOURCE, RESULT,
                MPI_COMM_WORLD, &status);
      sum_cor_el(temp_result, result, inputArgument);
      // printf("\nMaster received result from process %d \n",status.MPI_SOURCE);
      // printf("\nCurrent temp_result: \n");
      // print_result(temp_result, inputArgument);
      free(temp_result);

      // printf("\nCurrent result: \n");
      // print_result(result, inputArgument);

      MPI_Send (arr+PACKAGE_SIZE*(i-1), PACKAGE_SIZE, MPI_INT, status.MPI_SOURCE, DATA,
            MPI_COMM_WORLD);

            i = i + 1;

    } while ((i-1)*PACKAGE_SIZE < inputArgument);

    // receive results
    for (i = 0; i < (nproc - 1); i++)
	  {
      MPI_Recv (temp_result, inputArgument, MPI_INT, MPI_ANY_SOURCE, RESULT,
      MPI_COMM_WORLD, &status);
      sum_cor_el(temp_result, result, inputArgument);

      // printf("\nMaster received result from process %d",status.MPI_SOURCE);
      // printf("\nCurrent temp_result:\n");
      // print_result(temp_result, inputArgument);

      // printf("\nCurrent result: \n");
      // print_result(result, inputArgument);
	  }

    // shut down the slaves
    for (i = 1; i < nproc; i++)
    {
      MPI_Send (NULL, 0, MPI_INT, i, FINISH, MPI_COMM_WORLD);
    }

    printf ("\nHi, I am process 0 \n");
    printf ("Run values: \n");
    print_rand_values(arr, inputArgument);
    printf("\n");
    printf("Result: \n");
    print_result(result, 1000, &counter);
    printf("\ncounter: %d", counter);
        // now display the result
        // printf ("\nHi, I am process 0, the result is %f\n", result);

  }
  // myrank is not 0
  else {
    do {
      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG==DATA) {
        temp_result = (int*)calloc(inputArgument, sizeof(int));
        MPI_Recv(package, PACKAGE_SIZE, MPI_INT, 0, DATA, MPI_COMM_WORLD, &status);
        // slave computations
        // printf("\n hey from slave %d \n", myrank);
        // print_arr(package, PACKAGE_SIZE, inputArgument);
        calc_sum(package, temp_result, inputArgument);
        // printf("\nSlave %d result: \n", myrank);
        // print_result(temp_result, inputArgument);
        MPI_Send(temp_result, inputArgument, MPI_INT, 0, RESULT, MPI_COMM_WORLD);
        free(temp_result);
      }
    
    } while (status.MPI_TAG != FINISH);

  }
  // synchronize/finalize your computations
  free(arr);
  free(package);
  // free(temp_result);
  free(result);

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
  
  MPI_Finalize();
  return 0;
}
