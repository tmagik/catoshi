// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_givecoin_H
#define CODECOIN_givecoin_H

#include "util.h"

static const int RPC_PORT = 31410;
static const int RPC_PORT_TESTNET =31411;
static const int P2P_PORT = 31415;
static const int P2P_PORT_TESTNET = 31416;

static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 100000; // 0.001 GIVE
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 1000;   // 0.00001 GIVE mininput
/** No amount larger than this (in catoshi) is valid */
static const int64_t MAX_MONEY = 500000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

/** Minimum block time spacing (hard limit) **/
static const int64_t MINIMUM_BLOCK_SPACING = 60;	// Absolute minimum spacing

/** really only used in rpcmining.cpp **/
static const int RETARGET_INTERVAL = 15;

#define BRAND "GiveCoin"
#define BRAND_upper "GiveCoin"
#define BRAND_lower "givecoin"
#define BRAND_domain "givecoin.org"

#endif
