// Copyright (c) 2011 The Bitcoin developers
// Copyright (c) 2011-2012 The PPCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "headers.h"
#include "checkpoints.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;   // hardened checkpoints

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, hashGenesisBlock )
        ; // ppcoin: no checkpoint yet; to be created in future releases

    // ppcoin: automatic checkpoint (represented by height of checkpoint)
    int nAutoCheckpoint = 0;
    int nBranchPoint = 0;    // branch point to alternative branch

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints

        MapCheckpoints::const_iterator i = mapCheckpoints.find(nHeight);
        if (i == mapCheckpoints.end()) return true;
        return hash == i->second;
    }

    // ppcoin: check automatic checkpoint
    // To pass the check:
    //   - All ancestors (including the block itself) have block index already
    //   - The immediate ancestor in main chain must not have height less than
    //     checkpoint height
    bool CheckAuto(const CBlockIndex *pindex)
    {
        while (pindex)
        {
            if (pindex->IsInMainChain())
            {
                if (pindex->nHeight >= nAutoCheckpoint)
                    return true;
                else
                {
                    nBranchPoint = pindex->nHeight;
                    return error("Checkpoints: new block on alternative branch at height=%d before auto checkpoint at height=%d", pindex->nHeight, nAutoCheckpoint);
                }
            }
            else
                pindex = pindex->pprev;
        }
        return error("Checkpoints: failed to find any ancestor on main chain for the new block - internal error");
    }

    // ppcoin: get next chain checkpoint
    int GetNextChainCheckpoint(const CBlockIndex *pindexLast)
    {
        CBigNum bnTarget;
        CBigNum bnTargetMax = 0;  // max target of all blocks since checkpoint
        CBigNum bnTargetMin = 0;  // min target of all candidate checkpoints
        int nMinTargetHeight = 0; // min target height of candidate checkpoints
        int nCheckpointMin = 0;   // minimum candidate checkpoint
        int nCheckpointMax = 0;   // maximum candidate checkpoint
        int nDepth = pindexLast->nHeight - pindexLast->nCheckpoint;
        const CBlockIndex *pindex = pindexLast;
        while (nDepth >= 0 && pindex)
        {
            bnTarget.SetCompact(pindex->nBits);
            if (bnTarget > bnTargetMax)
                bnTargetMax = bnTarget;
            if (nCheckpointMax > 0 && bnTarget < bnTargetMin)
            {
                bnTargetMin = bnTarget;
                nMinTargetHeight = pindex->nHeight;
            }
            if (nCheckpointMax == 0 && pindexLast->GetBlockTime() - pindex->GetBlockTime() > AUTO_CHECKPOINT_MIN_SPAN)
            {
                nCheckpointMax = pindex->nHeight;
                bnTargetMin.SetCompact(pindex->nBits);
                nMinTargetHeight = pindex->nHeight;
            }
            if (pindexLast->GetBlockTime() - pindex->GetBlockTime() < AUTO_CHECKPOINT_MAX_SPAN)
                nCheckpointMin = pindex->nHeight;
            pindex = pindex->pprev;
            nDepth--;
        }

        assert (nDepth == -1);  // arrive at chain checkpoint now

        printf("Checkpoints: min=%d max=%d tminheight=%d tmin=0x%08x tmax=0x%08x\n",
            nCheckpointMin, nCheckpointMax, nMinTargetHeight,
            bnTargetMin.GetCompact(), bnTargetMax.GetCompact());
        if (nCheckpointMax == 0) // checkpoint stays if max candidate not found
            return pindexLast->nCheckpoint;

        if (bnTargetMin * 100 > bnTargetMax * 90)
            return nCheckpointMax;
        if (bnTarget * 100 > bnTargetMax * 90)
            return nMinTargetHeight;
        else
            return nCheckpointMin;
    }

    // ppcoin: get next auto checkpoint from the new chain checkpoint
    int GetNextAutoCheckpoint(int nCheckpoint)
    {
        return (std::max(nAutoCheckpoint, nCheckpoint));
    }

    // ppcoin: advance to next automatic checkpoint
    void AdvanceAutoCheckpoint(int nCheckpoint)
    {
        nAutoCheckpoint = GetNextAutoCheckpoint(nCheckpoint);
        printf("Checkpoints: auto checkpoint now at height=%d\n", nAutoCheckpoint);
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0;

        return mapCheckpoints.rbegin()->first;
    }

    // ppcoin: reset auto checkpoint
    bool ResetAutoCheckpoint(int nCheckpoint)
    {
        if (nCheckpoint <= 0 || nCheckpoint > nBestHeight)
            return error("ResetAutoCheckpoint() : new checkpoint invalid");
        if (nCheckpoint >= nAutoCheckpoint)
            return error("ResetAutoCheckpoint() : new checkpoint not earlier than current auto checkpoint");
        CTxDB txdb;
        txdb.TxnBegin();
        if (!txdb.WriteAutoCheckpoint(nCheckpoint, true))
            return error("ResetAutoCheckpoint() : database write failed");
        if (!txdb.TxnCommit())
            return error("ResetAutoCheckpoint() : database commit failed");
        nAutoCheckpoint = nCheckpoint;
        nBranchPoint = 0;  // clear branch point

        // clear ban list to accept alternative branches
        CRITICAL_BLOCK(cs_vNodes)
        {
            BOOST_FOREACH(CNode* pnode, vNodes)
                pnode->ClearBanned();
        }

        return true;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL;

        int64 nResult;
        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
