#include <pthread.h>
#include <stdlib.h>

#include <openssl/crypto.h>

/* Enthält Prototyp für openssl_thread_init() */
#include "openssl-thread-init.h"

struct CRYPTO_dynlock_value
{
  /* Realisierung des DynLocks über einen Pthreads-Mutex */
  pthread_mutex_t mutex;
};

/* Zeiger auf ein festes Feld von Mutex-Variablen */
pthread_mutex_t *openssl_mutex = NULL;

unsigned long openssl_thread_id( void )
{
  /* Typumwandlung von pthread_t nach unsigned long */
  return( (unsigned long)pthread_self() );
}

void openssl_mutex_lock( int mode, int n, const char *file,
  int line )
{
  if( mode & CRYPTO_LOCK ) /* Lock oder Unlock? */
    pthread_mutex_lock( &openssl_mutex[n] );
  else
    pthread_mutex_unlock( &openssl_mutex[n] );
}

struct CRYPTO_dynlock_value *openssl_dl_create(
  const char *file, int line )
{
  int st;
  struct CRYPTO_dynlock_value *dl;

  /* Speicher für neuen Mutex anfordern ... */
  dl = malloc( sizeof( pthread_t ) );
  if( dl != NULL )
  {
    /* ... und Mutex initialisieren */
    st = pthread_mutex_init( &dl->mutex, NULL );
    if( st != 0 ) /* Fehler beim Mutex Initialisieren? */
    {
      free( dl ); /* belegten Speicher wieder freigeben */
      dl = NULL; /* ... und Nullzeiger zurückgeben */
    }
  }

  return( dl ); /* Zeiger auf neue Struktur liefern */
}

void openssl_dl_destroy(
  struct CRYPTO_dynlock_value *dl, const char *file,
  int line )
{
  /* Mutex zerstören und Speicher freigeben */
  pthread_mutex_destroy( &dl->mutex );
  free( dl );
}

void openssl_dl_lock( int mode,
  struct CRYPTO_dynlock_value *dl, const char *file,
  int line )
{
  if( mode & CRYPTO_LOCK ) /* Lock oder Unlock? */
    pthread_mutex_lock( &dl->mutex );
  else
    pthread_mutex_unlock( &dl->mutex );
}

int openssl_thread_init( void )
{
  int i, st, max = CRYPTO_num_locks();

  /* Initialisierung des Felds der statischen Mutexe */
  if( openssl_mutex == NULL ) /* schon initialisiert? */
  {
    /* Feld mit Mutex-Variablen anlegen ... */
    openssl_mutex = calloc( max, sizeof( pthread_mutex_t ) );
    if( openssl_mutex == NULL )
      return( 0 ); /* Rücksprung mit Fehler */

    /* ... und Mutex-Variablen initialisieren */
    for( i = 0; i < max; i++ )
    {
      st = pthread_mutex_init( &openssl_mutex[i], NULL );
      if( st != 0 )
        return( 0 ); /* Rücksprung mit Fehler */
    }
  }

  /* Callback zur Bestimmung der Thread-ID registrieren */
  CRYPTO_set_id_callback( openssl_thread_id );

  /* statischen Mutex-Lock-Callback registrieren */
  CRYPTO_set_locking_callback( openssl_mutex_lock );

  /* Callbacks für dynamische Mutexe registrieren */
  CRYPTO_set_dynlock_create_callback( openssl_dl_create );
  CRYPTO_set_dynlock_destroy_callback( openssl_dl_destroy );
  CRYPTO_set_dynlock_lock_callback( openssl_dl_lock );

  return( 1 ); /* Erfolgsmeldung zurückgeben */
}
