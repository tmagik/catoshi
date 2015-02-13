// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013-2014 The Catcoin developers
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "givecoin.h"
#include "checkpoints.h"
#include "db.h"
#include "txdb.h"
#include "net.h"
#include "init.h"
#include "ui_interface.h"
#include "checkqueue.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;

uint256 hashGenesisBlock = 0;			// TODO: objectize this for multicoin support

const string strMessageMagic = "Givecoin Signed Message:\n";

/* stake stuff TODO: */
static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
static CBigNum bnProofOfStakeLimit(~uint256(0) >> 20);

static CBigNum bnProofOfWorkLimitTestNet(~uint256(0) >> 20);
static CBigNum bnProofOfStakeLimitTestNet(~uint256(0) >> 20);

unsigned int nStakeMinAge = 60 * 60 * 24 * 2;	// minimum age for coin age: 2d
unsigned int nStakeMaxAge = 60 * 60 * 24 * 100;	// stake age of full weight: -1
unsigned int nStakeTargetSpacing = 90;			// 60 sec block spacing
unsigned int nStakeTargetSpacing2 = 60;			// 90 sec block spacing

//int nCoinbaseMaturity = 350; // old from bluecoin
/* end stake stuff */

// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
// FIXME use a single string and/or objectify this
/*
  Givecoin policy for getting on this list:
  TODO: come up with a policy
 */
const char *strMainNetDNSSeed[][2] = 
{
   // {"weminemnc.com", "dnsseed.weminemnc.com"},
   {"testrelay.com", "givecoin.no-ip.biz"},
   {"primarySEED.com", "5.250.177.28"},
   {"givecoinSecondarySeed.com", "66.172.12.54"},
   {NULL, NULL}
};

const char *strTestNetDNSSeed[][2] = {
    //{"Givecointools.com", "testnet-seed.Givecointools.com"},
    //{"weminemnc.com", "testnet-seed.weminemnc.com"},
    {NULL, NULL}
};

int64_t static GetBlockValue(int nHeight, int64_t nFees)
{
    int64_t nSubsidy = 0;
    if (nHeight <= 5) {    // For Each 5 blocks will have 0.5M coins
       nSubsidy = 5000000 * COIN;
    }
    else {
       nSubsidy = 1000 * COIN;
    }
    // Subsidy is cut in half every 250,000 blocks, which will occur approximately every .5 year
    nSubsidy >>= (nHeight / 250000); // Givecoin: 250k blocks in ~.5 years
    //
    if (nSubsidy < COIN) nSubsidy = COIN;  // Minimum Number of Coin = 1
    return nSubsidy + nFees;
}

static const int64_t nTargetTimespan = 10 * 60;                                   // Givecoin: Difficulty adjusted every 10 mintues
static const int64_t nTargetSpacing =   1 * 60;                                   // Givecoin: Every Minute
static const int64_t nInterval = nTargetTimespan / nTargetSpacing;                // 10 block readjustment

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime)
{
    // Testnet has min-difficulty blocks
    // after nTargetSpacing*2 time between blocks:
    if (fTestNet && nTime > nTargetSpacing*2)
        return bnProofOfWorkLimit.GetCompact();

    CBigNum bnResult;
    bnResult.SetCompact(nBase);
    while (nTime > 0 && bnResult < bnProofOfWorkLimit)
    {
        // Maximum 400% adjustment...
        bnResult *= 4;
        // ... in best-case exactly 4-times-normal target time
        nTime -= nTargetTimespan*4;
    }
    if (bnResult > bnProofOfWorkLimit)
        bnResult = bnProofOfWorkLimit;
    return bnResult.GetCompact();
}

unsigned int static GetNextWorkRequired_V1(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
   unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per interval
    if ((pindexLast->nHeight+1) % nInterval != 0)
    {
        // Special difficulty rule for testnet:
        if (fTestNet)
        {
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }

        return pindexLast->nBits;
    }

    // DarkCoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = nInterval-1;
    if ((pindexLast->nHeight+1) != nInterval)
        blockstogoback = nInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    printf("  nActualTimespan = %" PRId64"  before bounds\n", nActualTimespan);
    if (nActualTimespan < nTargetTimespan/4)
        nActualTimespan = nTargetTimespan/4;
    if (nActualTimespan > nTargetTimespan*4)
        nActualTimespan = nTargetTimespan*4;

    // Retarget
    CBigNum bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnProofOfWorkLimit)
        bnNew = bnProofOfWorkLimit;

    return bnNew.GetCompact();
}


