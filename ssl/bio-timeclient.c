#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <openssl/bio.h>

#define TIMESRVPORT "37"

int main( int argc, char *argv[] )
{
  BIO *bio_srv, *bio_stdout;
  time_t stime = 0;

  if( argc != 2 )
  {
    fprintf( stderr, "Usage: %s ipv4-address\n", argv[0] );
    exit( EXIT_FAILURE );
  }

  if( ( bio_srv = BIO_new( BIO_s_connect() ) ) == NULL ||
      ( bio_stdout = BIO_new( BIO_s_file() ) ) == NULL )
  {
    fprintf( stderr, "BIO_new() failed.\n" );
    exit( EXIT_FAILURE );
  }

  /* Verbindungsinformationen für das BIO-Objekt setzen */
  BIO_set_conn_hostname( bio_srv, argv[1] );
  BIO_set_conn_port( bio_srv, TIMESRVPORT );

  /* Spaßhalber ab jetzt auch die Standardausgabe via BIO */
  BIO_set_fp( bio_stdout, stdout, BIO_NOCLOSE );

  /* Neue TCP-Verbindung zum Server aufbauen */
  if( BIO_do_connect( bio_srv ) <= 0 )
  {
    fprintf( stderr, "BIO_do_connect() failed.\n" );
    exit( EXIT_FAILURE );
  }

  /* Ausgabe des Servers lesen */
  if( BIO_read( bio_srv, &stime, sizeof( stime ) ) <= 0 )
  {
    fprintf( stderr, "BIO_read() failed.\n" );
    BIO_free( bio_srv );
    exit( EXIT_FAILURE );
  }

  /* Sekunden auf Basis 1.1.1970 umrechnen und ausgeben */
  stime = ntohl( stime ) - 2208988800UL;
  BIO_puts( bio_stdout, ctime( &stime ) );

  /* BIOs freigeben und Verbindung beenden */
  BIO_free( bio_srv );
  BIO_free( bio_stdout );
  exit( EXIT_SUCCESS );
}
