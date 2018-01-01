// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2017 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_AMOUNT_H
#define COCECOIN_AMOUNT_H

#include <stdint.h>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;
#include "codecoin.h"

/* TODO make a static part of chain or params object */
extern const std::string CURRENCY_UNIT;

#endif //  CODECOIN_AMOUNT_H
