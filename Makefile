
LDFLAGS?=-I/usr/include
CFLAGS?=-L/usr/lib
STRIP?=/usr/bin/strip
DESTDIR?=/usr

all:plymouth-lite checkmodifier plymouth-text

plymouth-lite:
	$(CC) $@.c ply-frame-buffer.c ply-image.c ply-console.c -o $(CURDIR)/$@ -lpng16 -lm -lz $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

checkmodifier:
	$(CC) $@.c -o $(CURDIR)/$@
	$(STRIP) $@

plymouth-text:
	$(CC) $@.c -o $(CURDIR)/$@
	$(STRIP) $@

install:
	install -D systemd/* $(DESTDIR)/lib/systemd/system/
	install -D -m 0755 checkmodifier $(DESTDIR)/sbin/checkmodifier
	install -D -m 0755 ply-image $(DESTDIR)/bin/plymouth-lite

clean:
	rm -f plymouth-lite checkmodifier plymouth-text

.PHONY:ply-image checkmodifier plymouth-text install clean
