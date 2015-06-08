#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

void signal_handler( int sig )
{
  printf( "Ihre Bedenkzeit ist abgelaufen.\n" );
  exit( EXIT_FAILURE );
}

void print_sockaddr( struct sockaddr_storage *sa )
{
  struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
  struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)sa;
  int port;
  char ip_address[INET6_ADDRSTRLEN];

  if( sa->ss_family == AF_INET )
  {
    inet_ntop( AF_INET, (struct sockaddr *)&sa4->sin_addr,
      ip_address, INET6_ADDRSTRLEN );
    port = ntohs( sa4->sin_port );
  }
  else
  {
    inet_ntop( AF_INET6, (struct sockaddr *)&sa6->sin6_addr,
      ip_address, INET6_ADDRSTRLEN );
    port = ntohs( sa6->sin6_port );
  }

  printf( "IP %s, Port %d\n", ip_address, port );
}

int main( int argc, char *argv[] )
{
  char antwort[] = "Himbeerjoghurt";
  char eingabe[20];
  struct sigaction action, old_action;
  unsigned int size, i;
  struct sockaddr_storage sa;

  setvbuf( stdin, NULL, _IOLBF, 0 );
  setvbuf( stdout, NULL, _IOLBF, 0 );

  /* Socket-Adresse des lokalen Endpunkts ermitteln */
  size = sizeof( sa );
  if( getsockname( fileno( stdin ),
      (struct sockaddr *)&sa, &size ) == 0 )
  {
    printf( "Quizserver hört auf " );
    print_sockaddr( &sa );
  }

  /* Socket-Adresse des entfernten Endpunkts ermitteln */
  size = sizeof( sa );
  if( getpeername( fileno( stdin ),
      (struct sockaddr *)&sa, &size ) == 0 )
  {
    printf( "Quizclient kommt von " );
    print_sockaddr( &sa );
  }

  action.sa_handler = signal_handler;
  sigemptyset( &action.sa_mask );
  action.sa_flags = 0;

  if( sigaction( SIGALRM, &action, &old_action ) < 0 )
  {
    printf( "Konnte Handler nicht installieren: %s.\n",
      strerror( errno ) );
    return( EXIT_FAILURE );
  }

  printf( "Sie haben 20 Sekunden für die Antwort:\n" );
  printf( "Was ißt Sir Quickly am liebsten?\n" );

  alarm( 20 );

  fgets( eingabe, sizeof( eingabe ), stdin );

  /* Abschließende Zeilentrenner \n und \r entfernen */
  for( i = strlen( eingabe ) - 1; i >= 0 &&
      ( eingabe[i] == '\n' || eingabe[i] == '\r' ); i -- )
    eingabe[i] = '\0';

  if( strcmp( eingabe, antwort ) == 0 )
    printf( "Die Antwort ist richtig. Gratulation.\n" );
  else
    printf( "Leider falsch, richtig ist %s\n", antwort );

  exit( EXIT_SUCCESS );
}
