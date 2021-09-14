all:

package:
	#svn log | ./svnlog_to_dpkg.pl > debian/changelog
	fakeroot ./debian/rules binary

package-clean:
	fakeroot ./debian/rules clean
