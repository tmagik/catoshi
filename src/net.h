// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2012 The PPCoin developers
// Previously Distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include <deque>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <openssl/rand.h>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include "mruset.h"
#include "limitedmap.h"
#include "netbase.h"
#include "protocol.h"
#include "addrman.h"
#include "hash.h"
#include "bloom.h"

class CNode;
class CBlockIndex;
extern int nBestHeight;



inline unsigned int ReceiveFloodSize() { return 1000*GetArg("-maxreceivebuffer", 5*1000); }
inline unsigned int SendBufferSize() { return 1000*GetArg("-maxsendbuffer", 1*1000); }

void AddOneShot(std::string strDest);
bool RecvLine(SOCKET hSocket, std::string& strLine);
bool GetMyExternalIP(CNetAddr& ipRet);
void AddressCurrentlyConnected(const CService& addr);
CNode* FindNode(const CNetAddr& ip);
CNode* FindNode(const CService& ip);
CNode* ConnectNode(CAddress addrConnect, const char *strDest = NULL);
void MapPort(bool fUseUPnP);
unsigned short GetListenPort();
bool BindListenPort(const CService &bindAddr, std::string& strError=REF(std::string()));
void StartNode(boost::thread_group& threadGroup);
bool StopNode();
void SocketSendData(CNode *pnode);

enum
{
    LOCAL_NONE,   // unknown
    LOCAL_IF,     // address a local interface listens on
    LOCAL_BIND,   // address explicit bound to
    LOCAL_UPNP,   // address reported by UPnP
    LOCAL_HTTP,   // address reported by whatismyip.com and similar
    LOCAL_MANUAL, // address explicitly specified (-externalip=)

    LOCAL_MAX
};

void SetLimited(enum Network net, bool fLimited = true);
bool IsLimited(enum Network net);
bool IsLimited(const CNetAddr& addr);
bool AddLocal(const CService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& addr, int nScore = LOCAL_NONE);
bool SeenLocal(const CService& addr);
bool IsLocal(const CService& addr);
bool GetLocal(CService &addr, const CNetAddr *paddrPeer = NULL);
bool IsReachable(const CNetAddr &addr);
void SetReachable(enum Network net, bool fFlag = true);
CAddress GetLocalAddress(const CNetAddr *paddrPeer = NULL);


extern bool fDiscover;
extern uint64_t nLocalServices;
extern CAddress addrSeenByPeer;
extern uint64_t nLocalHostNonce;
extern CAddrMan addrman;
extern int nMaxConnections;

extern std::vector<CNode*> vNodes;
extern CCriticalSection cs_vNodes;
extern std::map<CInv, CDataStream> mapRelay;
extern std::deque<std::pair<int64_t, CInv> > vRelayExpiration;
extern CCriticalSection cs_mapRelay;
extern limitedmap<CInv, int64_t> mapAlreadyAskedFor;

extern std::vector<std::string> vAddedNodes;
extern CCriticalSection cs_vAddedNodes;




class CNodeStats
{
public:
    uint64_t nServices;
    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    std::string addrName;
    int nVersion;
    std::string cleanSubVer;
    bool fInbound;
    int nStartingHeight;
    int nMisbehavior;
    uint64_t nSendBytes;
    uint64_t nRecvBytes;
    uint64_t nBlocksRequested;
    bool fSyncNode;
};




class CNetMessage {
public:
    bool in_data;                   // parsing header (false) or data (true)

    CDataStream hdrbuf;             // partially received header
    CMessageHeader hdr;             // complete header
    unsigned int nHdrPos;

    CDataStream vRecv;              // received message data
    unsigned int nDataPos;

    CNetMessage(int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), vRecv(nTypeIn, nVersionIn) {
        hdrbuf.resize(24);
        in_data = false;
        nHdrPos = 0;
        nDataPos = 0;
    }

    bool complete() const
    {
        if (!in_data)
            return false;
        return (hdr.nMessageSize == nDataPos);
    }

    void SetVersion(int nVersionIn)
    {
        hdrbuf.SetVersion(nVersionIn);
        vRecv.SetVersion(nVersionIn);
    }

    int readHeader(const char *pch, unsigned int nBytes);
    int readData(const char *pch, unsigned int nBytes);
};





/** Information about a peer */
class CNode
{
public:
    // socket
    uint64_t nServices;
    SOCKET hSocket;
    CDataStream ssSend;
    size_t nSendSize; // total size of all vSendMsg entries
    size_t nSendOffset; // offset inside the first vSendMsg already sent
    uint64_t nSendBytes;
    std::deque<CSerializeData> vSendMsg;
    CCriticalSection cs_vSend;

