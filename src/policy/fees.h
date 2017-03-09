// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_POLICYESTIMATOR_H
#define BITCOIN_POLICYESTIMATOR_H

#include "amount.h"
#include "uint256.h"
#include "random.h"
#include "sync.h"

#include <map>
#include <string>
#include <vector>

class CAutoFile;
class CFeeRate;
class CTxMemPoolEntry;
class CTxMemPool;
class TxConfirmStats;

/** \class CBlockPolicyEstimator
 * The BlockPolicyEstimator is used for estimating the feerate needed
 * for a transaction to be included in a block within a certain number of
 * blocks.
 *
 * At a high level the algorithm works by grouping transactions into buckets
 * based on having similar feerates and then tracking how long it
 * takes transactions in the various buckets to be mined.  It operates under
 * the assumption that in general transactions of higher feerate will be
 * included in blocks before transactions of lower feerate.   So for
 * example if you wanted to know what feerate you should put on a transaction to
 * be included in a block within the next 5 blocks, you would start by looking
 * at the bucket with the highest feerate transactions and verifying that a
 * sufficiently high percentage of them were confirmed within 5 blocks and
 * then you would look at the next highest feerate bucket, and so on, stopping at
 * the last bucket to pass the test.   The average feerate of transactions in this
 * bucket will give you an indication of the lowest feerate you can put on a
 * transaction and still have a sufficiently high chance of being confirmed
 * within your desired 5 blocks.
 *
 * Here is a brief description of the implementation:
 * When a transaction enters the mempool, we
 * track the height of the block chain at entry.  Whenever a block comes in,
 * we count the number of transactions in each bucket and the total amount of feerate
 * paid in each bucket. Then we calculate how many blocks Y it took each
 * transaction to be mined and we track an array of counters in each bucket
 * for how long it to took transactions to get confirmed from 1 to a max of 25
 * and we increment all the counters from Y up to 25. This is because for any
 * number Z>=Y the transaction was successfully mined within Z blocks.  We
 * want to save a history of this information, so at any time we have a
 * counter of the total number of transactions that happened in a given feerate
 * bucket and the total number that were confirmed in each number 1-25 blocks
 * or less for any bucket.   We save this history by keeping an exponentially
 * decaying moving average of each one of these stats.  Furthermore we also
 * keep track of the number unmined (in mempool) transactions in each bucket
 * and for how many blocks they have been outstanding and use that to increase
 * the number of transactions we've seen in that feerate bucket when calculating
 * an estimate for any number of confirmations below the number of blocks
 * they've been outstanding.
 */

enum FeeEstimateHorizon {
    SHORT_HALFLIFE = 0,
    MED_HALFLIFE = 1,
    LONG_HALFLIFE = 2
};

struct EstimatorBucket
{
    double start = -1;
    double end = -1;
    double withinTarget = 0;
    double totalConfirmed = 0;
    double inMempool = 0;
    double leftMempool = 0;
};

struct EstimationResult
{
    EstimatorBucket pass;
    EstimatorBucket fail;
    double decay;
};

/**
 *  We want to be able to estimate feerates that are needed on tx's to be included in
 * a certain number of blocks.  Every time a block is added to the best chain, this class records
 * stats on the transactions included in that block
 */
class CBlockPolicyEstimator
{
private:
    /** Track confirm delays up to 12 blocks medium decay */
    static constexpr unsigned int SHORT_BLOCK_CONFIRMS = 12;
    /** Track confirm delays up to 48 blocks medium decay */
    static constexpr unsigned int MED_BLOCK_CONFIRMS = 48;
    /** Track confirm delays up to 1008 blocks for longer decay */
    static constexpr unsigned int LONG_BLOCK_CONFIRMS = 1008;

    /** Decay of .962 is a half-life of 18 blocks or about 3 hours */
    static constexpr double SHORT_DECAY = .962;
    /** Decay of .998 is a half-life of 144 blocks or about 1 day */
    static constexpr double MED_DECAY = .9952;
    /** Decay of .9995 is a half-life of 1008 blocks or about 1 week */
    static constexpr double LONG_DECAY = .99931;

    /** Require greater than 95% of X feerate transactions to be confirmed within Y blocks for X to be big enough */
    static constexpr double HALF_SUCCESS_PCT = .6;
    static constexpr double SUCCESS_PCT = .85;
    static constexpr double DOUBLE_SUCCESS_PCT = .95;

