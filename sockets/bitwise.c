#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int is_ipv4( const char *ip )
{
  while( ( *ip == '.' ) || isdigit( *ip ) )
    ip++;
  return( *ip == '\0' );
}

void print_bitwise( int af, const uint8_t ip[] )
{
  int i;
  uint8_t j;

  for( i = 0; i < ( af == AF_INET ? 4 : 16 ); i++ )
  {
    for( j = ( 1 << 7 ); j > 0; j >>= 1 )
      printf( "%d", ( ip[i] & j ) ? 1 : 0 );
    printf( "%s", ( i % 4 ) == 3 ? "\n" : " " );
  }
}

int main( int argc, char *argv[] )
{
  int i;
  char ip_address[INET6_ADDRSTRLEN];
  struct in_addr ipv4;
  struct in6_addr ipv6;

  for( i = 1; i < argc; i++ )
  {
    if( is_ipv4( argv[i] ) )
    {
      if( inet_pton( AF_INET, argv[i], &ipv4 ) != 1 )
      {
        printf( "ungültige Adresse: %s\n", argv[i] );
        continue;
      }
      printf( "IPv4 Adresse: %s\n", argv[i] );
      print_bitwise( AF_INET, (uint8_t *)&ipv4.s_addr );
      inet_ntop( AF_INET, &ipv4, ip_address,
        INET6_ADDRSTRLEN );
      printf( "IPv4 Adresse: %s\n", ip_address );
    }
    else
    {
      if( inet_pton( AF_INET6, argv[i], &ipv6 ) != 1 )
      {
        printf( "ungültige Adresse: %s\n", argv[i] );
        continue;
      }
      printf( "IPv6 Adresse: %s\n", argv[i] );
      print_bitwise( AF_INET6, ipv6.s6_addr );
      inet_ntop( AF_INET6, &ipv6, ip_address,
        INET6_ADDRSTRLEN );
      printf( "IPv6 Adresse: %s\n", ip_address );
    }
  }

  return( EXIT_SUCCESS );
}
