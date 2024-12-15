#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <time.h>

//#define DEBUG
//#define BLINKER
#define PROFILING

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1
#define N_STEPS 100
#define MAXVAL 1 /*1 -> white, 0 -> black*/
#define	CACHE_LINE_SIZE 64

typedef struct {
	int   	action;
	int   	k; 	//number of columns
	int   	e;
	int   	n;
	int   	s;
    int 	size_of_cluster;
	int 	nrows; //number of rows = k+2
	int		my_process_rows;
	int 	my_process_start_idx;
} Env;

#if defined(_OPENMP)
#define CPU_TIME ({struct  timespec ts; clock_gettime( CLOCK_REALTIME, &ts ),\
					  (double)ts.tv_sec +		\
					  (double)ts.tv_nsec * 1e-9;})

#define CPU_TIME_th ({struct  timespec myts; clock_gettime( CLOCK_THREAD_CPUTIME_ID, &myts ),\
					     (double)myts.tv_sec +	\
					     (double)myts.tv_nsec * 1e-9;})
#else

#define CPU_TIME ({struct  timespec ts; clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ),\
					  (double)ts.tv_sec +		\
					  (double)ts.tv_nsec * 1e-9;})

#endif

#endif  // CONSTANTS_H
