#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "openssl-lib-init.h"
#include "openssl-util.h"
#include "server.h"

int daemon_exit = 0;
SSL_CTX *ctx;

void sig_handler( int sig )
{
  daemon_exit = 1;
  return;
}

void errors2syslog( void )
{
  unsigned long code;

  /* Solange ein Fehler in der Queue ist, gilt code != 0 */
  while( ( code = ERR_get_error() ) != 0 )
    syslog( LOG_ERR, "%s", ERR_error_string( code, NULL ) );
}

int setup_openssl( void )
{
  /* SSL-Bibliothek und PRNG initialisieren */
  if( ! openssl_lib_init() )
  {
    printf( "Library/PRNG initialization failed.\n" );
    return( 0 );
  }

  /* SSL-Kontext mit Standardeinstellungen erstellen */
  if( ( ctx = openssl_create_ssl_ctx() ) == NULL )
  {
    printf( "Can't create SSL context.\n" );
    return( 0 );
  }

  if( SSL_CTX_use_certificate_chain_file( ctx,
      CERTFILE ) <= 0 )
  {
    printf( "Can't load certificate.\n" );
    return( 0 );
  }

  if( SSL_CTX_use_PrivateKey_file( ctx, KEYFILE,
      SSL_FILETYPE_PEM ) <= 0 )
  {
    printf( "Can't load private key.\n" );
    return( 0 );
  }

  return( 1 );
}

int send_smtp_response( BIO *bio, const char *res )
{
  int num;

  if( ( num = BIO_puts( bio, res ) ) > 0 )
    BIO_flush( bio );
  return( num );
}

BIO *build_ssl_chain( BIO *bio )
{
  BIO *ssl;

  /* gepuffertes BIO zur SSL-Kommunikation erstellen */
  if( ( ssl = BIO_new_ssl( ctx, 0 ) ) == NULL )
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

void handle_client( BIO *bio )
{
  char buf[256], res[256], *cmd, *arg, *tmp;
  int tls = 0;

  send_smtp_response( bio,
    "220 manhattan.irgendwie-sowieso.dom ESMTP smtpsrv\n" );

  while( BIO_gets( bio, buf, 256 ) > 0 )
  {
    if( ( cmd = strtok_r( buf, " \r\n", &tmp ) ) == NULL )
      continue; /* Leerzeile oder nur Whitespaces? */
    arg = strtok_r( NULL, " \r\n", &tmp );

    if( strcasecmp( cmd, "EHLO" ) == 0 )
    {
      snprintf( res, 256,
        "250-manhattan.irgendwie-sowieso.dom Hello %s\n"
        "250-%s\n"
        "250 HELP\n",
        arg ? arg : "unknown", /* Nullzeiger abfangen! */
        tls ? "AUTH PLAIN LOGIN" : "STARTTLS" );
      send_smtp_response( bio, res );
    }
    else if( strcasecmp( cmd, "STARTTLS" ) == 0 )
    {
      if( tls )
      {
        send_smtp_response( bio,
          "503 STARTTLS not advertised\n" );
      }
      else
      {
        send_smtp_response( bio, "220 TLS go ahead\n" );

        /* Socket-BIO-Kette in SSL-BIO-Kette umwandeln */
        if( ! ( bio = build_ssl_chain( bio ) ) )
        {
          syslog( LOG_ERR, "Can't create SSL BIO.\n" );
          errors2syslog();
          break;
        }

        tls = 1; /* Merker: Wir sprechen SSL/TLS! */
      }
    }
    else if( strcasecmp( cmd, "QUIT" ) == 0 )
    {
      send_smtp_response( bio, "221 closing connection\n" );
      break;
    }
    else
      send_smtp_response( bio, "500 unknown command\n" );
  }
}

int main( int argc, char *argv[] )
{
  BIO *bio, *buf, *client;
  int srv;
  struct sigaction action;

  /* SSL-Bibliothek und SSL-Kontext initialisieren */
  if( ! setup_openssl() )
  {
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* horchenden Socket öffnen (passive open) */
  if( ( srv = tcp_listen( NULL, "587", BACKLOG ) ) < 0 )
  {
    printf( "Can't create listening socket.\n" );
    exit( EXIT_FAILURE );
  }

  /* BIO-Objekte erstellen und initialisieren */
  if( ! ( buf = BIO_new( BIO_f_buffer() ) ) ||
      ! ( bio = BIO_new( BIO_s_accept() ) ) ||
      ! ( BIO_set_fd( bio, srv, BIO_CLOSE ) ) )
  {
    printf( "Can't create accept BIO.\n" );
    ERR_print_errors_fp( stdout );
    exit( EXIT_FAILURE );
  }

  /* Puffer-BIO Template für neue Verbindungen setzen */
  BIO_set_accept_bios( bio, buf );

  /* Signalbehandlungsroutine für SIGTERM installieren */
  action.sa_handler = sig_handler;
  sigemptyset( &action.sa_mask );
  action.sa_flags = 0;

  if( sigaction( SIGTERM, &action, NULL ) < 0 )
  {
    printf( "sigaction() failed: %s", strerror( errno ) );
    BIO_free_all( bio ); /* Accept-BIO schließen */
    exit( EXIT_FAILURE );
  }

  /* Prozeß in einen Daemon umwandeln */
  daemon_init( argv[0], PIDFILE, LOG_DAEMON );

  for(;;)
  {
    /* Auf neue eingehende Verbindungen warten */
    if( BIO_do_accept( bio ) <= 0 )
    {
      if( daemon_exit ) /* Falls ein SIGTERM kam: Ende */
        break;

      syslog( LOG_ERR, "BIO_do_accept() failed.\n" );
      errors2syslog();

      /* Trotz Fehler brechen wir nicht ab! */
      continue;
    }

    /* Accept-BIO aus der Kette lösen, liefert Client-BIO */
    client = BIO_pop( bio );

    /* Clientverbindung sequentiell behandeln */
    handle_client( client );

    /* Clientverbindung trennen und BIO-Kette freigeben */
    BIO_free_all( client );
  }

  /* BIO freigeben und Verbindung beenden, Programmende */
  BIO_free( bio );
  exit( EXIT_SUCCESS );
}
