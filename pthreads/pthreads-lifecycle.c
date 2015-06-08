#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *job( void *arg )
{
  printf( "Thread l�uft.\n" );
  sleep( 15 );
  printf( "Thread ist fertig.\n" );

  return( NULL ); /* entspricht pthread_exit( NULL ); */
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  void *result;
  int status;

  printf( "Programm l�uft.\n" );

  status = pthread_create( &tid, NULL, job, NULL );
  if( status != 0 )
  {
    printf( "Fehler in pthread_create(): %s\n",
      strerror( status ) );
    exit( 1 );
  }

  printf( "Thread gestartet, Programm l�uft weiter.\n" );
  sleep( 5 );
  printf( "Programm wartet auf Thread.\n" );

  status = pthread_join( tid, &result );
  if( status != 0 )
  {
    printf( "Fehler in pthread_join(): %s\n",
      strerror( status ) );
    exit( 1 );
  }

  printf( "Programm beendet sich.\n" );
  exit( 0 );
}
