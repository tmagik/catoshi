// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_codecoin_H
#define CODECOIN_codecoin_H

#ifdef BRAND_solarcoin
#include "solarcoin.h"
#elif BRAND_catcoin
#include "catcoin.h"
#elif BRAND_uro
#include "uro.h"
#elif BRAND_grantcoin
#include "grantcoin.h"
#else
#error "No BRAND_*COIN defined!!"
#endif

#endif
