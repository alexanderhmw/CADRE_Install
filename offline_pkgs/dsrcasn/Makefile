all: clean package

package:
	mkdir -p libdsrcasn_gcc5/DEBIAN
	mkdir -p libdsrcasn_gcc5/usr/local/include
	mkdir -p libdsrcasn_gcc5/usr/local/lib
	cp -r locomatex86/incs/* libdsrcasn_gcc5/usr/local/include/.
	cp -r locomatex86/lib/* libdsrcasn_gcc5/usr/local/lib/.
	cp control.dsrc.amd64 libdsrcasn_gcc5/DEBIAN/control
	dpkg -b libdsrcasn_gcc5 .