    /** Require an avg of 0.1 tx in the combined feerate bucket per block to have stat significance */
    static constexpr double SUFFICIENT_FEETXS = 0.1;
    /** Require an avg of 0.5 tx when using short decay since there are fewer blocks considered*/
    static constexpr double SUFFICIENT_TXS_SHORT = 0.5;

    /** Minimum and Maximum values for tracking feerates
     * The MIN_BUCKET_FEERATE should just be set to the lowest reasonable feerate we
     * might ever want to track.  Historically this has been 1000 since it was
     * inheriting DEFAULT_MIN_RELAY_TX_FEE and changing it is disruptive as it
     * invalidates old estimates files. So leave it at 1000 unless it becomes
     * necessary to lower it, and then lower it substantially.
     */
    static constexpr double MIN_BUCKET_FEERATE = 1000;
    static constexpr double MAX_BUCKET_FEERATE = 1e7;

    /** Spacing of FeeRate buckets
     * We have to lump transactions into buckets based on feerate, but we want to be able
     * to give accurate estimates over a large range of potential feerates
     * Therefore it makes sense to exponentially space the buckets
     */
    static constexpr double FEE_SPACING = 1.05;

public:
    /** Create new BlockPolicyEstimator and initialize stats tracking classes with default values */
    CBlockPolicyEstimator();
    ~CBlockPolicyEstimator();

    /** Process all the transactions that have been included in a block */
    void processBlock(unsigned int nBlockHeight,
                      std::vector<const CTxMemPoolEntry*>& entries);

    /** Process a transaction accepted to the mempool*/
    void processTransaction(const CTxMemPoolEntry& entry, bool validFeeEstimate);

    /** Remove a transaction from the mempool tracking stats*/
    bool removeTx(uint256 hash, bool inBlock);

    /** Return a feerate estimate */
    CFeeRate estimateFee(int confTarget) const;

    /** Estimate feerate needed to get be included in a block within
     *  confTarget blocks. If no answer can be given at confTarget, return an
     *  estimate at the lowest target where one can be given.
     */
    CFeeRate estimateSmartFee(int confTarget, int *answerFoundAtTarget, const CTxMemPool& pool) const;

    /** Return a specific fee estimate calculation with a given success threshold and time horizon.
     */
    CFeeRate estimateRawFee(int confTarget, double successThreshold, FeeEstimateHorizon horizon, EstimationResult *result = nullptr) const;

    /** Write estimation data to a file */
    bool Write(CAutoFile& fileout) const;

    /** Read estimation data from a file */
    bool Read(CAutoFile& filein);

    /** Empty mempool transactions on shutdown to record failure to confirm for txs still in mempool */
    void FlushUnconfirmed(CTxMemPool& pool);

private:
    CFeeRate minTrackedFee;    //!< Passed to constructor to avoid dependency on main
    unsigned int nBestSeenHeight;
    struct TxStatsInfo
    {
        unsigned int blockHeight;
        unsigned int bucketIndex;
        TxStatsInfo() : blockHeight(0), bucketIndex(0) {}
    };

    // map of txids to information about that transaction
    std::map<uint256, TxStatsInfo> mapMemPoolTxs;

    /** Classes to track historical data on transaction confirmations */
    TxConfirmStats* feeStats;
    TxConfirmStats* shortStats;
    TxConfirmStats* longStats;

    unsigned int trackedTxs;
    unsigned int untrackedTxs;

    std::vector<double> buckets;              // The upper-bound of the range for the bucket (inclusive)
    std::map<double, unsigned int> bucketMap; // Map of bucket upper-bound to index into all vectors by bucket

    mutable CCriticalSection cs_feeEstimator;

    /** Process a transaction confirmed in a block*/
    bool processBlockTx(unsigned int nBlockHeight, const CTxMemPoolEntry* entry);

};

class FeeFilterRounder
{
private:
    static constexpr double MAX_FILTER_FEERATE = 1e7;
    /** FEE_FILTER_SPACING is just used to provide some quantization of fee
     * filter results.  Historically it reused FEE_SPACING, but it is completely
     * unrelated, and was made a separate constant so the two concepts are not
     * tied together */
    static constexpr double FEE_FILTER_SPACING = 1.1;

public:
    /** Create new FeeFilterRounder */
    FeeFilterRounder(const CFeeRate& minIncrementalFee);

    /** Quantize a minimum fee for privacy purpose before broadcast **/
    CAmount round(CAmount currentMinFee);

private:
    std::set<double> feeset;
    FastRandomContext insecure_rand;
};

#endif /*BITCOIN_POLICYESTIMATOR_H */
