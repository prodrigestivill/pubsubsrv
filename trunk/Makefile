include config.mk

all:
	cd src && ${MAKE} all
	cd libsrc && ${MAKE} all

clean:
	cd src && ${MAKE} clean
	cd libsrc && ${MAKE} clean

install:
	install -D -v -m 755 -s src/pubsubsrv ${DESTDIR}${PREFIX}/bin/pubsubsrv
	install -D -v -m 644  libsrc/libpubsub_textline.a ${DESTDIR}${PREFIX}/lib/libpubsub_textline.a
	install -D -v -m 644  libsrc/textline.h ${DESTDIR}${PREFIX}/include/pubsub/textline.h