unsigned int static KimotoGravityWell(const CBlockIndex* pindexLast, const CBlockHeader *pblock, uint64_t TargetBlocksSpacingSeconds, uint64_t PastBlocksMin, uint64_t PastBlocksMax) {

        const CBlockIndex *BlockLastSolved = pindexLast;
        const CBlockIndex *BlockReading = pindexLast;
        const CBlockHeader *BlockCreating = pblock;
        BlockCreating = BlockCreating;
        uint64_t PastBlocksMass = 0;
        int64_t PastRateActualSeconds = 0;
        int64_t PastRateTargetSeconds = 0;
        double PastRateAdjustmentRatio = double(1);
        CBigNum PastDifficultyAverage;
        CBigNum PastDifficultyAveragePrev;
        double EventHorizonDeviation;
        double EventHorizonDeviationFast;
        double EventHorizonDeviationSlow;

        if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) {
           return bnProofOfWorkLimit.GetCompact();
        }

        int64_t LatestBlockTime = BlockLastSolved->GetBlockTime();

        for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {

            if ((PastBlocksMax > 0) && (i > PastBlocksMax)) {
               break;
            }
            PastBlocksMass++;

            if (i == 1) {
               PastDifficultyAverage.SetCompact(BlockReading->nBits);
            }
            else {
               PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;

            if (LatestBlockTime < BlockReading->GetBlockTime()) {
               LatestBlockTime = BlockReading->GetBlockTime();
            }

            PastRateActualSeconds = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
            PastRateTargetSeconds = TargetBlocksSpacingSeconds * PastBlocksMass;
            PastRateAdjustmentRatio = double(1);

            if (PastRateActualSeconds < 1) {
               PastRateActualSeconds  = 1;
            }
            if ((PastRateActualSeconds != 0) && (PastRateTargetSeconds != 0)) {
               PastRateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
            }
            EventHorizonDeviation = 1 + (0.7084 * pow((double(PastBlocksMass)/double(28.2)), -1.228));
            EventHorizonDeviationFast = EventHorizonDeviation;
            EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

            if (PastBlocksMass >= PastBlocksMin) {
               if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) { assert(BlockReading); break; }
            }
            if (BlockReading->pprev == NULL) {
               assert(BlockReading);
               break;
            }
            BlockReading = BlockReading->pprev;

        }

        CBigNum bnNew(PastDifficultyAverage);
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
           bnNew *= PastRateActualSeconds;
           bnNew /= PastRateTargetSeconds;
        }

    if (bnNew > bnProofOfWorkLimit) {
        bnNew = bnProofOfWorkLimit;
    }

    return bnNew.GetCompact();
}

unsigned int static GetNextWorkRequired_V2(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
        static const int64_t BlocksTargetSpacing   = 60;                    // 60 seconds
        static const unsigned int TimeDaySeconds = 60 * 60 * 24;

        int64_t  PastSecondsMin = TimeDaySeconds * 0.02; // 0.01;
        int64_t  PastSecondsMax = TimeDaySeconds * 0.30; // 0.14;
        uint64_t PastBlocksMin  = PastSecondsMin / BlocksTargetSpacing;
        uint64_t PastBlocksMax  = PastSecondsMax / BlocksTargetSpacing;

        return KimotoGravityWell(pindexLast, pblock, BlocksTargetSpacing, PastBlocksMin, PastBlocksMax);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
        int DiffMode = 1;
        if (fTestNet) {
             if (pindexLast->nHeight+1 >=   50) { DiffMode = 2; }
        }
        else {
             if      (pindexLast->nHeight+1 >= 34804) { DiffMode = 1; }
             else if (pindexLast->nHeight+1 >=   200) { DiffMode = 2; }
        }
        if      (DiffMode == 1) { return GetNextWorkRequired_V1(pindexLast, pblock); }
        else if (DiffMode == 2) { return GetNextWorkRequired_V2(pindexLast, pblock); }
        return GetNextWorkRequired_V2(pindexLast, pblock);
}

