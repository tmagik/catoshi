Release Process
====================

###update (commit) version in sources

	contrib/verifysfbinaries/verify.sh
	doc/README*
	share/setup.nsi
	src/clientversion.h (change CLIENT_VERSION_IS_RELEASE to true)

###tag version in mercurial

	(do something)

###write release notes.

###build a debian package

###check for reproducible build

###TODO: compare multiple sources of builds, like gitian

###Windows builds:

Look at https://bitbucket.org/tmagik/cxe

###Mac Builds
  
Wait for someone to write a nice brew formula

 - Have a brew once this works
