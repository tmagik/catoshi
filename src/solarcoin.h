// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_catcoin_H
#define CODECOIN_catcoin_H

#include "util.h"

static const int RPC_PORT = 18181;
static const int RPC_PORT_TESTNET = 28181;
static const int P2P_PORT = 18188;
static const int P2P_PORT_TESTNET = 28188;

static const uint64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 100000; // 0.001 CAT
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 1000;   // 0.00001 CAT mininput
/** No amount larger than this (in satoshi) is valid */
static const uint64_t MAX_MONEY = 100000000000 * COIN;
//inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
inline bool MoneyRange(uint64_t nValue) { return (nValue <= MAX_MONEY); }

/** The interval over which we look to calculate the next difficulty **/
static const int RETARGET_INTERVAL = 36;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

#define BRAND "SolarCoin"
#define BRAND_upper "SolarCoin"
#define BRAND_lower "solarcoin"

#endif
