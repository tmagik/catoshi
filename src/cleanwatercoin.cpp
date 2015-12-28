// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013-2014 The Catcoin developers
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "codecoin.h"
#include "codecoinrpc.h"   // GetDifficulty
#include "cleanwatercoin.h"
#include "kernel.h"
#include "checkpoints.h"
#include "db.h"
#include "txdb.h"
#include "net.h"
#include "init.h"
#include "ui_interface.h"
#include "checkqueue.h"
#include "scrypt.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;

/* TODO: make this part of a decent C++ object with proper constructors */
uint256 hashGenesisBlock = 0;		
const string strMessageMagic = "CleanWaterCoin Signed Message:\n";
unsigned char pchMessageStart[4];

const CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
const CBigNum bnProofOfStakeLimit(~uint256(0) >> 20);

unsigned int nStakeMinAge = 60 * 60 * 24 * 1;	// After height 120,000 minimum age for coin age: 1d
unsigned int nStakeMinAgeOld = 60 * 60 * 24 * 20;	// Before height 120,000 minimum age for coin age: 20d
unsigned int nStakeMaxAge = 60 * 60 * 24 * 90;	// stake age of full weight: 90d
unsigned int nStakeTargetSpacing = 120;			// 120 sec block spacing
const unsigned int nMaxClockDrift = 90 * 60; 		// 90 minutes

// fixme make unsigned?
int nCoinbaseMaturity = 30;
const int nCutoff_Pos_Block = 14628; /* first PoS block */

/** TODO: this goes into src/policy/fees.cpp when latest bitcoin code is merged */
/** Fees smaller than this (in satoshi) are considered zero fee (for transaction creation) */
int64_t CTransaction::nMinTxFee = CENT;
/** Fees smaller than this (in satoshi) are considered zero fee (for relaying) */
int64_t CTransaction::nMinRelayTxFee = CENT;

// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
// FIXME use a single string and/or objectify this
/*
  CleanWaterCoin policy for getting on this list:
  TODO: come up with a policy
 */
const char *strMainNetDNSSeed[][2] = 
{
   {"intercom", "xtc.inter.com"},
   {NULL, NULL}
};

const char *strTestNetDNSSeed[][2] = {
	//{"CleanWaterCointools.com", "testnet-seed.CleanWaterCointools.com"},
	{NULL, NULL}
};

// miner's coin base reward based on nBits
int64_t CleanWaterCoinWorkReward(int nHeight, int64_t nFees)
{
    int64_t nSubsidy = 150 * COIN;
    double coinDifficulty = (double)GetDifficulty();

    int64_t rewardCalc = 1/(sqrt(coinDifficulty + 500));
    double buttCalc =  1/(sqrt(coinDifficulty + 500));

 if (nHeight == 1)
    {
        nSubsidy = 10000000 * COIN;       // first block is premine
    }
    else if (nHeight >1 && nHeight <= 55)  // 55 blocks for confirmation of premine
        {
            nSubsidy = 1 * COIN;
        }
    else if (nHeight >55 && nHeight <= 7000)
        {
            nSubsidy = 1000 * COIN;
        }
    else if (nHeight >7000 && nHeight < 8000)
        {
            nSubsidy = (int64_t)((double)(20000 * sqrt(coinDifficulty + 500)) * COIN);
        }

 // fork here for proper block reward - 8000 per cleanwatercoin guys

    else if (nHeight >= 8000 && nHeight < 8500)
     {
             nSubsidy = (int64_t)((double)(20000 * rewardCalc) * COIN);
     }

    else if (nHeight >= 8500 && nHeight <= 250000)
 {
         nSubsidy = (int64_t)((double)(20000 * buttCalc) * COIN);
 }


    else if (nHeight > 250000 && nHeight <= 500000)
        {
            nSubsidy = (int64_t)((double)(15000 * buttCalc) * COIN);
        }
    else if (nHeight >500000)
        {
            nSubsidy = (int64_t)((double)(10000 * buttCalc) * COIN);
        }
//    else if (nHeight > CUTOFF_HEIGHT)
//        {
//            nSubsidy = 0;
//        }


    return nSubsidy + nFees;
}

