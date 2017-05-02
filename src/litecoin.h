// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_litecoin_H
#define CODECOIN_litecoin_H

#include<cstdint>

static const int RPC_PORT = 9332;
static const int RPC_PORT_TESTNET = 19332;
static const int P2P_PORT = 9333;
static const int P2P_PORT_TESTNET = 19333;

// TODO: objectize this. as of 0.10.x is in amount.h
static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 84000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

#define BRAND "Litecoin"
#define BRAND_upper "Litecoin"
#define BRAND_lower "litecoin"
#define BRAND_CODE "LTC"
#define LITECOIN_SCRYPT_POWHASH

#endif
