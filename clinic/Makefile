CC = gcc
CFLAGS = -lssl -lcrypto

sslclient: sslclient.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm sslclient
