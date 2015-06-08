#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int sd;
  struct sockaddr_in sa;
  time_t stime = 0;

  if( argc != 2 )
  {
    printf( "Usage: %s ipv4-address\n", argv[0] );
    exit( EXIT_FAILURE );
  }

  /* TCP Socket anlegen */
  if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
  {
    printf( "socket() failed: %s\n", strerror( errno ) );
    exit( EXIT_FAILURE );
  }

  /* Initialisierung der Socket-Adreßstruktur */
  memset( &sa, 0, sizeof( sa ) ); /* erst alles auf 0 */
  sa.sin_family = AF_INET; /* IPv4 */
  sa.sin_port = htons( 37 ); /* Time Server Port */
  /* IPv4-Adresse in Netzwerkdarstellung einsetzen */
  if( inet_pton( AF_INET, argv[1], &sa.sin_addr ) != 1 )
  {
    printf( "inet_pton() failed.\n" );
    close( sd );
    exit( EXIT_FAILURE );
  }

  /* Verbindung zum Time Server aufbauen */
  if( connect( sd, (struct sockaddr *)&sa,
      sizeof( sa ) ) < 0 )
  {
    printf( "connect() failed: %s\n", strerror( errno ) );
    close( sd );
    exit( EXIT_FAILURE );
  }

  /* Ausgabe des Servers lesen */
  if( read( sd, &stime, sizeof( stime ) ) < 0 )
  {
    printf( "read() failed: %s\n", strerror( errno ) );
    close( sd );
    exit( EXIT_FAILURE );
  }

  /* Sekunden auf Basis 1.1.1970 umrechnen und ausgeben */
  stime = ntohl( stime ) - 2208988800UL;
  printf( "%s", ctime( &stime ) );

  /* Socketdeskriptor schließen, Verbindung beenden */
  close( sd );
  exit( EXIT_SUCCESS );
}
