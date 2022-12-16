// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013-2014 The Catcoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
// Copyright (c) 2015 The Manna Foundation

#include <codecoin.h>
#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <util.h>
#include <inttypes.h>
#include <consensus/validation.h>

int64_t GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams)
{
    int64_t nSubsidy = COIN;

    if (nHeight == 1)
        nSubsidy = 10000000000 * COIN;  // Manna created for planned distribution
    else if (nHeight < 50000)
        nSubsidy = CENT;  // De minimus reward pre-launch and up to 2 weeks post-launch
    else if (nHeight < 51000)
        nSubsidy = 6.25 * COIN;  // Public mining begins
    else if (nHeight < 52000)
        nSubsidy = 12.5 * COIN;  // Reward gradually increases during first few days
    else if (nHeight < 53000)
        nSubsidy = 25 * COIN;
    else if (nHeight < 54000)
        nSubsidy = 50 * COIN;
    else if (nHeight < 55000)
        nSubsidy = 100 * COIN;
    else if (nHeight < 56000)
        nSubsidy = 200 * COIN;
    else if (nHeight < 100000)
        nSubsidy = 400 * COIN;  // Maximum reward reached
    else if (nHeight < 150000)
        nSubsidy = 200 * COIN;  // Reward starts to decline
    else if (nHeight < 200000)
        nSubsidy = 100 * COIN;
    else if (nHeight < 250000)
        nSubsidy = 50 * COIN;
    else if (nHeight < 300000)
        nSubsidy = 25 * COIN;
    else if (nHeight < 1100000)  // approx June 2018
        nSubsidy = 10 * COIN;
    else if (nHeight < 1200000)  // approx Oct 2018
        nSubsidy = 5 * COIN;
    else if (nHeight < 1300000)  // approx Jan 2019
        nSubsidy = 2.5 * COIN;
    else if (nHeight >= 1300000) //
        nSubsidy = 1 * COIN;     // Final minimum 90 sec reward for PoW


    // if (fDebug && GetBoolArg("-printcreation"))
    //     LogPrintf("GetProofOfWorkReward() : create=%s nBits=0x%08x nSubsidy=%"PRI64d"\n", FormatMoney(nSubsidy).c_str(), nBits, nSubsidy);

    return nSubsidy;
}

#if defined(PPCOINSTAKE)
// peercoin: miner's coin stake is rewarded based on coin age spent (coin-days)
int64_t PPCoin_StakeReward(int64_t nCoinAge)
{
    static int64_t nRewardCoinYear = CENT;  // creation amount per coin-year
    int64_t nSubsidy = nCoinAge * 33 / (365 * 33 + 8) * nRewardCoinYear;
    if (gArgs.GetBoolArg("-printcreation", false))
        LogPrintf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nCoinAge);
    return nSubsidy;
}

/*
 * Get the allow Seigniorage (money creation, or reward) of the current
 * block. If CoinAge is > 0, this is a proof of stake block.
 */
int64_t GetSeigniorage(const CBlockIndex *block, int64_t nFees, int64_t CoinAge)
{
	if(CoinAge == 0){
		return GetProofOfWorkReward(block->nHeight);
	} else {
                assert(0);
		//return GetProofOfStakeReward(CoinAge);
	}
}
#endif

//replaced: consensus.nTargetTimespan // static const int64_t nTargetTimespan = 24 * 60 * 60;  // 24 hours
static const int64_t nTargetSpacingWorkMax = 12 * STAKE_TARGET_SPACING; // 18 minutes

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime, const CBlockHeader* pblock, const Consensus::Params& params)
{
    arith_uint256 bnResult;
    bnResult.SetCompact(nBase);
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    bnResult *= 2;
    while (nTime > 0 && bnResult < bnPowLimit)
    {
        // Maximum 200% adjustment per 3 hours
        bnResult *= 2;
        nTime -= 3 * 60 * 60;
    }
    if (bnResult > bnPowLimit)
        bnResult = bnPowLimit;
    return bnResult.GetCompact();
}

// peercoin: find last block index up to pindex
// TODO: move up to generic function
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
#if !defined(PPCOINSTAKE)
    /* better to assert than look for things that don't exist? */
    assert(fProofOfStake == false);
    // TODO: this could probably be optimized better to just return pindex-pprev,
    // but genesis block case has to be dealt with.
#endif 
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

unsigned int static GetNextTargetRequired(const CBlockIndex* pindexLast, const Consensus::Params & params, bool fProofOfStake)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    if (pindexLast == NULL)
        return bnPowLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnPowLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnPowLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // peercoin: target change every block  // TODO: audit ppcoin version
    // peercoin: retarget with exponential moving toward target spacing
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nTargetSpacing = fProofOfStake? STAKE_TARGET_SPACING : std::min(nTargetSpacingWorkMax, (int64_t) STAKE_TARGET_SPACING * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = params.nPowTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params & params)
{
	return GetNextTargetRequired(pindexLast, params, false);	
}

static int minimum_time_fork = 250000;	// minimum time fork
static int minimum_time_fork_2 = 250000;	// minimum time fork

//Checks for 'hardcoded' block timestamps
bool AcceptBlockTimestamp(const CBlockHeader* pblock, CValidationState &state, const CBlockIndex* pindexPrev)
{
	int64_t time_allow = -30;
	int64_t time_warn = MINIMUM_BLOCK_SPACING;
	int64_t delta = pblock->GetBlockTime() - pindexPrev->GetBlockTime();
	int nHeight = pindexPrev->nHeight + 1;

	if (nHeight > minimum_time_fork_2){
		time_allow = 30;
	}
	
	if (delta < time_warn){
		LogPrintf("WARNING blocktime nHeight %d time_allow %" PRId64" time_warn %" PRId64" time delta %" PRId64"\n", nHeight, time_allow, time_warn, delta);
	}

	if (nHeight >= minimum_time_fork_2) {
		if (delta <= time_allow) // see above, from first hard limit
			return state.Invalid(error("AcceptBlock(height=%d) : block time delta %" PRId64" too short", nHeight, delta));
	}
	if (nHeight >= minimum_time_fork) { /* don't forward these */
		if (delta <= MINIMUM_BLOCK_SPACING)
			return state.DoS(10, (error("AcceptBlock(height=%d) : block time delta %" PRId64" too short", nHeight, delta)));
	}
	return true;	
}
