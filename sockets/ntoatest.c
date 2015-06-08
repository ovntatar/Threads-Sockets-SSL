#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
  char *addr1, *addr2;
  struct in_addr ia1, ia2;

  /* Initialisierung mit beliebigen 32-Bit Werten */
  ia1.s_addr = (in_addr_t)16777343;
  ia2.s_addr = (in_addr_t)33663168;

  /* erste Adresse umwandeln und ausheben */
  addr1 = inet_ntoa( ia1 );
  printf( "1. IP-Adresse: %s\n", addr1 );

  /* zweite Adresse umwandeln und ausheben */
  addr2 = inet_ntoa( ia2 );
  printf( "2. IP-Adresse: %s\n", addr2 );

  /* ... und nochmal die erste Adresse: Autsch! */
  printf( "1. IP-Adresse: %s\n", addr1 );

  return( EXIT_SUCCESS );
}
