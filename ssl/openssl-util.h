#ifndef OPENSSL_UTIL_H
#define OPENSSL_UTIL_H

#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

#define CAFILE NULL
#define CAPATH "/wo/auch/immer"

/*
 * Maximal akzeptierte Tiefe der Zertifizierunghierarchie
 * bei der Prüfung von Zertifikaten: In OpenSSL-Versionen
 * vor 0.9.6 existiert eine Sicherheitslücke bei der
 * Zertifikatsprüfung, die sich mit der maximalen Tiefe 1
 * auf Kosten der Zertifizierunghierarchie eliminieren läßt.
 */

#if ( OPENSSL_VERSION_NUMBER < 0x0090600FL )
#define VERIFY_DEPTH 1
#else
#define VERIFY_DEPTH 3
#endif

SSL_CTX *openssl_create_ssl_ctx( void );
int openssl_match_host_cert( BIO *ssl_bio, char *host );

#endif
