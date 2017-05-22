// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html


#include "chainparams.h"

#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <inttypes.h>
#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

/**
 * Main network
 */

/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
	boost::assign::map_list_of
	(     0, uint256("0000000f0483c7cc4433d89e321373d82d86ef5ba8157d8f7b9ef3449283421a"))
//	(33000, uint256("0x"))
	;
const Checkpoints::CCheckpointData data = {
	&mapCheckpoints,
	1434870875, 	// * UNIX timestamp of last checkpoint block
	106400,		// * total number of transactions between genesis and last checkpoint
				//	 (the tx=... number in the SetBestChain debug.log lines)
	1000.0		// * estimated number of transactions per day after checkpoint
};


class CMainParams : public CChainParams {
public:
    CMainParams() {
        networkID = CBaseChainParams::MAIN;
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
        nDefaultPort = 9982;
        bnProofOfWorkLimit = ~uint256(0) >> 28; // Reduced initial difficulty from Peercoin's 32
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days FIXME not used for GRT?
        nTargetSpacing = 1.5 * 60; // 1.5 minutes
        nMaxTipAge = 24 * 60 * 60;

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         */
        const char* pszTimestamp = "The Courier-Journal 21-MAR-2015 Prince Charles calls for a revolution";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(9999) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].SetNull(); // Technically the constructor should do this already
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1427086539;
//const CBigNum bnInitialHashTarget(~uint256(0) >> 28);  // Reduced from Peercoin's 40
        genesis.nBits    = 0x1d0fffff;
        genesis.nNonce   = 413974755;

        hashGenesisBlock = genesis.GetHash();
	cout << genesis.ToString();
        assert(hashGenesisBlock == uint256("0000000f0483c7cc4433d89e321373d82d86ef5ba8157d8f7b9ef3449283421a"));
        assert(genesis.hashMerkleRoot == uint256("0xca7e1b14fe8d66d18650db8fa0c1b2787fa48b4a342fff3b00aa1cc9b0ae85f3"));

        vSeeds.push_back(CDNSSeedData("grantcoin.net", "seed1.grantcoin.net"));
        vSeeds.push_back(CDNSSeedData("grantcoin.net", "seed2.grantcoin.net"));

        base58Prefixes[PUBKEY_ADDRESS] = {  38 }; // grantcoin: addresses begin with 'G'
        base58Prefixes[SCRIPT_ADDRESS] = {  97 }; // grantcoin: addresses begin with 'g'
        base58Prefixes[SECRET_KEY] =     { 166 }; // pubkey + 128
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04,0x88,0xB2,0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04,0x88,0xAD,0xE4};

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        // Litecoin: Mainnet v2 enforced as of block 710k
        nEnforceV2AfterHeight = 710000;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (Grantcoin v1)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 9984;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        nTargetSpacing = 1.5 * 60; // 1.5 minutes
        nMaxTipAge = 0x7fffffff;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1317798646;
        genesis.nNonce = 385270584;
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x000000075c9bddc6a4638910415b2995febabf9dd8b634f0832da86c5bab2df5"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("seed.grantcoin.orb", "seed1.grantcoin.org"));

        base58Prefixes[PUBKEY_ADDRESS] = { 65 };  // grantcoin test blockchain: addresses begin with 'T'
        base58Prefixes[SCRIPT_ADDRESS] = { 127 }; // grantcoin test blockchain: addresses begin with 't'
        base58Prefixes[SECRET_KEY]     = { 193 }; // pubkey + 128
        base58Prefixes[EXT_PUBLIC_KEY] = { 0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY] = { 0x04,0x35,0x83,0x94};

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        // Litecoin: Testnet v2 enforced as of block 400k
        nEnforceV2AfterHeight = 400000;
    }
//    const Checkpoints::CCheckpointData& Checkpoints() const 
//    {
//        return dataTestnet;
//    }
};

static CTestNetParams testNetParams;

static CChainParams *pCurrentParams = 0;

CModifiableParams *ModifiableParams()
{
   assert(pCurrentParams);
   assert(!"no unitTest options for Grantcoin yet");
   return NULL;
}

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

#include "txdb.h"
#include "utilmoneystr.h"

using namespace std;
using namespace boost;

uint256 hashGenesisBlock = 0;			// TODO: objectize this for multicoin support

const string strMessageMagic = "Grantcoin Signed Message:\n";

int nCoinbaseMaturity = COINBASE_MATURITY;

// TODO: separate max clock drift from tx timestamp limits?
const unsigned int nMaxClockDrift = 2*60*60;   // this is WAY to big..



/* value, in percent of what difficulty value we'll accept for orphans */
const int ORPHAN_WORK_THRESHOLD = 1; // FIXME WAY TOO WIDE right now

// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
// FIXME use a single string and/or objectify this
const char *strMainNetDNSSeed[][2] = {
	{"grantcoin.net", "seed1.grantcoin.net"},
	{"grantcoin.net", "seed2.grantcoin.net"},
	{NULL, NULL}
};

const char *strTestNetDNSSeed[][2] = {
	{"seed", "159.203.84.95"},
	{NULL, NULL}
};

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
    if (pindexLast == NULL)
        return Params().ProofOfWorkLimit().GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return Params().ProofOfWorkLimit().GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return Params().ProofOfWorkLimit().GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // peercoin: target change every block
    // peercoin: retarget with exponential moving toward target spacing
    uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nTargetSpacing = fProofOfStake? STAKE_TARGET_SPACING : min(nTargetSpacingWorkMax, (int64_t) STAKE_TARGET_SPACING * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > Params().ProofOfWorkLimit())
        bnNew = Params().ProofOfWorkLimit();

    return bnNew.GetCompact();
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


namespace Checkpoints
{
}
