// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_7coin_H
#define CODECOIN_7coin_H

#include "util.h"

static const int RPC_PORT = 9932;
static const int RPC_PORT_TESTNET = 19932;
static const int P2P_PORT = 9933;
static const int P2P_PORT_TESTNET = 19933;

static const int64 COIN = 100000000;
static const int64 CENT = 1000000;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64 DUST_SOFT_LIMIT = 100000; // 0.001 CAT
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64 DUST_HARD_LIMIT = 1000;   // 0.00001 CAT mininput
/** No amount larger than this (in satoshi) is valid */
static const int64 MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** The interval over which we look to calculate the next difficulty **/
static const int RETARGET_INTERVAL = 36;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;
/** Orphan work threshold, in percent.. should probably be max adjustment for 3-4 blocks **/
// https://bitbucket.org/dahozer/catcoin/issue/14/add-a-threshold-for-orphans-with-slightly
// FIXME go back to 2-3 once we have a working network again
//static const int ORPHAN_WORK_THRESHOLD = 63; // (100/112)^4
extern const int ORPHAN_WORK_THRESHOLD;

#endif
