#include <stdio.h>
#include <stdlib.h>

#include <openssl/rand.h>

#define SEED_FILE "/var/tmp/my.seed"
#define EGD_SOCKET "/var/run/egd-pool"

int main( int argc, char *argv[] )
{
  int num, prng_ok;

  prng_ok = RAND_status(); /* PRNG bereits initialisiert? */

  if( ! prng_ok ) /* Falls nicht: selbst initialisieren! */
  {
    printf( "Der PRNG muß noch initialisiert werden.\n" );

    /* ein evtl. vorhandenes Seed-File laden */
    num = RAND_load_file( SEED_FILE, 1024 );
    printf( "%d Bytes aus %s bezogen.\n", num, SEED_FILE );

    /* Entropy Gathering Daemon (EGD) einbeziehen */
    num = RAND_egd( EGD_SOCKET );
    printf( "%d Bytes aus %s bezogen.\n", num, EGD_SOCKET );

    if( ! RAND_status() ) /* PRNG jetzt ok? */
    {
      printf( "Fehler bei der PRNG-Initialisierung.\n" );
      exit( EXIT_FAILURE );
    }
  }

  /* Hier würde das eigentliche SSL-Programm beginnen */

  /* Zum Schluß: Status des PRNG in Seed-File sichern ... */
  if( ! prng_ok ) /* falls ursprünglich uninitialisiert */
  {
    num = RAND_write_file( SEED_FILE );
    if( num < 0 )
      printf( "Achtung: Inhalt von %s fragwürdig!\n",
        SEED_FILE );
    else
      printf( "%d Bytes in %s geschrieben.\n", num,
        SEED_FILE );
  }

  return( EXIT_SUCCESS );
}
