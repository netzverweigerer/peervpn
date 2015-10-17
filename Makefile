CFLAGS+=-O2
LIBS+=-lcrypto -lz

all: peervpn
peervpn: peervpn.o
	git describe --tags > version
	$(CC) $(LDFLAGS) peervpn.o $(LIBS) -o $@
peervpn.o: peervpn.c

clean:
	rm -f peervpn peervpn.o
