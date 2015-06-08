#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SRVPORT 1037
#define BACKLOG 32

int main( int argc, char *argv[] )
{
  int sd, client;
  struct sockaddr_in sa;
  time_t stime;

  /* TCP Socket anlegen */
  if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
  {
    printf( "socket() failed: %s\n", strerror( errno ) );
    exit( EXIT_FAILURE );
  }

  /* Initialisierung der Socket-Adreßstruktur */
  memset( &sa, 0, sizeof( sa ) ); /* erst alles auf 0 */
  sa.sin_family = AF_INET; /* IPv4 */
  sa.sin_port = htons( SRVPORT ); /* Time Server Port */
  sa.sin_addr.s_addr = htonl( INADDR_ANY ); /* Wildcard */

  /* Socket an Socket-Adresse binden */
  if( bind( sd, (struct sockaddr *)&sa,
      sizeof( sa ) ) < 0 )
  {
    printf( "bind() failed: %s\n", strerror( errno ) );
    close( sd );
    exit( EXIT_FAILURE );
  }

  /* aktiven Socket in passiven Socket umwandeln */
  if( listen( sd, BACKLOG ) < 0 )
  {
    printf( "listen() failed: %s\n", strerror( errno ) );
    close( sd );
    exit( EXIT_FAILURE );
  }

  for(;;)
  {
    /* Neue Socketverbindung annehmen */
    if( ( client = accept( sd, NULL, NULL ) ) < 0 )
    {
      printf( "accept() failed: %s\n", strerror( errno ) );
      close( sd );
      exit( EXIT_FAILURE );
    }

    /* Sekunden auf Basis 1.1.1900 umrechnen und senden */
    stime = htonl( (long)time( NULL ) + 2208988800UL );
    write( client, &stime, sizeof( stime ) );

    /* Socketdeskriptor schließen, Verbindung beenden */
    close( client );
  }
}
