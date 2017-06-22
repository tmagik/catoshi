// Copyright (c) 2014-2015 The Bitcoin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_COMPAT_ENDIAN_H
#define CODECOIN_COMPAT_ENDIAN_H

/* support Debian now, others later */

#include <stdint.h>

#include "compat/byteswap.h"

//#if defined(HAVE_ENDIAN_H)
#include <endian.h>
//#elif defined(HAVE_SYS_ENDIAN_H)
//#include <sys/endian.h>
//#endif

#endif // CODECOIN_COMPAT_ENDIAN_H
