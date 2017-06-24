// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <cinttypes>
#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "arith_uint256.h"

//using namespace std;

/**
 * Main network
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "grantcoin";
        /** 
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xe2;
        pchMessageStart[1] = 0xe7;
        pchMessageStart[2] = 0xe1;
        pchMessageStart[3] = 0xe4;
        nDefaultPort = 9982; /* P2P_PORT */
        //consensus.powLimit = ~uint256S(0) >> 28; // Reduced initial difficulty from Peercoin's 32
        //consensus.powLimit = uint256S("0000000fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	consensus.powLimit = ArithToUint256(~arith_uint256(0) >> 28); // TODO: implement operator= for this
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days FIXME not used for GRT?
        consensus.nPowTargetSpacing = 1.5 * 60; // 1.5 minutes
        consensus.nMinerConfirmationWindow = 3360;
        consensus.nRuleChangeActivationThreshold = 3024; /* 90% of miners */

        nMaxTipAge = 24 * 60 * 60;

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         */
        const char* pszTimestamp = "The Courier-Journal 21-MAR-2015 Prince Charles calls for a revolution";
        CMutableTransaction txNew;
        txNew.nTime = 1427081625;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(9999) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 0; /* SetEmpty() used to do this */
        txNew.vout[0].scriptPubKey.clear(); /* maybe put it back?? */

        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock.SetNull();  // = 0;  // old way is simpler to read
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis); //genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1427086539;
//const CBigNum bnInitialHashTarget(~uint256(0) >> 28);  // Reduced from Peercoin's 40
        genesis.nBits    = 0x1d0fffff;
        genesis.nNonce   = 413974755;

        //cout << genesis.ToString();
        assert(genesis.hashMerkleRoot == uint256S("0xca7e1b14fe8d66d18650db8fa0c1b2787fa48b4a342fff3b00aa1cc9b0ae85f3"));
        assert(genesis.GetHash() == uint256S("0000000f0483c7cc4433d89e321373d82d86ef5ba8157d8f7b9ef3449283421a"));

        vSeeds.push_back(CDNSSeedData("grantcoin.net", "seed1.grantcoin.net"));
        vSeeds.push_back(CDNSSeedData("grantcoin.net", "seed2.grantcoin.net"));

        base58Prefixes[PUBKEY_ADDRESS] = {  38 }; // grantcoin: addresses begin with 'G'
        base58Prefixes[SCRIPT_ADDRESS] = {  97 }; // grantcoin: addresses begin with 'g'
        base58Prefixes[SECRET_KEY] =     { 166 }; // pubkey + 128
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04,0x88,0xB2,0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04,0x88,0xAD,0xE4};

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

#if 0
        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
	    (     0, uint256S("0000000f0483c7cc4433d89e321373d82d86ef5ba8157d8f7b9ef3449283421a"))
//	    (33000, uint256S("0x"))
	    1434870875, 	// * UNIX timestamp of last checkpoint block
	    106400,		// * total number of transactions between genesis and last checkpoint
				//	 (the tx=... number in the SetBestChain debug.log lines)
	    1000.0		// * estimated number of transactions per day after checkpoint
	};
#else
        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 11111, uint256S("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d"))
	};
#endif
    }
};
static CMainParams mainParams;

