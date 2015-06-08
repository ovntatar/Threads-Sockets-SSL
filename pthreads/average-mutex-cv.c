#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBERS 10
#define NUM_CONSUMERS 2

typedef struct intbuffer {
  int val[MAX_NUMBERS];
  int in, out;
  pthread_mutex_t mutex;
  pthread_cond_t add, rem;
} intbuffer_t;

intbuffer_t data;

void error_exit( char *message, int status )
{
  printf( "%s: %s\n", message, strerror( status ) );
  exit( EXIT_FAILURE );
}

void *avg( void *arg )
{
  int id = (int)arg;
  int sum = 0, num = 0, status;

  for(;;) /* Endlosschleife */
  {
    /* exklusiven Zugriff auf data sicherstellen */
    status = pthread_mutex_lock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_lock()", status );

    while( data.in == data.out ) /* solange Puffer leer */
    {
      /* auf neuen Wert warten, Mutex temporär freigeben */
      status = pthread_cond_wait( &data.add, &data.mutex );
      if( status != 0 )
        error_exit( "pthread_cond_wait()", status );
    }

    /* Wert aufsummieren und Puffer weiterschalten */
    sum += data.val[data.out];
    data.out = ( data.out + 1 ) % MAX_NUMBERS;

    /* erfolgte Aktualisierung des Puffers mitteilen */
    status = pthread_cond_signal( &data.rem );
    if( status != 0 )
      error_exit( "pthread_cond_signal()", status );

    /* exklusiven Zugriff auf data freigeben */
    status = pthread_mutex_unlock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_unlock()", status );

    num++; /* Zähler inkrementieren */

    printf( "Thread %d - Durchschnitt der %d Werte: %lf\n",
      id, num, (double)sum / num );
  }

  return( NULL );
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  char input[32];
  int i, status;

  /* Puffer, Mutex und Bedingungsvariable initialisieren */
  data.in = 0;
  data.out = 0;
  status = pthread_mutex_init( &data.mutex, NULL );
  if( status != 0 )
    error_exit( "pthread_mutex_init()", status );
  status = pthread_cond_init( &data.add, NULL );
  if( status != 0 )
    error_exit( "pthread_cond_init()", status );
  status = pthread_cond_init( &data.rem, NULL );
  if( status != 0 )
    error_exit( "pthread_cond_init()", status );

  /* Die gewünschte Anzahl Verbraucher-Threads starten */
  for( i = 1; i <= NUM_CONSUMERS; i++ )
  {
    status = pthread_create( &tid, NULL, avg, (void *)i );
    if( status != 0 )
      error_exit( "pthread_create()", status );
  }

  for(;;) /* Endlosschleife */
  {
    /* Einen neuen Wert einlesen ... */
    printf( "input> " );
    fgets( input, sizeof( input ), stdin );

    /* exklusiven Zugriff auf data sicherstellen */
    status = pthread_mutex_lock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_lock()", status );

    /* solange kein Platz im Puffer ist */
    while( ( ( data.in + 1 ) % MAX_NUMBERS ) == data.out )
    {
      /* abwarten und Mutex temporär freigeben */
      status = pthread_cond_wait( &data.rem, &data.mutex );
      if( status != 0 )
        error_exit( "pthread_cond_wait()", status );
    }

    /* Wert im Puffer speichern und Puffer weiterschalten */
    data.val[data.in] = atoi( input );
    data.in = ( data.in + 1 ) % MAX_NUMBERS;

    /* erfolgte Aktualisierung des Puffers mitteilen */
    status = pthread_cond_broadcast( &data.add );
    if( status != 0 )
      error_exit( "pthread_cond_broadcast()", status );

    /* exklusiven Zugriff auf data freigeben */
    status = pthread_mutex_unlock( &data.mutex );
    if( status != 0 )
      error_exit( "pthread_mutex_unlock()", status );
  }

  exit( 0 );
}