bool LoadBlockIndex()
{
    if (fTestNet)
    {
        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        hashGenesisBlock = uint256("0x8bbee88d47cc0862fb5cc5db1045097d1dd2457393b24f8c74fa506a82661f05");
    } else {
        pchMessageStart[0] = 0xd1;
        pchMessageStart[1] = 0xd2;
        pchMessageStart[2] = 0xd3;
        pchMessageStart[3] = 0xdb;
        hashGenesisBlock = uint256("0x00000279dba2a8c40c2cd690f160f127a2a7edb8194f0d4e7605f212adf294bc");
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



         // Genesis Block:
         // block.nTime = 1393884829
         // 2014-03-09 22:00:10 block.nNonce = 2088083604
         // 2014-03-09 22:00:10 block.GetHash = f092eda63502ff8b19f584c56fc6ed32a9e265b798831690ea9aaf44cc74f7f0
         // Genesis block

        const char* pszTimestamp = "CNN 03/Mar/20a14 Ukraine crisis: UN Security Council meets";
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 1000 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
        CBlock block;
        block.vtx.push_back(txNew);
        block.hashPrevBlock = 0;
        block.hashMerkleRoot = block.BuildMerkleTree();
        block.nVersion = 1;
        block.nTime    = 1393884829;
        block.nBits    = bnProofOfWorkLimit.GetCompact();
        block.nNonce   = 2090389678;

        if (fTestNet)
        {
            block.nTime    = 1393884829;
            block.nNonce   = 2085170731;
        }

        /*
        uint256 hash;
        if (true)
        {
            printf("Searching for genesis block...\n");
            // This will figure out a valid hash and Nonce if you're
            // creating a different genesis block:
            uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
           // char scratchpad[SCRYPT_SCRATCHPAD_SIZE];

            loop
            {
                hash = block.GetHash();
                if (hash <= hashTarget)
                    break;
                if ((block.nNonce & 0xFFF) == 0)
                {
                    printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, hash.ToString().c_str(), hashTarget.ToString().c_str());
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
        */
        uint256 hash = block.GetHash();
        printf("%s\n", hash.ToString().c_str());
        printf("%s\n", hashGenesisBlock.ToString().c_str());
        printf("%s\n", block.hashMerkleRoot.ToString().c_str());
        assert(block.hashMerkleRoot == uint256("0xdc43fdf63088ee667d957a39f87b217822b9e3d31805b008124c4e821079a43c"));
        block.print();
        assert(hash == hashGenesisBlock);

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
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
          (     0, uint256("0xf092eda63502ff8b19f584c56fc6ed32a9e265b798831690ea9aaf44cc74f7f0"))
   //     (  4032, uint256("0x9ce90e427198fc0ef05e5905ce3503725b80e26afd35a987965fd7e3d9cf0846"))
     //   (  8064, uint256("0xeb984353fc5190f210651f150c40b8a4bab9eeeff0b729fcb3987da694430d70"))
    //    ( 16128, uint256("0x602edf1859b7f9a6af809f1d9b0e6cb66fdc1d4d9dcd7a4bec03e12a1ccd153d"))
   //     ( 23420, uint256("0xd80fdf9ca81afd0bd2b2a90ac3a9fe547da58f2530ec874e978fce0b5101b507"))
    //    ( 50000, uint256("0x69dc37eb029b68f075a5012dcc0419c127672adb4f3a32882b2b3e71d07a20a6"))
    //    ( 80000, uint256("0x4fcb7c02f676a300503f49c764a89955a8f920b46a8cbecb4867182ecdb2e90a"))
    //    (120000, uint256("0xbd9d26924f05f6daa7f0155f32828ec89e8e29cee9e7121b026a7a3552ac6131"))
    //    (161500, uint256("0xdbe89880474f4bb4f75c227c77ba1cdc024991123b28b8418dbbf7798471ff43"))
    //    (179620, uint256("0x2ad9c65c990ac00426d18e446e0fd7be2ffa69e9a7dcb28358a50b2b78b9f709"))
    //    (240000, uint256("0x7140d1c4b4c2157ca217ee7636f24c9c73db39c4590c4e6eab2e3ea1555088aa"))
     //   (383640, uint256("0x2b6809f094a9215bafc65eb3f110a35127a34be94b7d0590a096c3f126c6f364"))
    //    (409004, uint256("0x487518d663d9f1fa08611d9395ad74d982b667fbdc0e77e9cf39b4f1355908a3"))
    //    (456000, uint256("0xbf34f71cc6366cd487930d06be22f897e34ca6a40501ac7d401be32456372004"))
        ;
    const CCheckpointData data = {
        &mapCheckpoints,
        1383887110, // * UNIX timestamp of last checkpoint block
        2179203,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        8000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        (   546, uint256("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70"))
        ( 35000, uint256("2af959ab4f12111ce947479bfcef16702485f04afd95210aa90fde7d1e4a64ad"))
        ;
    const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1369685559,
        37581,
        300
    }; /* estimated number of transactions per day after checkpoint */
}
