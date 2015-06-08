#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "openssl-lib-init.h"
#include "openssl-util.h"
#include "server.h"

void send_smtp_request( BIO *bio, const char *req )
{
  BIO_puts( bio, req );
  BIO_flush( bio );
  printf( "%s", req );
}

void print_smtp_response( BIO *bio )
{
  char buf[256];

  do
  {
    BIO_gets( bio, buf, 256 );
    printf( "%s", buf );
  } while( ( strlen( buf ) > 3 ) && ( buf[3] == '-' ) );
}

BIO *build_tcp_chain( int socket )
{
  BIO *buf, *tcp;

  /* gepuffertes BIO zur TCP-Kommunikation erstellen */
  if( ( ! ( buf = BIO_new( BIO_f_buffer() ) ) ) ||
      ( ! ( tcp = BIO_new_socket( socket, BIO_CLOSE ) ) ) )
    return( NULL );

  /* BIO-Kette "buffer -> socket" erstellen */
  return( BIO_push( buf, tcp ) );
}

BIO *build_ssl_chain( BIO *bio )
{
  BIO *ssl;
  SSL_CTX *ctx;

  /* SSL-Kontext mit Standardeinstellungen erstellen */
  if( ( ctx = openssl_create_ssl_ctx() ) == NULL )
    return( NULL );

  /* gepuffertes BIO zur SSL-Kommunikation erstellen */
  if( ( ssl = BIO_new_ssl( ctx, 1 ) ) == NULL )
    return( NULL );

  /* erst die BIO-Kette "ssl -> socket" und damit ... */
  ssl = BIO_push( ssl, BIO_pop( bio ) );
  /* die BIO-Kette "buffer -> ssl -> socket" erstellen */
  ssl = BIO_push( bio, ssl );

  /* Verbindung aufbauen und SSL-Handshake durchführen */
  if( BIO_do_handshake( ssl ) <= 0 )
    return( NULL );

  return( ssl );
}

int main( int argc, char *argv[] )
{
  BIO *bio;
  int srv;

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

  /* Neue TCP-Verbindung zum Server aufbauen */
  if( ( srv = tcp_connect( argv[1], "587" ) ) < 0 )
  {
    printf( "tcp_connect() failed.\n" );
    exit( EXIT_FAILURE );
  }

  /* gepuffertes BIO zur TCP-Kommunikation erstellen */
  if( ! ( bio = build_tcp_chain( srv ) ) )
  {
    printf( "build_tcp_chain() failed.\n" );
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* Frage/Antwort-Spielchen mit abschließenden STARTTLS */
  print_smtp_response( bio );
  send_smtp_request( bio, "EHLO indien\n" );
  print_smtp_response( bio );
  send_smtp_request( bio, "STARTTLS\n" );
  print_smtp_response( bio );

  /* gepuffertes BIO zur SSL-Kommunikation erstellen */
  if( ! ( bio = build_ssl_chain( bio ) ) )
  {
    printf( "Can't create SSL BIO ...\n" );
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  if( ! openssl_match_host_cert( bio, argv[1] ) )
  {
    printf( "Failed to verify peer certificate: "
      "subjectAltName or commonName doesn't match %s\n",
      argv[1] );
    exit( EXIT_FAILURE );
  }

  /* Frage/Antwort-Spielchen mit dem Server */
  send_smtp_request( bio, "EHLO indien\n" );
  print_smtp_response( bio );
  send_smtp_request( bio, "QUIT\n" );
  print_smtp_response( bio );

  /* BIO freigeben und Verbindung beenden, Programmende */
  BIO_free_all( bio );
  exit( EXIT_SUCCESS );
}
