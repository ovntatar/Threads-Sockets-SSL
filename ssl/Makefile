CC = gcc
CFLAGS = -Wall

BINARIES = bio-timeclient bio-ssl-smtpsrv bio-ssl-smtpcli1 bio-ssl-smtpcli2 openssl-rand-seed

SMTPCLI1_OBJS = bio-ssl-smtpcli1.o openssl-lib-init.o
SMTPCLI2_OBJS = bio-ssl-smtpcli2.o openssl-lib-init.o openssl-util.o tcp-connect.o
SMTPSRV_OBJS = bio-ssl-smtpsrv.o daemon-init.o openssl-lib-init.o openssl-util.o tcp-listen.o

all: $(BINARIES)

bio-timeclient: bio-timeclient.o
	$(CC) -o $@ bio-timeclient.o -lcrypto

bio-ssl-smtpcli1: $(SMTPCLI1_OBJS)
	$(CC) -o $@ $(SMTPCLI1_OBJS) -lcrypto -lssl

bio-ssl-smtpcli2: $(SMTPCLI2_OBJS)
	$(CC) -o $@ $(SMTPCLI2_OBJS) -lcrypto -lssl

openssl-rand-seed: openssl-rand-seed.o
	$(CC) -o $@ openssl-rand-seed.o -lcrypto

bio-ssl-smtpsrv: $(SMTPSRV_OBJS)
	$(CC) -o $@ $(SMTPSRV_OBJS) -lcrypto -lssl

bio-ssl-smtpcli2.o: server.h
bio-ssl-smtpsrv.o: server.h
openssl-lib-init.o:openssl-lib-init.h
openssl-thread-init.o: openssl-thread-init.h
openssl-util.o: openssl-util.h
daemon-init.o: server.h
tcp-connect.o: server.h
tcp-listen.o: server.h

clean:
	rm -f $(BINARIES) *.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c
