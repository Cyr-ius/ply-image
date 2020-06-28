
LDFLAGS?=-I/usr/include
CFLAGS?=-L/usr/lib
STRIP?=strip
DESTDIR:=/usr

all:ply-image checkmodifier

ply-image:
	$(CC) $@.c ply-frame-buffer.c -o $(CURDIR)/$@ -lpng16 -lm -lz $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

checkmodifier:
	$(CC) $@.c -o $(CURDIR)/$@
	$(STRIP) $@

install:
	install -D checkmodifier $(DESTDIR)/sbin/checkmodifier
	install -D ply-image $(DESTDIR)/usr/bin/ply-image

clean:
	rm -f ply-image checkmodifier

.PHONY:ply-image checkmodifier install clean
