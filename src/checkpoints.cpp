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

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints

        MapCheckpoints::const_iterator i = mapCheckpoints.find(nHeight);
        if (i == mapCheckpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0;

        return mapCheckpoints.rbegin()->first;
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

    // ppcoin: synchronized checkpoint (centrally broadcasted)
    uint256 hashSyncCheckpoint;
    CSyncCheckpoint checkpointMessage;
    CSyncCheckpoint checkpointMessagePending;
    uint256 hashInvalidCheckpoint = 0;
    CCriticalSection cs_hashSyncCheckpoint;

    // ppcoin: get last synchronized checkpoint
    CBlockIndex* GetLastSyncCheckpoint()
    {
        CRITICAL_BLOCK(cs_hashSyncCheckpoint)
        {
            if (!mapBlockIndex.count(hashSyncCheckpoint))
                error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
            else
                return mapBlockIndex[hashSyncCheckpoint];
        }
        return NULL;
    }

    // ppcoin: only descendant of current sync-checkpoint is allowed
    bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
    {
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        if (!mapBlockIndex.count(hashCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());

        CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
        CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

        if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
        {
            // Received an older checkpoint, trace back from current checkpoint
            // to the same height of the received checkpoint to verify
            // that current checkpoint should be a descendant block
            CBlockIndex* pindex = pindexSyncCheckpoint;
            while (pindex->nHeight > pindexCheckpointRecv->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("ValidateSyncCheckpoint: pprev1 null - block index structure failure");
            if (pindex->GetBlockHash() != hashCheckpoint)
            {
                hashInvalidCheckpoint = hashCheckpoint;
                return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
            }
            return false; // ignore older checkpoint
        }

        // Received checkpoint should be a descendant block of the current
        // checkpoint. Trace back to the same height of current checkpoint
        // to verify.
        CBlockIndex* pindex = pindexCheckpointRecv;
        while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
            if (!(pindex = pindex->pprev))
                return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
        if (pindex->GetBlockHash() != hashSyncCheckpoint)
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
        }
        return true;
    }

    bool AcceptPendingSyncCheckpoint()
    {
        CRITICAL_BLOCK(cs_hashSyncCheckpoint)
        {
            if ((!checkpointMessagePending.IsNull()) && mapBlockIndex.count(checkpointMessagePending.hashCheckpoint))
            {
                if (!ValidateSyncCheckpoint(checkpointMessagePending.hashCheckpoint))
                {
                    checkpointMessagePending.SetNull();
                    return false;
                }

                CTxDB txdb;
                CBlockIndex* pindexCheckpoint = mapBlockIndex[checkpointMessagePending.hashCheckpoint];
                if (!pindexCheckpoint->IsInMainChain())
                {
                    txdb.TxnBegin();
                    if (!Reorganize(txdb, pindexCheckpoint))
                    {
                        txdb.TxnAbort();
                        hashInvalidCheckpoint = checkpointMessagePending.hashCheckpoint;
                        return error("ProcessSyncCheckpoint: Reorganize failed for sync checkpoint %s", checkpointMessagePending.hashCheckpoint.ToString().c_str());
                    }
                }

                txdb.TxnBegin();
                if (!txdb.WriteSyncCheckpoint(checkpointMessagePending.hashCheckpoint))
                {
                    txdb.TxnAbort();
                    return error("AcceptPendingSyncCheckpoint() : failed to write to db sync checkpoint %s\n", checkpointMessagePending.hashCheckpoint.ToString().c_str());
                }
                if (!txdb.TxnCommit())
                    return error("AcceptPendingSyncCheckpoint() : failed to commit to db sync checkpoint %s\n", checkpointMessagePending.hashCheckpoint.ToString().c_str());
                txdb.Close();

                hashSyncCheckpoint = checkpointMessagePending.hashCheckpoint;
                checkpointMessage = checkpointMessagePending;
                checkpointMessagePending.SetNull();
                printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());
                // relay the checkpoint
                CRITICAL_BLOCK(cs_hashSyncCheckpoint)
                    BOOST_FOREACH(CNode* pnode, vNodes)
                        checkpointMessage.RelayTo(pnode);
                return true;
            }
        }

        return false;
    }

    uint256 AutoSelectSyncCheckpoint()
    {
        // select block roughly 8 hours ago
        CBlockIndex *pindex = mapBlockIndex[hashSyncCheckpoint];
        while (pindex->pnext && pindex->pnext->GetBlockTime() + AUTO_CHECKPOINT_MIN_SPAN <= GetAdjustedTime())
            pindex = pindex->pnext;
        return pindex->GetBlockHash();
    }

    // Check against synchronized checkpoint
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        int nHeight = pindexPrev->nHeight + 1;

        CRITICAL_BLOCK(cs_hashSyncCheckpoint)
        {
            // sync-checkpoint should always be accepted block
            assert(mapBlockIndex.count(hashSyncCheckpoint));
            const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

            if (nHeight > pindexSync->nHeight)
            {
                // trace back to same height as sync-checkpoint
                const CBlockIndex* pindex = pindexPrev;
                while (pindex->nHeight > pindexSync->nHeight)
                    if (!(pindex = pindex->pprev))
                        return error("CheckSync: pprev null - block index structure failure");
                if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
                    return false; // only descendant of sync-checkpoint can pass check
            }
            if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
                return false; // same height with sync-checkpoint
            if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
                return false; // lower height than sync-checkpoint
        }
        return true;
    }

    bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
    {
        CRITICAL_BLOCK(cs_hashSyncCheckpoint)
        {
            if (checkpointMessagePending.IsNull())
                return false;
            if (hashBlock == checkpointMessagePending.hashCheckpoint)
                return true;
            if (mapOrphanBlocks.count(checkpointMessagePending.hashCheckpoint) 
                && hashBlock == WantedByOrphan(mapOrphanBlocks[checkpointMessagePending.hashCheckpoint]))
                return true;
        }
        return false;
    }
}

