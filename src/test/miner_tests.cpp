// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "miner.h"
#include "pubkey.h"
#include "uint256.h"
#include "util.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(miner_tests)

static
struct {
    unsigned char extranonce;
    unsigned int nonce;
} blockinfo[] = {
    {4, 0xa4ad9f65}, {2, 0x15cf2b27}, {1, 0x037620ac}, {1, 0x700d9c54},
    {2, 0xce79f74f}, {2, 0x52d9c194}, {1, 0x77bc3efc}, {2, 0xbb62c5e8},
    {2, 0x83ff997a}, {1, 0x48b984ee}, {1, 0xef925da0}, {2, 0x680d2979},
    {2, 0x08953af7}, {1, 0x087dd553}, {2, 0x210e2818}, {2, 0xdfffcdef},
    {1, 0xeea1b209}, {2, 0xba4a8943}, {1, 0xa7333e77}, {1, 0x344f3e2a},
    {3, 0xd651f08e}, {2, 0xeca3957f}, {2, 0xca35aa49}, {1, 0x6bb2065d},
    {2, 0x0170ee44}, {1, 0x6e12f4aa}, {2, 0x43f4f4db}, {2, 0x279c1c44},
    {2, 0xb5a50f10}, {2, 0xb3902841}, {2, 0xd198647e}, {2, 0x6bc40d88},
    {1, 0x633a9a1c}, {2, 0x9a722ed8}, {2, 0x55580d10}, {1, 0xd65022a1},
    {2, 0xa12ffcc8}, {1, 0x75a6a9c7}, {2, 0xfb7c80b7}, {1, 0xe8403e6c},
    {1, 0xe34017a0}, {3, 0x659e177b}, {2, 0xba5c40bf}, {5, 0x022f11ef},
    {1, 0xa9ab516a}, {5, 0xd0999ed4}, {1, 0x37277cb3}, {1, 0x830f735f},
    {1, 0xc6e3d947}, {2, 0x824a0c1b}, {1, 0x99962416}, {1, 0x75336f63},
    {1, 0xaacf0fea}, {1, 0xd6531aec}, {5, 0x7afcf541}, {5, 0x9d6fac0d},
    {1, 0x4cf5c4df}, {1, 0xabe0f2a0}, {6, 0x4a3dac18}, {2, 0xf265febe},
    {2, 0x1bc9f23f}, {1, 0xad49ab71}, {1, 0x9f2d8923}, {1, 0x15acb65d},
    {2, 0xd1cecb52}, {2, 0xf856808b}, {1, 0x0fa96e29}, {1, 0xe063ecbc},
    {1, 0x78d926c6}, {5, 0x3e38ad35}, {5, 0x73901915}, {1, 0x63424be0},
    {1, 0x6d6b0a1d}, {2, 0x888ba681}, {2, 0xe96b0714}, {1, 0xb7fcaa55},
    {2, 0x19c106eb}, {1, 0x5aa13484}, {2, 0x5bf4c2f3}, {2, 0x94d401dd},
    {1, 0xa9bc23d9}, {1, 0x3a69c375}, {1, 0x56ed2006}, {5, 0x85ba6dbd},
    {1, 0xfd9b2000}, {1, 0x2b2be19a}, {1, 0xba724468}, {1, 0x717eb6e5},
    {1, 0x70de86d9}, {1, 0x74e23a42}, {1, 0x49e92832}, {2, 0x6926dbb9},
    {0, 0x64452497}, {1, 0x54306d6f}, {2, 0x97ebf052}, {2, 0x55198b70},
    {2, 0x03fe61f0}, {1, 0x98f9e67f}, {1, 0xc0842a09}, {1, 0xdfed39c5},
    {1, 0x3144223e}, {1, 0xb3d12f84}, {1, 0x7366ceb7}, {5, 0x6240691b},
    {2, 0xd3529b57}, {1, 0xf4cae3b1}, {1, 0x5b1df222}, {1, 0xa16a5c70},
    {2, 0xbbccedc6}, {2, 0xfe38d0ef},
};

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity)
{
    CScript scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    CBlockTemplate *pblocktemplate;
    CMutableTransaction tx,tx2;
    CScript script;
    uint256 hash;

    LOCK(cs_main);
    Checkpoints::fEnabled = false;

    // Simple block creation, nothing special yet:
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));

    // We can't make transactions until we have inputs
    // Therefore, load 100 blocks :)
    std::vector<CTransaction*>txFirst;
    for (unsigned int i = 0; i < sizeof(blockinfo)/sizeof(*blockinfo); ++i)
    {
        CBlock *pblock = &pblocktemplate->block; // pointer for convenience
        pblock->nVersion = 1;
        pblock->nTime = chainActive.Tip()->GetMedianTimePast()+1;
        CMutableTransaction txCoinbase(pblock->vtx[0]);
        txCoinbase.vin[0].scriptSig = CScript();
        txCoinbase.vin[0].scriptSig.push_back(blockinfo[i].extranonce);
        txCoinbase.vin[0].scriptSig.push_back(chainActive.Height());
        txCoinbase.vout[0].scriptPubKey = CScript();
        pblock->vtx[0] = CTransaction(txCoinbase);
        if (txFirst.size() < 2)
            txFirst.push_back(new CTransaction(pblock->vtx[0]));
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();
        pblock->nNonce = blockinfo[i].nonce;
        CValidationState state;
        BOOST_CHECK(ProcessNewBlock(state, NULL, pblock));
        BOOST_CHECK(state.IsValid());
        pblock->hashPrevBlock = pblock->GetHash();
    }
    delete pblocktemplate;

    // Just to make sure we can still make simple blocks
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;

    // block sigops > limit: 1000 CHECKMULTISIG + 1
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = 5000000000LL;
    for (unsigned int i = 0; i < 1001; ++i)
    {
        tx.vout[0].nValue -= 1000000;
        hash = tx.GetHash();
        mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // block size > limit
    tx.vin[0].scriptSig = CScript();
    // 18 * (520char + DROP) + OP_1 = 9433 bytes
    std::vector<unsigned char> vchData(520);
    for (unsigned int i = 0; i < 18; ++i)
        tx.vin[0].scriptSig << vchData << OP_DROP;
    tx.vin[0].scriptSig << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vout[0].nValue = 5000000000LL;
    for (unsigned int i = 0; i < 128; ++i)
    {
        tx.vout[0].nValue -= 10000000;
        hash = tx.GetHash();
        mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // orphan in mempool
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // child with higher priority than parent
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vout[0].nValue = 4900000000LL;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vin[0].prevout.hash = hash;
    tx.vin.resize(2);
    tx.vin[1].scriptSig = CScript() << OP_1;
    tx.vin[1].prevout.hash = txFirst[0]->GetHash();
    tx.vin[1].prevout.n = 0;
    tx.vout[0].nValue = 5900000000LL;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // coinbase in mempool
    tx.vin.resize(1);
    tx.vin[0].prevout.SetNull();
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    tx.vout[0].nValue = 0;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // invalid (pre-p2sh) txn in mempool
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = 4900000000LL;
    script = CScript() << OP_0;
    tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vin[0].prevout.hash = hash;
    tx.vin[0].scriptSig = CScript() << (std::vector<unsigned char>)script;
    tx.vout[0].nValue -= 1000000;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // double spend txn pair in mempool
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = 4900000000LL;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vout[0].scriptPubKey = CScript() << OP_2;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // subsidy changing
    int nHeight = chainActive.Height();
    chainActive.Tip()->nHeight = 209999;
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    chainActive.Tip()->nHeight = 210000;
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    chainActive.Tip()->nHeight = nHeight;

    // non-final txs in mempool
    SetMockTime(chainActive.Tip()->GetMedianTimePast()+1);

    // height locked
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].nSequence = 0;
    tx.vout[0].nValue = 4900000000LL;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    tx.nLockTime = chainActive.Tip()->nHeight+1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(!IsFinalTx(tx, chainActive.Tip()->nHeight + 1));

    // time locked
    tx2.vin.resize(1);
    tx2.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx2.vin[0].prevout.n = 0;
    tx2.vin[0].scriptSig = CScript() << OP_1;
    tx2.vin[0].nSequence = 0;
    tx2.vout.resize(1);
    tx2.vout[0].nValue = 4900000000LL;
    tx2.vout[0].scriptPubKey = CScript() << OP_1;
    tx2.nLockTime = chainActive.Tip()->GetMedianTimePast()+1;
    hash = tx2.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx2, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(!IsFinalTx(tx2));

    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));

    // Neither tx should have make it into the template.
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 1);
    delete pblocktemplate;

    // However if we advance height and time by one, both will.
    chainActive.Tip()->nHeight++;
    SetMockTime(chainActive.Tip()->GetMedianTimePast()+2);

    BOOST_CHECK(IsFinalTx(tx, chainActive.Tip()->nHeight + 1));
    BOOST_CHECK(IsFinalTx(tx2));

    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 3);
    delete pblocktemplate;

    chainActive.Tip()->nHeight--;
    SetMockTime(0);
    mempool.clear();

    BOOST_FOREACH(CTransaction *tx, txFirst)
        delete tx;

    Checkpoints::fEnabled = true;
}

BOOST_AUTO_TEST_SUITE_END()
