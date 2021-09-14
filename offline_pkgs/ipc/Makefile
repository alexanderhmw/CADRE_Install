all: clean package-3_9_1

package-3_9_1: ipc-3_9_1
	mkdir -p ipc-3.9.1_amd64/DEBIAN
	mkdir -p ipc-3.9.1_amd64/usr/bin
	mkdir -p ipc-3.9.1_amd64/usr/include
	mkdir -p ipc-3.9.1_amd64/usr/include/ipc
	mkdir -p ipc-3.9.1_amd64/usr/lib
	mkdir -p ipc-3.9.1_amd64/usr/sbin
	mkdir -p ipc-3.9.1_amd64/usr/share
	cp -r ipc-3.9.1/bin/Linux-4.10/* ipc-3.9.1_amd64/usr/bin/.
	cp -r ipc-3.9.1/include/* ipc-3.9.1_amd64/usr/include/ipc/.
	cp -r ipc-3.9.1/lib/Linux-4.10/* ipc-3.9.1_amd64/usr/lib/.
	cp control.ipc-3.9.1.amd64 ipc-3.9.1_amd64/DEBIAN/control
	dpkg -b ipc-3.9.1_amd64 .

ipc-3_9_1:
	tar -xvf ipc-3.9.1a.tar.gz
	cd ipc-3.9.1/src; make MAKE_SHARED_LIBS=1
	cd ipc-3.9.1/src;  make install

clean:
	rm -rf ipc-3.9.1