    std::deque<CInv> vRecvGetData;
    std::deque<CNetMessage> vRecvMsg;
    CCriticalSection cs_vRecvMsg;
    uint64_t nRecvBytes;
    int nRecvVersion;

    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nLastSendEmpty;
    int64_t nTimeConnected;
    uint64_t nBlocksRequested;
    CAddress addr;
    std::string addrName;
    CService addrLocal;
    int nVersion;
    // strSubVer is whatever byte array we read from the wire. However, this field is intended 
    // to be printed out, displayed to humans in various forms and so on. So we sanitize it and
    // store the sanitized version in cleanSubVer. The original should be used when dealing with
    // the network or wire types and the cleaned string used when displayed or logged.
    std::string strSubVer, cleanSubVer;
    bool fOneShot;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    bool fDisconnect;
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in their version message that we should not relay tx invs
    //    until they have initialized their bloom filter.
    bool fRelayTxes;
    CSemaphoreGrant grantOutbound;
    CCriticalSection cs_filter;
    CBloomFilter* pfilter;
    int nRefCount;
protected:

    // Denial-of-service detection/prevention
    // Key is IP address, value is banned-until-time
    static std::map<CNetAddr, int64_t> setBanned;
    static CCriticalSection cs_setBanned;
    int nMisbehavior;

public:
    uint256 hashContinue;
    CBlockIndex* pindexLastGetBlocksBegin;
    uint256 hashLastGetBlocksEnd;
    int nStartingHeight;
    bool fStartSync;

    // flood relay
    std::vector<CAddress> vAddrToSend;
    std::set<CAddress> setAddrKnown;
    bool fGetAddr;
    std::set<uint256> setKnown;

    // inventory based relay
    mruset<CInv> setInventoryKnown;
    std::vector<CInv> vInventoryToSend;
    CCriticalSection cs_inventory;
    std::multimap<int64_t, CInv> mapAskFor;

    CNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false) : ssSend(SER_NETWORK, INIT_PROTO_VERSION)
    {
        nServices = 0;
        hSocket = hSocketIn;
        nRecvVersion = INIT_PROTO_VERSION;
        nLastSend = 0;
        nLastRecv = 0;
        nSendBytes = 0;
        nRecvBytes = 0;
        nLastSendEmpty = GetTime();
        nTimeConnected = GetTime();
        nBlocksRequested = 0;
        addr = addrIn;
        addrName = addrNameIn == "" ? addr.ToStringIPPort() : addrNameIn;
        nVersion = 0;
        strSubVer = "";
        fOneShot = false;
        fClient = false; // set by version message
        fInbound = fInboundIn;
        fNetworkNode = false;
        fSuccessfullyConnected = false;
        fDisconnect = false;
        nRefCount = 0;
        nSendSize = 0;
        nSendOffset = 0;
        hashContinue = 0;
        pindexLastGetBlocksBegin = 0;
        hashLastGetBlocksEnd = 0;
        nStartingHeight = -1;
        fStartSync = false;
        fGetAddr = false;
        nMisbehavior = 0;
        fRelayTxes = false;
        setInventoryKnown.max_size(SendBufferSize() / 1000);
        pfilter = new CBloomFilter();

        // Be shy and don't send version until we hear
        if (hSocket != INVALID_SOCKET && !fInbound)
            PushVersion();
    }

    ~CNode()
    {
        if (hSocket != INVALID_SOCKET)
        {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
        }
        if (pfilter)
            delete pfilter;
    }

private:
    CNode(const CNode&);
    void operator=(const CNode&);
