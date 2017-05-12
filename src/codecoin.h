// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_codecoin_H
#define CODECOIN_codecoin_H

#include "chainparamsbase.h"

//#if !defined(BRANDING_ONLY)  // hack for resource codecoin-qt.rc
//#include <inttypes.h>
//#endif

#if defined(BRAND_litecoin)
#include "litecoin.h"
#elif defined(BRAND_solarcoin)
#include "solarcoin.h"
#elif defined(BRAND_catcoin)
#include "catcoin.h"
#elif defined(BRAND_kittycoin)
#include "kittycoin.h"
#elif defined(BRAND_uro)
#include "uro.h"
#elif defined(BRAND_grantcoin)
#include "grantcoin.h"
#elif defined(BRAND_grantstake)
#include "grantstake.h"
#elif defined(BRAND_givecoin) || defined(BRAND_givestake)
#include "givecoin.h"
#elif defined(BRAND_hamburger)
#include "hamburger.h"
#elif defined(BRAND_cleanwatercoin)
#include "cleanwatercoin.h"
#else
#error "No BRAND_*COIN defined!!"
#endif

#ifndef BRAND_SYM
#define BRAND_SYM BRAND_CODE
#endif

#if defined(BITCOIN_COMPAT)
/* define these for compat with existing code */
#ifndef CBlock
#define CBlock Block
#endif
#ifndef CBlockHeader
#define CBlockHeader BlockHeader
#endif
#endif

/* main.h do we always CheckProofOfWork in CheckIndex */
/* Litecoin does not do so for performance reasons */
#if defined(BRAND_bluecoin) || defined(BRAND_uro) || defined(BRAND_givestake)
#define CHECKINDEX_SLOW
#endif

#endif
