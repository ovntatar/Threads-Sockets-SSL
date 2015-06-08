#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS 3

void *job( void *arg )
{
  int *num = arg; /* übergebenen Parameter verwerten */

  printf( "Thread Nr. %d läuft.\n", *num );
  sleep( 10 );
  printf( "Thread Nr. %d ist fertig.\n", *num );

  pthread_exit( num ); /* Threadnummer zurück */
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  int i, status;

  printf( "Programm läuft.\n" );

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

  printf( "Threads gestartet, Programm läuft weiter.\n" );
  sleep( 5 );
  printf( "Haupt-Thread beendet sich.\n" );
  exit( 0 );
}
