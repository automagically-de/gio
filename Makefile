
obj-m := ${XMOD:.ko=.o}

MIPS_KERNEL_DIR = ${HOME}/src/linux-2.6-mips
GIO_VERSION = 0.1

all: gio.ko

%.ko: %.c
	${MAKE} ARCH=mips -C ${MIPS_KERNEL_DIR} XMOD=$@ SUBDIRS=`pwd` modules

clean:
	rm -f *.o *.ko

dist:
	mkdir -p gio
	cp Makefile gio.c gio.h gio/
	tar zcvf gio-${GIO_VERSION}.tar.gz gio/
