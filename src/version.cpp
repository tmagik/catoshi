// Copyright (c) 2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <string>

#include "version.h"

// Name of client reported in the 'version' message. Report the same name
// for both bitcoind and bitcoin-qt, to make it harder for attackers to
// target servers or GUI users specifically.
const std::string CLIENT_NAME("Satoshi");

// Client version number
#define CLIENT_VERSION_SUFFIX   "-beta"


// The following part of the code determines the CLIENT_BUILD variable.
// Several mechanisms are used for this:
// * first, if HAVE_BUILD_INFO is defined, include build.h, a file that is
//   generated by the build environment, possibly containing the output
//   of git-describe in a macro called BUILD_DESC
// * secondly, if this is an exported version of the code, GIT_ARCHIVE will
//   be defined (automatically using the export-subst git attribute), and
//   GIT_COMMIT will contain the commit id.
// * then, three options exist for determining CLIENT_BUILD:
//   * if BUILD_DESC is defined, use that literally (output of git-describe)
//   * if not, but GIT_COMMIT is defined, use v[maj].[min].[rev].[build]-g[commit]
//   * otherwise, use v[maj].[min].[rev].[build]-unk
// finally CLIENT_VERSION_SUFFIX is added

// First, include build.h if requested
#ifdef HAVE_BUILD_INFO
#    include "build.h"
#endif

// git will put "#define GIT_ARCHIVE 1" on the next line inside archives. $Format:%n#define GIT_ARCHIVE 1$
#ifdef GIT_ARCHIVE
#    define GIT_COMMIT_ID "$Format:%h$"
#    define GIT_COMMIT_DATE "$Format:%cD"
#endif

#define STRINGIFY(s) #s

#define BUILD_DESC_FROM_COMMIT(maj,min,rev,build,commit) \
    "v" STRINGIFY(maj) "." STRINGIFY(min) "." STRINGIFY(rev) "." STRINGIFY(build) "-g" commit

#define BUILD_DESC_FROM_UNKNOWN(maj,min,rev,build) \
    "v" STRINGIFY(maj) "." STRINGIFY(min) "." STRINGIFY(rev) "." STRINGIFY(build) "-unk"

#ifndef BUILD_DESC
#    ifdef GIT_COMMIT_ID
#        define BUILD_DESC BUILD_DESC_FROM_COMMIT(PPCOIN_VERSION_MAJOR, PPCOIN_VERSION_MINOR, PPCOIN_VERSION_REVISION, PPCOIN_VERSION_BUILD, GIT_COMMIT_ID)
#    else
#        define BUILD_DESC BUILD_DESC_FROM_UNKNOWN(PPCOIN_VERSION_MAJOR, PPCOIN_VERSION_MINOR, PPCOIN_VERSION_REVISION, PPCOIN_VERSION_BUILD)
#    endif
#endif

#ifndef BUILD_DATE
#    ifdef GIT_COMMIT_DATE
#        define BUILD_DATE GIT_COMMIT_DATE
#    else
#        define BUILD_DATE __DATE__ ", " __TIME__
#    endif
#endif

const std::string CLIENT_BUILD(BUILD_DESC CLIENT_VERSION_SUFFIX);
const std::string CLIENT_DATE(BUILD_DATE);
