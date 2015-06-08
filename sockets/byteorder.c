#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
  uint16_t host_val, net_val;

  /* Initialisierung in Host Byte Order */
  host_val = 0x0001;
  printf( "Host Byte Order: host_val = %04x\n", host_val );

  /* Umwandlung in Network Byte Order */
  net_val = htons( host_val );
  printf( "Network Byte Order: net_val = %04x\n", net_val );

  /* Gilt "Host Byte Order = Network Byte Order"? */
  printf( "Ich bin ein %s-Endian-System.\n",
    ( host_val == net_val ) ? "Big" : "Little" );

  return( EXIT_SUCCESS );
}
