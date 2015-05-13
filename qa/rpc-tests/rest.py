#!/usr/bin/env python2
# Copyright (c) 2014 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test REST interface
#

from test_framework import BitcoinTestFramework
from util import *
from struct import *
import binascii
import json
import StringIO

try:
    import http.client as httplib
except ImportError:
    import httplib
try:
    import urllib.parse as urlparse
except ImportError:
    import urlparse

def deser_uint256(f):
    r = 0
    for i in range(8):
        t = unpack(b"<I", f.read(4))[0]
        r += t << (i * 32)
    return r

#allows simple http get calls with a request body
def http_get_call(host, port, path, requestdata = '', response_object = 0):
    conn = httplib.HTTPConnection(host, port)
    conn.request('GET', path, requestdata)

    if response_object:
        return conn.getresponse()

    return conn.getresponse().read()

class RESTTest (BitcoinTestFramework):
    FORMAT_SEPARATOR = "."

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 3)

    def setup_network(self, split=False):
        self.nodes = start_nodes(3, self.options.tmpdir)
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()
        
    def run_test(self):
        url = urlparse.urlparse(self.nodes[0].url)
        print "Mining blocks..."
        
        self.nodes[0].generate(1)
        self.sync_all()
        self.nodes[2].generate(100)
        self.sync_all()
        
        assert_equal(self.nodes[0].getbalance(), 50)
        
        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        bb_hash = self.nodes[0].getbestblockhash()
        
        assert_equal(self.nodes[1].getbalance(), Decimal("0.1")) #balance now should be 0.1 on node 1
        
        # load the latest 0.1 tx over the REST API
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint 
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']
        
        
        ######################################
        # GETUTXOS: query a unspent outpoint #
        ######################################
        json_request = '{"checkmempool":true,"outpoints":[{"txid":"'+txid+'","n":'+str(n)+'}]}'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request)
        json_obj = json.loads(json_string)
        
        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)
        
        #make sure there is one utxo
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['utxos'][0]['value'], 0.1)
        
        
        ################################################
        # GETUTXOS: now query a already spent outpoint #
        ################################################
        json_request = '{"checkmempool":true,"outpoints":[{"txid":"'+vintx+'","n":0}]}'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request)
        json_obj = json.loads(json_string)
        
        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)

        #make sure there is no utox in the response because this oupoint has been spent
        assert_equal(len(json_obj['utxos']), 0)
        
        #check bitmap
        assert_equal(json_obj['bitmap'], "0")
        
        
        ##################################################
        # GETUTXOS: now check both with the same request #
        ##################################################
        json_request = '{"checkmempool":true,"outpoints":[{"txid":"'+txid+'","n":'+str(n)+'},{"txid":"'+vintx+'","n":0}]}'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request)
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['bitmap'], "10")
        
        #test binary response
        bb_hash = self.nodes[0].getbestblockhash()

        binaryRequest = b'\x01\x02'
        binaryRequest += binascii.unhexlify(txid)
        binaryRequest += pack("i", n);
        binaryRequest += binascii.unhexlify(vintx);
        binaryRequest += pack("i", 0);
        
        bin_response = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'bin', binaryRequest)
        
        output = StringIO.StringIO()
        output.write(bin_response)
        output.seek(0)
        chainHeight = unpack("i", output.read(4))[0]
        hashFromBinResponse = hex(deser_uint256(output))[2:].zfill(65).rstrip("L")
        
        assert_equal(bb_hash, hashFromBinResponse) #check if getutxo's chaintip during calculation was fine
        assert_equal(chainHeight, 102) #chain height must be 102
        
        
        ############################
        # GETUTXOS: mempool checks #
        ############################

        # do a tx and don't sync
        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint 
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']
        
        json_request = '{"checkmempool":false,"outpoints":[{"txid":"'+txid+'","n":'+str(n)+'}]}'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request)
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 0) #there should be a outpoint because it has just added to the mempool
        
        json_request = '{"checkmempool":true,"outpoints":[{"txid":"'+txid+'","n":'+str(n)+'}]}'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request)
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1) #there should be a outpoint because it has just added to the mempool
        
        #do some invalid requests
        json_request = '{"checkmempool'
        response = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid json request
        
        json_request = '{"checkmempool'
        response = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'bin', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid bin request
        
        #test limits
        json_request = '{"checkmempool":true,"outpoints":['
        for x in range(0, 200):
            json_request += '{"txid":"'+txid+'","n":'+str(n)+'},'
        json_request = json_request.rstrip(",")
        json_request+="]}";
        response = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we exceeding the limits
        
        json_request = '{"checkmempool":true,"outpoints":['
        for x in range(0, 90):
            json_request += '{"txid":"'+txid+'","n":'+str(n)+'},'
        json_request = json_request.rstrip(",")
        json_request+="]}";
        response = http_get_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request, True)
        assert_equal(response.status, 200) #must be a 500 because we exceeding the limits

        self.nodes[0].generate(1) #generate block to not affect upcomming tests
        self.sync_all()
        
        ################
        # /rest/block/ #
        ################
        
        # check binary format
        response = http_get_call(url.hostname, url.port, '/rest/block/'+bb_hash+self.FORMAT_SEPARATOR+"bin", "", True)
        assert_equal(response.status, 200)
        assert_greater_than(int(response.getheader('content-length')), 80)
        response_str = response.read()

        # compare with block header
        response_header = http_get_call(url.hostname, url.port, '/rest/headers/1/'+bb_hash+self.FORMAT_SEPARATOR+"bin", "", True)
        assert_equal(response_header.status, 200)
        assert_equal(int(response_header.getheader('content-length')), 80)
        response_header_str = response_header.read()
        assert_equal(response_str[0:80], response_header_str)

        # check block hex format
        response_hex = http_get_call(url.hostname, url.port, '/rest/block/'+bb_hash+self.FORMAT_SEPARATOR+"hex", "", True)
        assert_equal(response_hex.status, 200)
        assert_greater_than(int(response_hex.getheader('content-length')), 160)
        response_hex_str = response_hex.read()
        assert_equal(response_str.encode("hex")[0:160], response_hex_str[0:160])

        # compare with hex block header
        response_header_hex = http_get_call(url.hostname, url.port, '/rest/headers/1/'+bb_hash+self.FORMAT_SEPARATOR+"hex", "", True)
        assert_equal(response_header_hex.status, 200)
        assert_greater_than(int(response_header_hex.getheader('content-length')), 160)
        response_header_hex_str = response_header_hex.read()
        assert_equal(response_hex_str[0:160], response_header_hex_str[0:160])
        assert_equal(response_header_str.encode("hex")[0:160], response_header_hex_str[0:160])

        # check json format
        json_string = http_get_call(url.hostname, url.port, '/rest/block/'+bb_hash+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(json_obj['hash'], bb_hash)

        # do tx test
        tx_hash = json_obj['tx'][0]['txid'];
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+tx_hash+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        assert_equal(json_obj['txid'], tx_hash)

        # check hex format response
        hex_string = http_get_call(url.hostname, url.port, '/rest/tx/'+tx_hash+self.FORMAT_SEPARATOR+"hex", "", True)
        assert_equal(hex_string.status, 200)
        assert_greater_than(int(response.getheader('content-length')), 10)
        


        # check block tx details
        # let's make 3 tx and mine them on node 1
        txs = []
        txs.append(self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 11))
        txs.append(self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 11))
        txs.append(self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 11))
        self.sync_all()

        # now mine the transactions
        newblockhash = self.nodes[1].generate(1)
        self.sync_all()

        #check if the 3 tx show up in the new block
        json_string = http_get_call(url.hostname, url.port, '/rest/block/'+newblockhash[0]+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        for tx in json_obj['tx']:
            if not 'coinbase' in tx['vin'][0]: #exclude coinbase
                assert_equal(tx['txid'] in txs, True)

        #check the same but without tx details
        json_string = http_get_call(url.hostname, url.port, '/rest/block/notxdetails/'+newblockhash[0]+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        for tx in txs:
            assert_equal(tx in json_obj['tx'], True)

        #test rest bestblock
        bb_hash = self.nodes[0].getbestblockhash()
        
        json_string = http_get_call(url.hostname, url.port, '/rest/chaininfo.json')
        json_obj = json.loads(json_string)
        assert_equal(json_obj['bestblockhash'], bb_hash)

if __name__ == '__main__':
    RESTTest ().main ()