/**
 * Testnet (Grantcoin v1)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        pchMessageStart[0] = 0xe2;
        pchMessageStart[1] = 0xe7;
        pchMessageStart[2] = 0xe1;
        pchMessageStart[3] = 0xe5;
        nDefaultPort = 9984; /* P2P_PORT_TESTNET */
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 1.5 * 60; // 1.5 minutes

        nMaxTipAge = 0x7fffffff;

        //! Modify the genesis block for the testnet
        const char* pszTimestamp = "Reuters 10-OCT-2015 Hundreds of thousands protest in Berlin against EU-US trade deal";
        CMutableTransaction txNew;
        txNew.nTime = 1444509104;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(9999) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 0; /* SetEmpty() used to do this */
        txNew.vout[0].scriptPubKey.clear(); /* maybe put it back?? */
        genesis.vtx.assign(1, txNew); /* clobber the base class TX */
        genesis.hashPrevBlock.SetNull();
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis); //genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1444510495;
//const CBigNum bnInitialHashTarget(~uint256(0) >> 28);  // Reduced from Peercoin's 40
        genesis.nBits    = 0x1d0fffff;
        genesis.nNonce   = 87045764;

	//cout << genesis.ToString();
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(genesis.hashMerkleRoot == uint256S("650de4987865a27a1c248908c6a93b9d55931ee3df0e97a845c0915bb53a362f"));
        assert(consensus.hashGenesisBlock == uint256S("000000075c9bddc6a4638910415b2995febabf9dd8b634f0832da86c5bab2df5"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("testseed.grantcoin.org", "testseed.grt.7el.us"));

        base58Prefixes[PUBKEY_ADDRESS] = { 65 };  // grantcoin test blockchain: addresses begin with 'T'
        base58Prefixes[SCRIPT_ADDRESS] = { 127 }; // grantcoin test blockchain: addresses begin with 't'
        base58Prefixes[SECRET_KEY]     = { 193 }; // pubkey + 128
        base58Prefixes[EXT_PUBLIC_KEY] = { 0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY] = { 0x04,0x35,0x83,0x94};

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;
    }
};

static CTestNetParams testNetParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

#include "consensus/validation.h"
#include "chain.h"
#include "txdb.h"
#include "utilmoneystr.h"
#include "arith_uint256.h"

using namespace std;
using namespace boost;

// TODO: separate max clock drift from tx timestamp limits?
const unsigned int nMaxClockDrift = 2*60*60;   // this is WAY to big..

const std::string strMessageMagic = "Grantcoin Signed Message:\n";

int64_t GetProofOfWorkRewardTestNet(int nHeight)
{
    int64_t nSubsidy = COIN;
    if (nHeight == 1)
        nSubsidy = 10000000000 * COIN;  // Grantcoin created for planned distribution
    else if (nHeight < 500)
        nSubsidy = CENT;  // De minimus reward pre-launch and up to 2 weeks post-launch
    else if (nHeight < 510)
        nSubsidy = 6.25 * COIN;  // Public mining begins
    else if (nHeight < 520)
        nSubsidy = 12.5 * COIN;  // Reward gradually increases during first few days
    else if (nHeight < 530)
        nSubsidy = 25 * COIN;
    else if (nHeight < 540)
        nSubsidy = 50 * COIN;
    else if (nHeight < 550)
        nSubsidy = 100 * COIN;
    else if (nHeight < 560)
        nSubsidy = 200 * COIN;
    else if (nHeight < 1000)
        nSubsidy = 400 * COIN;  // Maximum reward reached
    else if (nHeight < 1500)
        nSubsidy = 200 * COIN;  // Reward starts to decline
    else if (nHeight < 2000)
        nSubsidy = 100 * COIN;
    else if (nHeight < 2500)
        nSubsidy = 50 * COIN;
    else if (nHeight < 3000)
        nSubsidy = 25 * COIN;
    else if (nHeight < 3500)
        nSubsidy = 12.5 * COIN;
    else if (nHeight < 4000)
        nSubsidy = 6.25 * COIN;
    else if (nHeight < 4500)
        nSubsidy = 3.13 * COIN;
    else if (nHeight < 5000)
        nSubsidy = 1.57 * COIN;
    else if (nHeight >= 5000)
        nSubsidy = COIN;  // PoW reward reaches final minimum value

    // if (fDebug && GetBoolArg("-printcreation"))
    //     printf("GetProofOfWorkReward() : create=%s nBits=0x%08x nSubsidy=%"PRI64d"\n", FormatMoney(nSubsidy).c_str(), nBits, nSubsidy);

    return nSubsidy;
}

