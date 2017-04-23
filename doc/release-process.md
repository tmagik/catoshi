Release Process
====================

* update translations (ping wumpus, Diapolo or tcatm on IRC)
* see https://github.com/bitcoin/bitcoin/blob/master/doc/translation_process.md#syncing-with-transifex

* * *

###update (commit) version in sources


	bitcoin-qt.pro
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
