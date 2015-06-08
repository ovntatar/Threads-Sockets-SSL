#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBERS 10

typedef struct intbuffer {
  int val[MAX_NUMBERS];
  int in, out;
  pthread_mutex_t mutex;
} intbuffer_t;

intbuffer_t data;

void error_exit( char *message, int status )
{
  printf( "%s: %s\n", message, strerror( status ) );
  exit( 1 );
}

void *avg( void *arg )
{
  int sum = 0, num = 0, status;

  for(;;) /* Endlosschleife */
  {
    /* exklusiven Zugriff auf data sicherstellen */
    status = pthread_mutex_lock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_lock()", status );

    if( data.in != data.out ) /* liegt neuer Wert vor? */
    {
      /* Wert aufsummieren und Puffer weiterschalten */
      sum += data.val[data.out];
      data.out = ( data.out + 1 ) % MAX_NUMBERS;

      num++; /* internen Zähler inkrementieren */

      printf( "Durchschnitt der %d Werte: %lf\n", num,
        (double)sum / num );
    }
    
    /* exklusiven Zugriff auf data freigeben */
    status = pthread_mutex_unlock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_unlock()", status );
  }

  return( NULL );
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  char input[32];
  int status;

  /* Datenpuffer inklusive Mutex initialisieren */
  data.in = 0;
  data.out = 0;
  status = pthread_mutex_init( &data.mutex, NULL );
  if( status != 0 )
    error_exit( "pthread_mutex_init()", status );

  status = pthread_create( &tid, NULL, avg, NULL );
  if( status != 0 )
    error_exit( "pthread_create()", status );

  for(;;) /* Endlosschleife */
  {
    /* Einen neuen Wert einlesen ... */
    printf( "input> " );
    fgets( input, sizeof( input ), stdin );

    /* exklusiven Zugriff auf data sicherstellen */
    status = pthread_mutex_lock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_lock()", status );

    /* Wert im Puffer speichern und Puffer weiterschalten */
    if( ( ( data.in + 1 ) % MAX_NUMBERS ) != data.out )
    {
      data.val[data.in] = atoi( input );
      data.in = ( data.in + 1 ) % MAX_NUMBERS;
    }
    else
      printf( "Puffer voll, Eingabe wiederholen.\n" );

    /* exklusiven Zugriff auf data freigeben */
    status = pthread_mutex_unlock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_unlock()", status );
  }

  exit( 0 );
}
