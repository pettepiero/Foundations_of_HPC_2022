// simple hello world program 
#include <stdio.h>
#include <mpi.h>

int main (int argc, char * argv[])
{
  int rank, size;
  int my_val = 0;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD,&rank );
  MPI_Comm_size( MPI_COMM_WORLD,&size );
  if (rank==0)
    my_val = 40;
  printf( "I am %d of %d, and for me my_val = %d\n", rank, size, my_val );
  int a = 0;

  for(int i =0; i < 100000; i++)
    a++;

  MPI_Bcast(&my_val, 4, MPI_INT, 0, MPI_COMM_WORLD);

  printf( "I am %d of %d, and for me my_val = %d\n", rank, size, my_val );


  MPI_Finalize();
}
