#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "openssl-util.h"

int verify_cert( int ok, X509_STORE_CTX *x509ctx );

SSL_CTX *openssl_create_ssl_ctx( void )
{
  SSL_CTX *ctx;

  /* SSL-Kontext für SSLv2, SSLv3 und TLSv1 erstellen */
  if( ( ctx = SSL_CTX_new( SSLv23_method() ) ) == NULL )
    return( NULL );
  
  /* bitte kein SSL 2.0, dafür maximale Kompatibilität */
  SSL_CTX_set_options( ctx, SSL_OP_NO_SSLv2 | SSL_OP_ALL );

  /* Keine Wiederholungsanforderungen auf Anwendungsebene */
  SSL_CTX_set_mode( ctx, SSL_MODE_AUTO_RETRY );

  /* schwache Verschlüsselungsalgorithmen ausschließen */
  if( ! SSL_CTX_set_cipher_list( ctx, CIPHER_LIST ) )
  {
    SSL_CTX_free( ctx );
    return( NULL );
  }

  /* systemweite und eigene Zertifikatspeicher festlegen */
  if( ! ( SSL_CTX_set_default_verify_paths( ctx ) &&
    SSL_CTX_load_verify_locations( ctx, CAFILE, CAPATH ) ) )
  {
    SSL_CTX_free( ctx );
    return( NULL );
  }

  /* Zertfikatsprüfung aktivieren und Callback setzen */
  SSL_CTX_set_verify( ctx, SSL_VERIFY_PEER, verify_cert );

  /* maximale Hierarchietiefe an Zertifizierungsstellen */
  SSL_CTX_set_verify_depth( ctx, VERIFY_DEPTH + 1 );

  return( ctx );
}

int verify_cert( int ok, X509_STORE_CTX *x509ctx )
{
  X509 *cert;
  X509_NAME *subject, *issuer;
  int error, depth;

  /* welcher Fehler ist aufgetreten und ... */
  error = X509_STORE_CTX_get_error( x509ctx );
  /* ... wo in der Zertifikathierachie befinden wir uns? */
  depth = X509_STORE_CTX_get_error_depth( x509ctx );

  /* tiefer als die maximal gewünschte Hierarchietiefe? */
  if( depth > VERIFY_DEPTH )
  {
    /* falls ja, dann Fehlerursache setzen */
    error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
    /* und Rückgabewert modifizieren */
    ok = 0;
  }

  /* falls das Zertifikat nicht verifiziert werden konnte */
  if( ! ok )
  {
    /* Fehlermeldung ausgeben */
    printf( "Failed to verify certificate: %s\n",
      X509_verify_cert_error_string( error ) );
    if( depth > VERIFY_DEPTH )
      printf( "Maximum depth %d, current depth %d.\n",
        VERIFY_DEPTH, depth );

    /* zugehöriges Zertifikat bestimmen */
    if( ( cert = X509_STORE_CTX_get_current_cert( x509ctx ) ) )
    {
      /* Zusatzinfos zu Subject und Issuer ermitteln ... */
      subject = X509_get_subject_name( cert );
      issuer = X509_get_issuer_name( cert );

      /* ... und ausgeben */
      if( subject )
      {
        printf( "Certificate subject:\n" );
        X509_NAME_print_ex_fp( stdout, subject, 2,
          XN_FLAG_MULTILINE );
        printf( "\n" );
      }
      if( issuer )
      {
        printf( "Certificate issuer:\n" );
        X509_NAME_print_ex_fp( stdout, issuer, 2,
          XN_FLAG_MULTILINE );
        printf( "\n" );
      }
    }
  }

  return( ok );
}

int openssl_match_host_cert( BIO *ssl_bio, char *host )
{
  SSL *ssl;
  X509 *cert;
  STACK_OF( GENERAL_NAME ) *altnames;
  GENERAL_NAME *gn;
  X509_NAME *subject;
  int numaltnames, i, ok = -1;
  char *dns, commonname[256];

  if( ssl_bio == NULL || host == NULL )
    return( 0 );

  /* die zum BIO assoziierte SSL-Struktur ermitteln */
  BIO_get_ssl( ssl_bio, &ssl );
  if( ssl == NULL )
    return( 0 );

  /* das Zertifikat des Kommunikationspartners ermitteln */
  if( ! ( cert = SSL_get_peer_certificate( ssl ) ) )
    return( 0 );

  /* steckt im Zertifikat eine SubjectAltName-Extension? */
  if( ( altnames = X509_get_ext_d2i( cert,
        NID_subject_alt_name, NULL, NULL ) ) )
  {
    /* falls ja: wieviele GeneralNames enthält diese? */
    numaltnames = sk_GENERAL_NAME_num( altnames );

    /* alle GeneralNames der Extension durchsuchen */
    for( i = 0; i < numaltnames && ok <= 0; i++ )
    {
      /* i-ten GeneralName aus dem Stapel herausgreifen */
      gn = sk_GENERAL_NAME_value( altnames, i );

      /* Falls DNSName: Textinhalt ermitteln ... */
      if( gn->type == GEN_DNS &&
          ( dns = (char *)ASN1_STRING_data( gn->d.ia5 ) ) )
      {
        /* ... und mit dem Hostnamen vergleichen */
        ok = ( strcasecmp( dns, host ) == 0 );
        /* ok ist 1 bei einem Treffer, andernfalls 0 */
      }
    }

    /* zum Schluß die GeneralNames wieder freigeben */
    sk_GENERAL_NAME_free( altnames );
  }

  /*
   * Falls es im Zertifikat kein subjectAltName-Element vom
   * Typ DNS-Name gibt, dann gilt ok < 0 und der gesuchte
   * DNS-Name wird aus dem commonName im Subject des
   * Zertifikats ermittelt.
   */
  if( ( ok < 0 ) &&
      ( subject = X509_get_subject_name( cert ) ) &&
      ( X509_NAME_get_text_by_NID( subject, NID_commonName,
        commonname, 256 ) > 0 ) )
  {
    /* commonName mit dem Hostnamen vergleichen */
    ok = ( strcasecmp( commonname, host ) == 0 );
    /* ok ist 1 bei einem Treffer, andernfalls 0 */
  }

  X509_free( cert ); /* Zertifikat wieder freigeben */

  /* Ist ok > 0, dann steckt der Hostname im Zertifikat */
  return( ok > 0 );
}
