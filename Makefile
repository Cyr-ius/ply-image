
LDFLAGS?=-I/usr/include
CFLAGS?=-L/usr/lib
STRIP?=/usr/bin/strip
DESTDIR?=/usr

all:plymouth-lite checkmodifier

plymouth-lite:
	$(CC) $@.c ply-frame-buffer.c -o $(CURDIR)/$@ -lpng16 -lm -lz $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

checkmodifier:
	$(CC) $@.c -o $(CURDIR)/$@
	$(STRIP) $@

install:
	install -D systemd/* $(DESTDIR)/lib/systemd/system/
	install -D -m 0755 checkmodifier $(DESTDIR)/sbin/checkmodifier
	install -D -m 0755 ply-image $(DESTDIR)/bin/plymouth-lite

clean:
	rm -f ply-image checkmodifier

.PHONY:ply-image checkmodifier install clean
