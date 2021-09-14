all: clean package-1_40_0

package-1_40_0: bayes-1_40_0
	mkdir -p bayes++-dev_2010.8.1-2_amd64/DEBIAN
	mkdir -p bayes++-dev_2010.8.1-2_amd64/usr/include
	mkdir -p bayes++-dev_2010.8.1-2_amd64/usr/lib
	mkdir -p bayes++-dev_2010.8.1-2_amd64/usr/share/doc/bayes++-dev
	cp -r Bayes++_1_40_0/BayesFilter bayes++-dev_2010.8.1-2_amd64/usr/include
	cp Bayes++_1_40_0/target/BayesFilter/gcc-4.4.3/release/link-static/libBayesFilter.a bayes++-dev_2010.8.1-2_amd64/usr/lib
	cp Bayes++_1_40_0/*.html bayes++-dev_2010.8.1-2_amd64/usr/share/doc/bayes++-dev
	cp -r ClassDocumentation bayes++-dev_2010.8.1-2_amd64/usr/share/doc/bayes++-dev
	cp control.bayes++.2010.8.1-2.amd64 bayes++-dev_2010.8.1-2_amd64/DEBIAN/control
	dpkg -b bayes++-dev_2010.8.1-2_amd64 .


package-2017:
	mkdir -p bayes++-dev_2017/DEBIAN
	mkdir -p bayes++-dev_2017/usr/include
	mkdir -p bayes++-dev_2017/usr/lib
	mkdir -p bayes++-dev_2017/usr/share/doc/bayes++-dev
	cp -r Bayes++_2017/BayesFilter bayes++-dev_2017/usr/include
	cp Bayes++_2017/target/BayesFilter/gcc-5.4.0/release/link-static/libBayesFilter.a bayes++-dev_2017/usr/lib
	cp Bayes++_2017/*.html bayes++-dev_2017/usr/share/doc/bayes++-dev
	cp -r ClassDocumentation bayes++-dev_2017/usr/share/doc/bayes++-dev
	cp control.bayes++.2017 bayes++-dev_2017/DEBIAN/control
	dpkg -b bayes++-dev_2017 .

package-1_40_0_i386: bayes-1_40_0
	mkdir -p bayes++-dev_2010.8.1-2_i386/DEBIAN
	mkdir -p bayes++-dev_2010.8.1-2_i386/usr/include
	mkdir -p bayes++-dev_2010.8.1-2_i386/usr/lib
	mkdir -p bayes++-dev_2010.8.1-2_i386/usr/share/doc/bayes++-dev
	cp -r Bayes++_1_40_0/BayesFilter bayes++-dev_2010.8.1-2_i386/usr/include
	cp Bayes++_1_40_0/target/BayesFilter/gcc-4.4.3/release/link-static/libBayesFilter.a bayes++-dev_2010.8.1-2_i386/usr/lib
	cp Bayes++_1_40_0/*.html bayes++-dev_2010.8.1-2_i386/usr/share/doc/bayes++-dev
	cp -r ClassDocumentation bayes++-dev_2010.8.1-2_i386/usr/share/doc/bayes++-dev
	cp control.bayes++.2010.8.1-2.i386 bayes++-dev_2010.8.1-2_i386/DEBIAN/control
	dpkg -b bayes++-dev_2010.8.1-2_i386 .

package-1_43_0: bayes-1_43_0
	mkdir -p bayes++-dev_2010.8.1-1_boost-143_amd64/DEBIAN
	mkdir -p bayes++-dev_2010.8.1-1_boost-143_amd64/usr/include
	mkdir -p bayes++-dev_2010.8.1-1_boost-143_amd64/usr/lib
	mkdir -p bayes++-dev_2010.8.1-1_boost-143_amd64/usr/share/doc/bayes++-dev
	cp -r Bayes++_1_43_0/BayesFilter bayes++-dev_2010.8.1-1_boost-143_amd64/usr/include
	cp Bayes++_1_43_0/target/BayesFilter/gcc-4.4.3/release/link-static/libBayesFilter.a bayes++-dev_2010.8.1-1_boost-143_amd64/usr/lib
	cp Bayes++_1_43_0/*.html bayes++-dev_2010.8.1-1_boost-143_amd64/usr/share/doc/bayes++-dev
	cp -r ClassDocumentation bayes++-dev_2010.8.1-1_boost-143_amd64/usr/share/doc/bayes++-dev
	cp control.bayes++.2010.8.1-1.amd64 bayes++-dev_2010.8.1-1_boost-143_amd64/DEBIAN/control
	dpkg -b bayes++-dev_2010.8.1-1_boost-143_amd64 .
	mkdir -p boost_1.43.0-1-local_amd64/DEBIAN
	mkdir -p boost_1.43.0-1-local_amd64/usr/local
	cp control.boost_1.43.0-1-local_amd64 boost_1.43.0-1-local_amd64/DEBIAN/control
	cp -r boost_1_43_0/include boost_1.43.0-1-local_amd64/usr/local
	cp -r boost_1_43_0/lib boost_1.43.0-1-local_amd64/usr/local
	dpkg -b boost_1.43.0-1-local_amd64 .

bayes-1_40_0:
	unzip -q bf-C++source-2010.8-1.zip
	tar -jxf bf-classdocumentation-2010.8.tar.bz2
	tar -jxf boost-build-2.0-m12.tar.bz2
	mv Bayes++ Bayes++_1_40_0
	cd Bayes++_1_40_0; patch -p1 -i ../bayes-2010.patch
	cd Bayes++_1_40_0; bjam toolset=gcc -sBOOST_ROOT="../boost-build"

bayes-2017:
	unzip -q bf-C++source-2017.3-1.zip
	tar -jxf bf-classdocumentation-2010.8.tar.bz2
	tar -jxf boost-build-2.0-m12.tar.bz2
	mv Bayes++ Bayes++_2017
#	cd Bayes++_2017; patch -p1 -i ../bayes-2010.patch
#	cd Bayes++_2017; bjam toolset=gcc -sBOOST_ROOT="../boost-build"

bayes-1_43_0: boost
	unzip -q bf-C++source-2010.8-1.zip
	tar -jxf bf-classdocumentation-2010.8.tar.bz2
	mv Bayes++ Bayes++_1_43_0
	cd Bayes++_1_43_0; patch -p1 -i ../bayes-2010.patch
	cd Bayes++_1_43_0; bjam toolset=gcc -sBOOST_ROOT="../boost_1_43_0"

boost:
	tar -jxvf boost_1_43_0.tar.bz2
	cd boost_1_43_0; ./bootstrap.sh --prefix=/usr/local/boost_1_43_0/
	cd boost_1_43_0; ./bjam

patch-system-boost:
	sudo cp ${UC_DIR}/system/customPackages/bayes++/src/symmetric.hpp /usr/include/boost/numeric/ublas/symmetric.hpp

clean:
	rm -rf bayes++-dev_2010.8.1-2_amd64 Bayes++_1_40_0 Bayes++_1_43_0 ClassDocumentation bayes++-dev_2010.8.1-1_boost-143_amd64 boost_1_43_0 boost_1.43.0-1-local_amd64