const int YEARLY_BLOCKCOUNT = 262800;	// 365 * 720
int64_t CleanWaterCoinStakeReward(int64_t nCoinAge, int nHeight)
{
    int64_t nRewardCoinYear;

    nRewardCoinYear = MAX_MINT_PROOF_OF_STAKE;

    if(nHeight >= 7000 && nHeight < 500000)
        nRewardCoinYear = 0.08 * COIN;
    else if(nHeight >= 500000 && nHeight < 1000000)
        nRewardCoinYear = 0.07 * COIN;
    else if(nHeight >= 1000000 && nHeight < 2000000)
        nRewardCoinYear = 0.06 * COIN;
    else if(nHeight >= 2000000)
        nRewardCoinYear = 0.02 * COIN;

    int64_t nSubsidy = nCoinAge * nRewardCoinYear / 365;

	if (fDebug && GetBoolArg("-printcreation"))
        printf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nCoinAge);
    return nSubsidy;
}

static const int64_t nTargetTimespan = 1 * 24 * 60 * 30;
static const int64_t nTargetSpacingWorkMax = 3 * nStakeTargetSpacing; 

/*
 * Get the allow Seigniorage (money creation, or reward) of the current
 * block. If CoinAge is > 0, this is a proof of stake block.
 */
int64_t CBlockIndex::GetSeigniorage(int64_t nFees, int64_t CoinAge) const
{
	if(IsProofOfWork()){
		return CleanWaterCoinWorkReward(nHeight, nFees);
	} else {
		return CleanWaterCoinStakeReward(CoinAge, nHeight);
	}
}

//
// maximum nBits value could possible be required nTime after
// minimum proof-of-work required was nBase
//
unsigned int ComputeMaxBits(CBigNum bnTargetLimit, unsigned int nBase, int64_t nTime)
{
    CBigNum bnResult;
    bnResult.SetCompact(nBase);
    bnResult *= 2;
    while (nTime > 0 && bnResult < bnTargetLimit)
    {
        // Maximum 200% adjustment per day...
        bnResult *= 2;
        nTime -= 24 * 60 * 60;
    }
    if (bnResult > bnTargetLimit)
        bnResult = bnTargetLimit;
    return bnResult.GetCompact();
}

//
// minimum amount of work that could possibly be required nTime after
// minimum proof-of-work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime, const CBlockHeader* pblock)
{
    return ComputeMaxBits(bnProofOfWorkLimit, nBase, nTime);
}

//
// minimum amount of stake that could possibly be required nTime after
// minimum proof-of-stake required was nBase
//
unsigned int ComputeMinStake(unsigned int nBase, int64_t nTime, unsigned int nBlockTime)
{
    return ComputeMaxBits(bnProofOfStakeLimit, nBase, nTime);
}