public:


    int GetRefCount()
    {
        assert(nRefCount >= 0);
        return nRefCount;
    }

    // requires LOCK(cs_vRecvMsg)
    unsigned int GetTotalRecvSize()
    {
        unsigned int total = 0;
        BOOST_FOREACH(const CNetMessage &msg, vRecvMsg) 
            total += msg.vRecv.size() + 24;
        return total;
    }

    // requires LOCK(cs_vRecvMsg)
    bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);

    // requires LOCK(cs_vRecvMsg)
    void SetRecvVersion(int nVersionIn)
    {
        nRecvVersion = nVersionIn;
        BOOST_FOREACH(CNetMessage &msg, vRecvMsg)
            msg.SetVersion(nVersionIn);
    }

    CNode* AddRef()
    {
        nRefCount++;
        return this;
    }

    void Release()
    {
        nRefCount--;
    }



    void AddAddressKnown(const CAddress& addr)
    {
        setAddrKnown.insert(addr);
    }

    void PushAddress(const CAddress& addr)
    {
        // Known checking here is only to save space from duplicates.
        // SendMessages will filter it again for knowns that were added
        // after addresses were pushed.
        if (addr.IsValid() && !setAddrKnown.count(addr))
            vAddrToSend.push_back(addr);
    }


    void AddInventoryKnown(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            setInventoryKnown.insert(inv);
        }
    }

    void PushInventory(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            if (!setInventoryKnown.count(inv))
                vInventoryToSend.push_back(inv);
        }
    }

    void AskFor(const CInv& inv)
    {
        // We're using mapAskFor as a priority queue,
        // the key is the earliest time the request can be sent
        int64_t nRequestTime;
        limitedmap<CInv, int64_t>::const_iterator it = mapAlreadyAskedFor.find(inv);
        if (it != mapAlreadyAskedFor.end())
            nRequestTime = it->second;
        else
            nRequestTime = 0;
        if (fDebugNet)
            printf("askfor %s   %" PRId64" (%s)\n", inv.ToString().c_str(), nRequestTime, DateTimeStrFormat("%H:%M:%S", nRequestTime/1000000).c_str());

        // Make sure not to reuse time indexes to keep things in the same order
        int64_t nNow = (GetTime() - 1) * 1000000;
        static int64_t nLastTime;
        ++nLastTime;
        nNow = std::max(nNow, nLastTime);
        nLastTime = nNow;

        // Each retry is 2 minutes after the last
        nRequestTime = std::max(nRequestTime + 2 * 60 * 1000000, nNow);
        if (it != mapAlreadyAskedFor.end())
            mapAlreadyAskedFor.update(it, nRequestTime);
        else
            mapAlreadyAskedFor.insert(std::make_pair(inv, nRequestTime));
        mapAskFor.insert(std::make_pair(nRequestTime, inv));
    }



    // TODO: Document the postcondition of this function.  Is cs_vSend locked?
    void BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(cs_vSend)
    {
        ENTER_CRITICAL_SECTION(cs_vSend);
        assert(ssSend.size() == 0);
        ssSend << CMessageHeader(pszCommand, 0);
        if (fDebug)
            printf("sending: %s ", pszCommand);
    }

    // TODO: Document the precondition of this function.  Is cs_vSend locked?
    void AbortMessage() UNLOCK_FUNCTION(cs_vSend)
    {
        ssSend.clear();

        LEAVE_CRITICAL_SECTION(cs_vSend);

        if (fDebug)
            printf("(aborted)\n");
    }

    // TODO: Document the precondition of this function.  Is cs_vSend locked?
    void EndMessage() UNLOCK_FUNCTION(cs_vSend)
    {
        if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
        {
            printf("dropmessages DROPPING SEND MESSAGE\n");
            AbortMessage();
            return;
        }

        if (ssSend.size() == 0)
            return;

        // Set the size
        unsigned int nSize = ssSend.size() - CMessageHeader::HEADER_SIZE;
        memcpy((char*)&ssSend[CMessageHeader::MESSAGE_SIZE_OFFSET], &nSize, sizeof(nSize));

        // Set the checksum
        uint256 hash = Hash(ssSend.begin() + CMessageHeader::HEADER_SIZE, ssSend.end());
        unsigned int nChecksum = 0;
        memcpy(&nChecksum, &hash, sizeof(nChecksum));
        assert(ssSend.size () >= CMessageHeader::CHECKSUM_OFFSET + sizeof(nChecksum));
        memcpy((char*)&ssSend[CMessageHeader::CHECKSUM_OFFSET], &nChecksum, sizeof(nChecksum));

        if (fDebug) {
            printf("(%d bytes)\n", nSize);
        }

        std::deque<CSerializeData>::iterator it = vSendMsg.insert(vSendMsg.end(), CSerializeData());
        ssSend.GetAndClear(*it);
        nSendSize += (*it).size();

        // If write queue empty, attempt "optimistic write"
        if (it == vSendMsg.begin())
            SocketSendData(this);

        LEAVE_CRITICAL_SECTION(cs_vSend);
    }

    void PushVersion();


    void PushMessage(const char* pszCommand)
    {
        try
        {
            BeginMessage(pszCommand);
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1>
    void PushMessage(const char* pszCommand, const T1& a1)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    void PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd);
    bool IsSubscribed(unsigned int nChannel);
    void Subscribe(unsigned int nChannel, unsigned int nHops=0);
    void CancelSubscribe(unsigned int nChannel);
    void CloseSocketDisconnect();
    void Cleanup();


    // Denial-of-service detection/prevention
    // The idea is to detect peers that are behaving
    // badly and disconnect/ban them, but do it in a
    // one-coding-mistake-won't-shatter-the-entire-network
    // way.
    // IMPORTANT:  There should be nothing I can give a
    // node that it will forward on that will make that
    // node's peers drop it. If there is, an attacker
    // can isolate a node and/or try to split the network.
    // Dropping a node for sending stuff that is invalid
    // now but might be valid in a later version is also
    // dangerous, because it can cause a network split
    // between nodes running old code and nodes running
    // new code.
    static void ClearBanned(); // needed for unit testing
    static bool IsBanned(CNetAddr ip);
    bool Misbehaving(int howmuch); // 1 == a little, 100 == a lot
    void copyStats(CNodeStats &stats);
};


/* shouldn't these be part of the actual CTransaction/CBlock class */
class CTransaction;
void RelayTransaction(const CTransaction& tx, const uint256& hash);
void RelayTransaction(const CTransaction& tx, const uint256& hash, const CDataStream& ss);

class CBlock;
void RelayBlock(const CBlock& tx, const uint256& hash);
void RelayBlock(const CBlock& tx, const uint256& hash, const CDataStream& ss);

#endif
