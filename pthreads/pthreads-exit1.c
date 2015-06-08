#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS 3

void *job( void *arg )
{
  int *num = arg; /* �bergebenen Parameter verwerten */

  printf( "Thread Nr. %d l�uft.\n", *num );
  sleep( 10 );
  printf( "Thread Nr. %d ist fertig.\n", *num );

  pthread_exit( num ); /* Threadnummer zur�ck */
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  int i, status;

  printf( "Programm l�uft.\n" );

  for( i = 1; i <= NUM_THREADS; i++ )
  {
    /* ACHTUNG: KRITISCHER FEHLER!!! */
    status = pthread_create( &tid, NULL, job, &i );
    if( status != 0 )
    {
      printf( "Fehler in pthread_create(): %s\n",
        strerror( status ) );
      exit( 1 );
    }
  }

  printf( "Threads gestartet, Programm l�uft weiter.\n" );
  sleep( 5 );
  printf( "Haupt-Thread beendet sich.\n" );
  exit( 0 );
}
