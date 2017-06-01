#!/usr/bin/env python3
# Copyright (c) 2014-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test RPCs related to blockchainstate.

Test the following RPCs:
    - gettxoutsetinfo
    - getdifficulty
    - getbestblockhash
    - getblockhash
    - getblockheader
    - getnetworkhashps
    - verifychain

Tests correspond to code in rpc/blockchain.cpp.
"""

from decimal import Decimal

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_raises_jsonrpc,
    assert_is_hex_string,
    assert_is_hash_string,
)


class BlockchainTest(BitcoinTestFramework):

    def __init__(self):
        super().__init__()
        self.setup_clean_chain = False
        self.num_nodes = 1

    def run_test(self):
        self._test_gettxoutsetinfo()
        self._test_getblockheader()
        self._test_getdifficulty()
        self._test_getnetworkhashps()
        self.nodes[0].verifychain(4, 0)

    def _test_gettxoutsetinfo(self):
        node = self.nodes[0]
        res = node.gettxoutsetinfo()

        assert_equal(res['total_amount'], Decimal('8725.00000000'))
        assert_equal(res['transactions'], 200)
        assert_equal(res['height'], 200)
        assert_equal(res['txouts'], 200)
        assert_equal(res['bytes_serialized'], 13924),
        assert_equal(res['bestblock'], node.getblockhash(200))
        assert_equal(len(res['bestblock']), 64)
        assert_equal(len(res['hash_serialized']), 64)

        self.log.info("Test that gettxoutsetinfo() works for blockchain with just the genesis block")
        b1hash = node.getblockhash(1)
        node.invalidateblock(b1hash)

        res2 = node.gettxoutsetinfo()
        assert_equal(res2['transactions'], 0)
        assert_equal(res2['total_amount'], Decimal('0'))
        assert_equal(res2['height'], 0)
        assert_equal(res2['txouts'], 0)
        assert_equal(res2['bestblock'], node.getblockhash(0))
        assert_equal(len(res2['hash_serialized']), 64)

        self.log.info("Test that gettxoutsetinfo() returns the same result after invalidate/reconsider block")
        node.reconsiderblock(b1hash)

        res3 = node.gettxoutsetinfo()
        assert_equal(res['total_amount'], res3['total_amount'])
        assert_equal(res['transactions'], res3['transactions'])
        assert_equal(res['height'], res3['height'])
        assert_equal(res['txouts'], res3['txouts'])
        assert_equal(res['bestblock'], res3['bestblock'])
        assert_equal(res['hash_serialized'], res3['hash_serialized'])

    def _test_getblockheader(self):
        node = self.nodes[0]

        assert_raises_jsonrpc(-5, "Block not found",
                              node.getblockheader, "nonsense")

        besthash = node.getbestblockhash()
        secondbesthash = node.getblockhash(199)
        header = node.getblockheader(besthash)

        assert_equal(header['hash'], besthash)
        assert_equal(header['height'], 200)
        assert_equal(header['confirmations'], 1)
        assert_equal(header['previousblockhash'], secondbesthash)
        assert_is_hex_string(header['chainwork'])
        assert_is_hash_string(header['hash'])
        assert_is_hash_string(header['previousblockhash'])
        assert_is_hash_string(header['merkleroot'])
        assert_is_hash_string(header['bits'], length=None)
        assert isinstance(header['time'], int)
        assert isinstance(header['mediantime'], int)
        assert isinstance(header['nonce'], int)
        assert isinstance(header['version'], int)
        assert isinstance(int(header['versionHex'], 16), int)
        assert isinstance(header['difficulty'], Decimal)

    def _test_getdifficulty(self):
        difficulty = self.nodes[0].getdifficulty()
        # 1 hash in 2 should be valid, so difficulty should be 1/2**31
        # binary => decimal => binary math is why we do this check
        assert abs(difficulty * 2**31 - 1) < 0.0001

    def _test_getnetworkhashps(self):
        hashes_per_second = self.nodes[0].getnetworkhashps()
        # This should be 2 hashes every 10 minutes or 1/300
        assert abs(hashes_per_second * 300 - 1) < 0.0001

if __name__ == '__main__':
    BlockchainTest().main()
