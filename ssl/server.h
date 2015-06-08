#ifndef SERVER_EXAMPLE_H
#define SERVER_EXAMPLE_H

#include <stdlib.h>

/* Allgemeine Definitionen */

#define BACKLOG 32 /* Länge der Listen-Queue */
#define PIDFILE "/var/run/smtpsrv.pid"

#define CERTFILE "/wo/auch/immer/smtp-cert.pem"
#define KEYFILE "/wo/auch/immer/smtp-key.pem"

/* Deklaration der externen Funktionen */

void daemon_init( const char *program, const char *pid_file,
  int facility );

int tcp_connect( const char *nodename,
  const char *servname );
int tcp_listen( const char *nodename,
  const char *servname, int backlog );

#endif
