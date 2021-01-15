/*
 * Copyright (C) Your copyright.
 *
 * Author: Nicola Dilillo
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the PG_ORGANIZATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY	THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS-IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/time.h>

/*
* Uncomment this line to see the time to elaborate each single value
*/
// #define DEBUG

#define WAIT_TIME 20000 // 20 ms


#define q	11		    /* for 2^11 points */
#define N	(1<<q)		/* N-point FFT, iFFT */

typedef float real;
typedef struct{real Re; real Im;} complex;
typedef struct{int fd; real* v;} pthread_struct;

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif

void *myThread(void *arg) {

  pthread_struct* app = (pthread_struct *) arg;
  // temporaly variable
  int tmp;
  
  // read value from device
  read(app->fd, &tmp, 4);
  *(app->v) = (float )tmp;

  pthread_exit(NULL);
}

void fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

int main(void)
{
  complex v[N], scratch[N];
  float abs[N];
  int k;
  int m;
  int i;
  int minIdx, maxIdx;

  #ifdef DEBUG
    struct timespec start, end;
  #endif

  char *dev_name = "/dev/PPG_dev";
  int fd = -1;
  if ((fd = open(dev_name, O_RDWR)) < 0)
  {
	  fprintf(stderr, "PPG APP: unable to open %s: %s\n", dev_name, strerror(errno));
	  return( 1 );
  }

  int ret;
  pthread_t mythread;
  pthread_struct app;
  app.fd = fd;

  #ifdef DEBUG
    long delta_us;
  #endif

  while(1) {
    #ifdef DEBUG
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    #endif

	  //Initialize the complex array for FFT computation
	  for(k=0; k<N; k++) {
		  app.v = &v[k].Re;
		  v[k].Im = 0;
		  ret = pthread_create(&mythread, NULL, myThread, &app);
		  if(ret != 0) {
			  printf("Can't create pthread (%s)\n", strerror(errno));
			  exit(-1);
		  }

      // sleep for 20 ms before read next value (f = 50 Hz)
		  usleep(WAIT_TIME);
		  pthread_join(mythread, NULL);
	  }

	  // FFT computation
	  fft( v, N, scratch );

	  // PSD computation
  	for(k=0; k<N; k++) {
		  abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im));
	  }

	  minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
	  maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm

  	// Find the peak in the PSD from 30 bpm to 180 bpm
	  m = minIdx;
	  for(k=minIdx; k<(maxIdx); k++) {
      if( abs[k] > abs[m] )
      m = k;
	  }

	  // Print the heart beat in bpm
	  printf( "\n\n\n%d bpm", (m)*60*50/2048 );

    #ifdef DEBUG
      //calculate time to compute operation
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);
      delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
      printf(" --> Time: %lu ms\n",delta_us/(1000));
    #endif
  }

  close(fd);

  exit(EXIT_SUCCESS);
}