#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int sd, i, status;
  struct addrinfo hints, *ai, *aptr;
  char ip_address[INET6_ADDRSTRLEN];
  struct sockaddr_in *ipv4addr;
  struct sockaddr_in6 *ipv6addr;
  time_t stime = 0;

  memset( &hints, 0, sizeof( struct addrinfo ) );
  hints.ai_flags = AI_CANONNAME;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  for( i = 1; i < argc; i++ )
  {
    status = getaddrinfo( argv[i], "time", &hints, &ai );
    if( status == 0 )
    {
      printf( "Rechner %s (%s) ...\n", argv[i],
        ai->ai_canonname );

      for( aptr = ai; aptr != NULL; aptr = aptr->ai_next )
      {
        if( aptr->ai_family == AF_INET )
        {
          ipv4addr = (struct sockaddr_in *)aptr->ai_addr;
          inet_ntop( aptr->ai_family, &ipv4addr->sin_addr,
            ip_address, INET6_ADDRSTRLEN );
        }
        else
        {
          ipv6addr = (struct sockaddr_in6 *)aptr->ai_addr;
          inet_ntop( aptr->ai_family, &ipv6addr->sin6_addr,
            ip_address, INET6_ADDRSTRLEN );
        }

        /* TCP Socket anlegen */
        if( ( sd = socket( aptr->ai_family, SOCK_STREAM, 0 ) ) < 0 )
        {
          printf( "  %s -> socket(): %s\n", ip_address,
            strerror( errno ) );
          continue;
        }

        /* Verbindung zum Time Server aufbauen */
        if( connect( sd, aptr->ai_addr, aptr->ai_addrlen ) < 0 )
        {
          printf( "  %s -> connect(): %s\n", ip_address,
            strerror( errno ) );
          close( sd );
          continue;
        }

        /* Ausgabe des Servers lesen */
        if( read( sd, &stime, sizeof( stime ) ) < 0 )
        {
          printf( "  %s -> read(): %s\n", ip_address,
            strerror( errno ) );
          close( sd );
          continue;
        }

        /* Sekunden auf Basis 1.1.1970 umrechnen sowie
           IP-Adresse und Zeit ausgeben */
        stime = ntohl( stime ) - 2208988800UL;
        printf( "  %s -> Zeit: %s", ip_address,
          ctime( &stime ) );

        /* Socketdeskriptor schlieﬂen, Verbindung beenden */
        close( sd );
      }

      freeaddrinfo( ai );
    }
    else
    {
      fprintf( stderr, "Fehler in getaddrinfo(%s): %s\n",
        argv[i], gai_strerror( status ) );
    }
  }

  exit( EXIT_SUCCESS );
}