int64_t Grantcoin_PoWReward(int nHeight)
{
    int64_t nSubsidy = COIN;
/* kludge, break this because no one much used old testnet above anyway */
//    if (fTestNet){
//	return GetProofOfWorkRewardTestNet(nHeight);
//    }

    if (nHeight == 1)
        nSubsidy = 10000000000 * COIN;  // Grantcoin created for planned distribution
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
    else if (nHeight >= 300000)
        nSubsidy = 10 * COIN;  // Final minimum reward for PoW phase

    // if (fDebug && GetBoolArg("-printcreation"))
    //     printf("GetProofOfWorkReward() : create=%s nBits=0x%08x nSubsidy=%"PRI64d"\n", FormatMoney(nSubsidy).c_str(), nBits, nSubsidy);

    return nSubsidy;
}

// peercoin: miner's coin stake is rewarded based on coin age spent (coin-days)
int64_t Grantcoin_StakeReward(int64_t nCoinAge)
{
    static int64_t nRewardCoinYear = CENT;  // creation amount per coin-year
    int64_t nSubsidy = nCoinAge * 33 / (365 * 33 + 8) * nRewardCoinYear;
    if (fDebug && GetBoolArg("-printcreation", false))
        printf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nCoinAge);
    return nSubsidy;
}

/*
 * Get the allow Seigniorage (money creation, or reward) of the current
 * block. If CoinAge is > 0, this is a proof of stake block.
 */
int64_t CBlockIndex::GetSeigniorage(int64_t nFees, int64_t CoinAge) const
{
	if(IsProofOfWork()){
		return Grantcoin_PoWReward(nHeight);
	} else {
		return Grantcoin_StakeReward(CoinAge);
	}
}

static const int64_t nTargetTimespan = 24 * 60 * 60;  // 24 hours
static const int64_t nTargetSpacingWorkMax = 12 * STAKE_TARGET_SPACING; // 18 minutes

// ppcoin: find last block index up to pindex
// TODO: move up to generic code
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


unsigned int static GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    arith_uint256 powLimit = UintToArith256(Params().GetConsensus().powLimit);
    if (pindexLast == NULL)
        return powLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return powLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return powLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // peercoin: target change every block
    // peercoin: retarget with exponential moving toward target spacing
    arith_uint256 newLimit;
    newLimit.SetCompact(pindexPrev->nBits);
    int64_t nTargetSpacing = fProofOfStake? STAKE_TARGET_SPACING : min(nTargetSpacingWorkMax, (int64_t) STAKE_TARGET_SPACING * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    newLimit *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    newLimit /= ((nInterval + 1) * nTargetSpacing);

    if (newLimit > powLimit)
	return powLimit.GetCompact();

    return newLimit.GetCompact();
}

#if defined(PPCOINSTAKE)
unsigned int GetNextTrustRequired(const CBlockIndex* pindexLast, const CBlock *pblock)
{
	return GetNextTargetRequired(pindexLast, pblock->IsProofOfStake());	
}
#else
unsigned int GetNextTrustRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
	return GetNextTargetRequired(pindexLast, false);	
}
#endif

static int minimum_time_fork = 250000;	// minimum time fork
static int minimum_time_fork_2 = 250000;	// minimum time fork

//Checks for 'hardcoded' block timestamps
bool AcceptBlockTimestamp(CValidationState &state, CBlockIndex* pindexPrev, const CBlockHeader *pblock)
{
	int64_t time_allow = -30;
	int64_t time_warn = MINIMUM_BLOCK_SPACING;
	int64_t delta = pblock->GetBlockTime() - pindexPrev->GetBlockTime();
	int nHeight = pindexPrev->nHeight + 1;

	if (nHeight > minimum_time_fork_2){
		time_allow = 30;
	}
	
	if (delta < time_warn){
		printf("WARNING blocktime nHeight %d time_allow %" PRId64" time_warn %" PRId64" time delta %" PRId64"\n", nHeight, time_allow, time_warn, delta);
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
