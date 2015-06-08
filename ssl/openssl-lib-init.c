#include <stdio.h>

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

/* EGD_SOCKET und  Prototyp für openssl_lib_init() */
#include "openssl-lib-init.h"

int openssl_lib_init( void )
{
  SSL_library_init(); /* SSL-Bibliothek initialisieren */

  if( ! RAND_status() ) /* PRNG ok? */
  {
    RAND_egd( EGD_SOCKET ); /* EGD zu Hilfe nehmen */
    if( ! RAND_status() ) /* PRNG jetzt ok? */
      return( 0 );
  }

  /* OpenSSL-Fehlerstrings laden */
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  return( 1 );
}