// ppcoin: sync-checkpoint master key
const std::string CSyncCheckpoint::strMasterPubKey = "0424f20205e5da98ba632bbd278a11a6499585f62bfb2c782377ef59f0251daab8085fc31471bcb8180bc75ed0fa41bb50c7c084511d54015a3a5241d645c7268a";

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    CKey key;
    if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
        return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
    if (!CheckSignature())
        return false;

    CRITICAL_BLOCK(Checkpoints::cs_hashSyncCheckpoint)
    {
        if (!mapBlockIndex.count(hashCheckpoint))
        {
            // We haven't received the checkpoint chain, keep the checkpoint as pending
            Checkpoints::checkpointMessagePending = *this;
            printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());
            // Ask this guy to fill in what we're missing
            if (pfrom)
            {
                pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
                // ask directly as well in case rejected earlier by duplicate
                // proof-of-stake because getblocks may not get it this time
                pfrom->AskFor(CInv(MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
            }
            return false;
        }

        if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
            return false;

        CTxDB txdb;
        CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];
        if (!pindexCheckpoint->IsInMainChain())
        {
            // checkpoint chain received but not yet main chain
            txdb.TxnBegin();
            if (!Reorganize(txdb, pindexCheckpoint))
            {
                txdb.TxnAbort();
                Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
                return error("ProcessSyncCheckpoint: Reorganize failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
            }
        }

        txdb.TxnBegin();
        if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
        {
            txdb.TxnAbort();
            return error("ProcessSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
        if (!txdb.TxnCommit())
            return error("ProcessSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        txdb.Close();

        Checkpoints::hashSyncCheckpoint = hashCheckpoint;
        Checkpoints::checkpointMessage = *this;
        Checkpoints::checkpointMessagePending.SetNull();
        printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
    }
    return true;
}
