#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBERS 10

typedef struct intbuffer {
  int val[MAX_NUMBERS];
  int in, out;
} intbuffer_t;

intbuffer_t data;

void *avg( void *arg )
{
  int sum = 0, num = 0;

  for(;;) /* Endlosschleife */
  {
    if( data.in != data.out ) /* liegt neuer Wert vor? */
    {
      /* Wert aufsummieren und Puffer weiterschalten */
      sum += data.val[data.out];
      data.out = ( data.out + 1 ) % MAX_NUMBERS;

      num++; /* internen Zähler inkrementieren */

      printf( "Durchschnitt der %d Werte: %lf\n", num,
        (double)sum / num );
    }
  }

  return( NULL );
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  char input[32];
  int status;

  /* Datenpuffer initialisieren */
  data.in = 0;
  data.out = 0;

  /* Verbraucher-Thread starten */
  status = pthread_create( &tid, NULL, avg, NULL );
  if( status != 0 )
  {
    printf( "Fehler in pthread_create(): %s\n",
      strerror( status ) );
    exit( 1 );
  }

  for(;;) /* Endlosschleife */
  {
    /* Einen neuen Wert einlesen ... */
    printf( "input> " );
    fgets( input, sizeof( input ), stdin );

    /* Wert im Puffer speichern und Puffer weiterschalten */
    if( ( ( data.in + 1 ) % MAX_NUMBERS ) != data.out )
    {
      data.val[data.in] = atoi( input );
      data.in = ( data.in + 1 ) % MAX_NUMBERS;
    }
    else
      printf( "Puffer voll, Eingabe wiederholen.\n" );
  }

  exit( 0 );
}
