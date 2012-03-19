// Copyright (c) 2011 The Bitcoin developers
// Copyright (c) 2011-2012 The PPCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_CHECKPOINT_H
#define  BITCOIN_CHECKPOINT_H

#include <map>
#include "util.h"

// ppcoin: auto checkpoint min at 1 day; max at 5 days
#define AUTO_CHECKPOINT_MIN_SPAN   (60 * 60 * 24)
#define AUTO_CHECKPOINT_MAX_SPAN   (60 * 60 * 24 * 5)
#define AUTO_CHECKPOINT_TRUST_SPAN (60 * 60 * 24 * 7)

class uint256;
class CBlockIndex;

//
// Block-chain checkpoints are compiled-in sanity checks.
// They are updated every release or three.
//
namespace Checkpoints
{
    extern int nAutoCheckpoint;
    extern int nBranchPoint;

    // Returns true if block passes checkpoint checks
    bool CheckHardened(int nHeight, const uint256& hash);
    bool CheckAuto(const CBlockIndex *pindex);

    int  GetNextChainCheckpoint(const CBlockIndex *pindex);
    int  GetNextAutoCheckpoint(int nCheckpoint);
    void AdvanceAutoCheckpoint(int nCheckpoint);
    bool ResetAutoCheckpoint(int nCheckpoint);

    // Return conservative estimate of total number of blocks, 0 if unknown
    int GetTotalBlocksEstimate();

    // Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);
}

#endif
