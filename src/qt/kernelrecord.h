#ifndef KERNELRECORD_H
#define KERNELRECORD_H

#include "uint256.h"
#include <QList>

class CWallet;
class CWalletTx;

class KernelRecord
{
public:
    KernelRecord():
        hash(), nTime(0), address(""), nValue(0), idx(0), spent(false), coinAge(0)
    {
    }

    KernelRecord(uint256 hash, int64 nTime):
            hash(hash), nTime(nTime), address(""), nValue(0), idx(0), spent(false), coinAge(0)
    {
    }

    KernelRecord(uint256 hash, int64 nTime,
                 const std::string &address,
                 int64 nValue, bool spent, int64 coinAge):
        hash(hash), nTime(nTime), address(address), nValue(nValue),
        idx(0), spent(spent), coinAge(coinAge)
    {
    }

    static bool showTransaction(const CWalletTx &wtx);
    static QList<KernelRecord> decomposeOutput(const CWallet *wallet, const CWalletTx &wtx);


    uint256 hash;
    int64 nTime;
    std::string address;
    int64 nValue;
    int idx;
    bool spent;
    int64 coinAge;

    std::string getTxID();
    int64 getAge() const;
    double getProbToMintStake(double difficulty) const;
    double getProbToMintWithinNMinutes(double difficulty, int minutes) const;
};

#endif // KERNELRECORD_H
