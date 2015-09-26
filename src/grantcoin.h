// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_grantcoin_H
#define CODECOIN_grantcoin_H

#include "util.h"

static const int RPC_PORT = 9983;
static const int RPC_PORT_TESTNET = 9985;
static const int P2P_PORT = 9982;
static const int P2P_PORT_TESTNET = 9984;

static const int64_t COIN = 1000000;
static const int64_t CENT = 10000;
/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 50000000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 500;

#warning TODO: sanity check following

static const int STAKE_TARGET_SPACING = 1.5 * 60; // 90-second block spacing 
static const int STAKE_MIN_AGE = 60 * 60 * 24; // minimum age for coin age (24 hours)
static const int STAKE_MAX_AGE = 60 * 60 * 24 * 90; // stake age of full weight
//static const int64 START_BLOCK_PROOF_OF_STAKE = 250000; // PoS allowed starting at this block

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 100000; // 0.001 GRT
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 1000;   // 0.00001 GRT mininput

/** The interval over which we look to calculate the next difficulty **/
// TODO: not needed for GRT???
static const int RETARGET_INTERVAL = 36;

/** Minimum block time spacing (hard limit) **/
static const int64_t MINIMUM_BLOCK_SPACING = 60;	// Absolute minimum spacing

#define BRAND "GrantCoin"
#define BRAND_upper "GrantCoin"
#define BRAND_lower "grantcoin"

#endif
