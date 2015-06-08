#include <stdlib.h>
#include <sys/select.h>

int select_input( int http, int https )
{
  fd_set read;
  struct timeval timeout;

  /* Deskriptormenge f�r die beiden Sockets vorbereiten */
  FD_ZERO( &read );
  FD_SET( http, &read );
  FD_SET( https, &read );

  /* Timeout auf zwei Minuten festlegen */
  timeout.tv_sec = 120;
  timeout.tv_usec = 0;

  /* Auf neue Daten warten, egal ob http oder https */
  select( MAX( http, https ) + 1, &read, NULL, NULL, &timeout );

  /* Zur�ckgegebene Socketdeskriptormenge interpretieren */
  if( FD_ISSET( http, &read ) ) /* Neue Daten �ber http */
    return( http ); /* http-Socket zur�ckgeben */
  if( FD_ISSET( https, &read ) ) /* Neue Daten �ber https */
    return( https ); /* https-Socket zur�ckgeben */

  return( -1 ); /* Timeout oder Fehler: keine neuen Daten */
}
