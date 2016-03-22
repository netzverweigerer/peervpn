CFLAGS+=-O2
LIBS+=-lcrypto -lz

all: peervpn
peervpn: peervpn.o
	$(CC) $(LDFLAGS) peervpn.o $(LIBS) -o $@
peervpn.o: peervpn.c

install:
	install peervpn /usr/local/sbin/peervpn
clean:
	rm -f peervpn peervpn.o
