CC = gcc
CFLAGS = -Wall

BINARIES = bitwise byteorder hostaddr ntoatest quiz3 quiz4 timeclient \
  timeclientudp timeclientudpconn timeserver

all: $(BINARIES)

bitwise: bitwise.o
	$(CC) -o $@ $@.o

byteorder: byteorder.o
	$(CC) -o $@ $@.o

hostaddr: hostaddr.o
	$(CC) -o $@ $@.o

ntoatest: ntoatest.o
	$(CC) -o $@ $@.o

quiz3: quiz3.o
	$(CC) -o $@ $@.o

quiz4: quiz4.o
	$(CC) -o $@ $@.o

timeclient: timeclient.o
	$(CC) -o $@ $@.o

timeclientudp: timeclientudp.o
	$(CC) -o $@ $@.o

timeclientudpconn: timeclientudpconn.o
	$(CC) -o $@ $@.o

timeserver: timeserver.o
	$(CC) -o $@ $@.o

clean:
	rm -f $(BINARIES) *.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c