bool AcceptBlockTimestamp(CValidationState &state, CBlockIndex* pindexPrev, const CBlockHeader *pblock)
{
	return true;	
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake) 
{
    CBigNum bnTargetLimit = bnProofOfWorkLimit;

    if(fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }
bnTargetLimit = bnProofOfStakeLimit;
    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	if(nActualSpacing < 0)
	{
		// printf(">> nActualSpacing = %"PRI64d" corrected to 1.\n", nActualSpacing);
		nActualSpacing = 1;
	}
	else if(nActualSpacing > nTargetTimespan)
	{
		// printf(">> nActualSpacing = %"PRI64d" corrected to nTargetTimespan (900).\n", nActualSpacing);
		nActualSpacing = nTargetTimespan;
	}

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    int64_t nTargetSpacing = fProofOfStake? nStakeTargetSpacing : min(nTargetSpacingWorkMax, (int64_t) nStakeTargetSpacing * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);
	
	/*
	printf(">> Height = %d, fProofOfStake = %d, nInterval = %"PRI64d", nTargetSpacing = %"PRI64d", nActualSpacing = %"PRI64d"\n", 
		pindexPrev->nHeight, fProofOfStake, nInterval, nTargetSpacing, nActualSpacing);  
	printf(">> pindexPrev->GetBlockTime() = %"PRI64d", pindexPrev->nHeight = %d, pindexPrevPrev->GetBlockTime() = %"PRI64d", pindexPrevPrev->nHeight = %d\n", 
		pindexPrev->GetBlockTime(), pindexPrev->nHeight, pindexPrevPrev->GetBlockTime(), pindexPrevPrev->nHeight);  
	*/

    if (bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextTrustRequired(const CBlockIndex* pindexLast, const CBlock *pblock)
{
	return GetNextTargetRequired(pindexLast, pblock->IsProofOfStake());	
}


bool LoadBlockIndex()
{
    if (fTestNet)
    {
        pchMessageStart[0] = 0xcd;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xef;

        nStakeMinAge = 20 * 60; // test net min age is 20 min
        nStakeMaxAge = 60 * 60; // test net min age is 60 min
        nModifierInterval = 60; // test modifier interval is 2 minutes
        nCoinbaseMaturity = 10; // test maturity is 10 blocks
        nStakeTargetSpacing = 3 * 60; // test block spacing is 3 minutes
        hashGenesisBlock = uint256("0x0000029c5af0435f4cc6068205cba81e7d6f57722f29e8d49763ee229a702a84");
    } else {
        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0xd5;
        pchMessageStart[2] = 0xdb;
        pchMessageStart[3] = 0xfa;
	hashGenesisBlock = uint256("0x0000029c5af0435f4cc6068205cba81e7d6f57722f29e8d49763ee229a702a84");
    }


	//
	// Load block index from databases
	//
	if (!fReindex && !LoadBlockIndexDB())
		return false;

	return true;
}

bool InitBlockIndex() {
	// Check whether we're already initialized
	if (pindexGenesisBlock != NULL)
		return true;

	// Use the provided setting for -txindex in the new database
	fTxIndex = GetBoolArg("-txindex", false);
	pblocktree->WriteFlag("txindex", fTxIndex);
	printf("Initializing databases...\n");

	// Only add the genesis block if not reindexing (in which case we reuse the one already on disk)
	if (!fReindex) {



		 // Genesis block
		const char* pszTimestamp = "March 28 2014 Oculus didnt expect such a negative reaction to Facebook deal";
		CTransaction txNew;
		txNew.nTime = 1396162732;
		txNew.vin.resize(1);
		txNew.vout.resize(1);
		txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(9999) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
		txNew.vout[0].SetEmpty();

		CBlock block;
		block.vtx.push_back(txNew);
		block.hashPrevBlock = 0;
		block.hashMerkleRoot = block.BuildMerkleTree();
		block.nVersion = 1;
		block.nTime    = 1396162732;
		block.nBits    = bnProofOfWorkLimit.GetCompact();
		block.nNonce   = 1566135 ;

		if (fTestNet)
		{
			block.nTime    = 1449967517;
			block.nNonce   = 0; // hack, same
		}


		uint256 hash = block.GetHash();
		if (hash != hashGenesisBlock)
		{
			printf("Searching for genesis block...\n");
			// This will figure out a valid hash and Nonce if you're
			// creating a different genesis block:
			uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
            uint256 thash;
            char scratchpad[SCRYPT_SCRATCHPAD_SIZE];

            while (true)
            {
#if defined(USE_SSE2)
                // Detection would work, but in cases where we KNOW it always has SSE2,
                // it is faster to use directly than to use a function pointer or conditional.
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
                // Always SSE2: x86_64 or Intel MacOS X
                scrypt_1024_1_1_256_sp_sse2(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#else
                // Detect SSE2: 32bit x86 Linux or Windows
                scrypt_1024_1_1_256_sp(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#endif
#else
                // Generic scrypt
                scrypt_1024_1_1_256_sp_generic(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#endif
                if (thash <= hashTarget)
					break;
				if ((block.nNonce & 0xFFF) == 0)
				{
                    printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
				}
				++block.nNonce;
				if (block.nNonce == 0)
				{
					printf("NONCE WRAPPED, incrementing time\n");
					++block.nTime;
				}
			}
			printf("block.nTime = %u \n", block.nTime);
			printf("block.nNonce = %u \n", block.nNonce);
			printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
		}

		printf("%s\n", hash.ToString().c_str());
		printf("%s\n", hashGenesisBlock.ToString().c_str());
		printf("%s\n", block.hashMerkleRoot.ToString().c_str());
		block.print();
		assert(block.hashMerkleRoot == uint256("0x22aa4379d53ac896396484f52a39a9f7a4395dd7f5d2384b7d04e3cb7744698d"));
		//block.print();
		assert(block.GetHash() == hashGenesisBlock);

		// Start new block file
		try {
			unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
			CDiskBlockPos blockPos;
			CValidationState state;
			if (!FindBlockPos(state, blockPos, nBlockSize+8, 0, block.nTime))
				return error("LoadBlockIndex() : FindBlockPos failed");
			if (!block.WriteToDisk(blockPos))
				return error("LoadBlockIndex() : writing genesis block to disk failed");
			if (!block.AddToBlockIndex(state, blockPos))
				return error("LoadBlockIndex() : genesis block not accepted");
		} catch(std::runtime_error &e) {
			return error("LoadBlockIndex() : failed to initialize block database: %s", e.what());
		}
	}

	return true;
}

namespace Checkpoints
{
	// What makes a good checkpoint block?
	// + Is surrounded by blocks with reasonable timestamps
	//	 (no blocks before with a timestamp after, none after with
	//	  timestamp before)
	// + Contains no strange transactions
	static MapCheckpoints mapCheckpoints =
		boost::assign::map_list_of
		(	0, uint256("00000279dba2a8c40c2cd690f160f127a2a7edb8194f0d4e7605f212adf294bc"))
		(  100000, uint256("00000000a55f6c849f19f82b9bfd7463e4fb744c1aacfe3d5f498d0894168042"))
		(  109739, uint256("000000003370cbb513af63612d624e0c655b495d7a64f201d2352f34e0315e51"))
		(  200000, uint256("13b7a2de7f298b43ff400a7d62134a874e63287f4dc348f6bb39c42798577a2b"))
		(  300000, uint256("ae66c9d17f3f35a8fd9a1d6efc9af172bdc425f0bc26db0b18797beeeaaa3307"))
		(  400000, uint256("645818f9f433621b1c3780f4dafd15500679339b121367788a7ff82c68bc99e6"))
		;
	const CCheckpointData data = {
		&mapCheckpoints,
		1424439826, // * UNIX timestamp of last checkpoint block
		219689,	// * total number of transactions between genesis and last checkpoint
					//	 (the tx=... number in the SetBestChain debug.log lines)
		1000.0	   // * estimated number of transactions per day after checkpoint
	};

	static MapCheckpoints mapCheckpointsTestnet = 
		boost::assign::map_list_of
		(	546, uint256("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70"))
//		( 35000, uint256("2af959ab4f12111ce947479bfcef16702485f04afd95210aa90fde7d1e4a64ad"))
		;
	const CCheckpointData dataTestnet = {
		&mapCheckpointsTestnet,
		1369685559,
		37581,
		300
	}; /* estimated number of transactions per day after checkpoint */


	/* TODO enable this someday */

	// Hard checkpoints of stake modifiers to ensure they are deterministic
	static std::map<int, unsigned int> mapStakeModifierCheckpoints =
		boost::assign::map_list_of
		//( 0, 0x0e00670bu )
		(5516, 0xdeadbeefu)
		(16665, 0xdeadbeefu)
		;

}
