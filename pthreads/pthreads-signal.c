#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERRUPTS 3
#define TIMEOUT 15

pthread_mutex_t sig_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sig_cond = PTHREAD_COND_INITIALIZER;
int sig_count = 0;

void error_exit( char *message, int status )
{
  printf( "%s: %s\n", message, strerror( status ) );
  exit( 1 );
}

void *sigcatcher( void *arg )
{
  sigset_t sigset;
  int status, signal;

  /* Signalmaske initialisieren */
  sigemptyset( &sigset );
  sigaddset( &sigset, SIGINT );

  for(;;)
  {
    /* eintreffende Signale synchron annehmen */
    sigwait( &sigset, &signal );
    if( signal == SIGINT )
    {
      printf( "Ctrl-C abgefangen.\n" );

      status = pthread_mutex_lock( &sig_mutex );
      if( status != 0 )
        error_exit( "pthread_mutex_lock()", status );

      sig_count++; /* Signal zählen und Änderung melden */

      status = pthread_cond_signal( &sig_cond );
      if( status != 0 )
        error_exit( "pthread_cond_signal()", status );

      status = pthread_mutex_unlock( &sig_mutex );
      if( status != 0 )
        error_exit( "pthread_mutex_unlock()", status );
    }
  }
}

int main( int argc, char *argv[] )
{
  pthread_t tid;
  sigset_t sigset;
  struct timespec timeout;
  int status;

  /* Signalmaske initialisieren */
  sigemptyset( &sigset );
  sigaddset( &sigset, SIGINT );

  /* Signalmaske für den main()-Thread setzen  */
  status = pthread_sigmask( SIG_BLOCK, &sigset, NULL );
  if( status != 0 )
    error_exit( "pthread_sigmask()", status );

  /* Der sigcatcher-Thread erbt die Signalmaske */
  status = pthread_create( &tid, NULL, sigcatcher, NULL );
  if( status != 0 )
    error_exit( "pthread_create()", status );
  
  /* Thread entkoppeln, es folgt kein pthread_join() mehr */
  status = pthread_detach( tid );
  if( status != 0 )
    error_exit( "pthread_detach()", status );

  /* relativen Timeout in absolute Zeitangabe umwandeln */
  timeout.tv_sec = time( NULL ) + TIMEOUT;
  timeout.tv_nsec = 0;

  printf( "Drück %d mal Ctrl-C und alles ist ok.\n",
    INTERRUPTS );

  status = pthread_mutex_lock( &sig_mutex );
  if( status != 0 )
    error_exit( "pthread_mutex_lock()", status );

  /* gewünschte Anzahl von Signalen oder Timeout abwarten */
  while( sig_count < INTERRUPTS )
  {
    status = pthread_cond_timedwait( &sig_cond, &sig_mutex,
      &timeout );
    if( status == ETIMEDOUT )
      break;
    else if( status != 0 )
      error_exit( "pthread_cond_timedwait()", status );
  }

  if( sig_count < INTERRUPTS )
    printf( "Timeout!\n" );
  else
    printf( "Na gut, dann hören wir halt auf.\n" );

  status = pthread_mutex_unlock( &sig_mutex );
  if( status != 0 )
    error_exit( "pthread_mutex_unlock()", status );

  exit( 0 );
}
