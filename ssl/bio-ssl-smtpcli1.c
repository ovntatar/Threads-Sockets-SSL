#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "openssl-lib-init.h"

int main( int argc, char *argv[] )
{
  BIO *bio;
  SSL_CTX *ctx;
  char buf[256];

  if( argc != 2 )
  {
    printf( "Usage: %s smtp-host\n", argv[0] );
    exit( EXIT_FAILURE );
  }

  /* SSL-Bibliothek und PRNG initialisieren */
  if( ! openssl_lib_init() )
  {
    printf( "Library/PRNG initialization failed.\n" );
    exit( EXIT_FAILURE );
  }

  /* SSL-Kontext für TLSv1-Verbindungen erstellen */
  if( ( ctx = SSL_CTX_new( TLSv1_method() ) ) == NULL )
  {
    printf( "Can't create SSL context ...\n" );
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* gepuffertes BIO zur SSL-Kommunikation erstellen */
  if( ( bio = BIO_new_buffer_ssl_connect( ctx ) ) == NULL )
  {
    printf( "Can't create SSL BIO ...\n" );
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* Verbindungsinformationen für das BIO-Objekt setzen */
  BIO_set_conn_hostname( bio, argv[1] );
  BIO_set_conn_port( bio, "ssmtp" );

  /* Verbindung aufbauen und SSL-Handshake durchführen */
  if( BIO_do_handshake( bio ) <= 0 )
  {
    printf( "SSL handshake failed ...\n");
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* Begrüßung des Mailservers empfangen */
  BIO_gets( bio, buf, 256 );
  printf( "%s", buf );

  /* ... und gleich wieder Adieu sagen */
  BIO_puts( bio, "QUIT\n" );
  BIO_flush( bio );

  /* Abschiedsgruß des Mailservers empfangen */
  BIO_gets( bio, buf, 256 );
  printf( "%s", buf );

  /* BIO freigeben und Verbindung beenden, Programmende */
  BIO_free_all( bio );
  exit( EXIT_SUCCESS );
}
