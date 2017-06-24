#if defined(PPCOINSTAKE)
// ppcoin: total coin age spent in transaction, in the unit of coin-days.
// Only those coins meeting minimum age requirement counts. As those
// transactions not in main chain are not currently indexed so we
// might not find out about their coin age. Older transactions are 
// guaranteed to be in main chain by sync-checkpoint. This rule is
// introduced to help nodes establish a consistent view of the coin
// age (trust score) of competing branches.
bool CTransaction::GetCoinAge(CValidationState &state, CCoinsViewCache &view, uint64_t& nCoinAge) const
{
    CBigNum bnCentSecond = 0;  // coin age in the unit of cent-seconds
    nCoinAge = 0;

    if (IsCoinBase())
        return true;

    BOOST_FOREACH(const CTxIn& txin, vin)
    {
        // First try finding the previous transaction in database
        const COutPoint &prevout = txin.prevout;
        CCoins coins;

        if (!view.GetCoins(prevout.hash, coins))
            continue;  // previous transaction not in main chain
        if (nTime < coins.nTime)
            return false;  // Transaction timestamp violation

        // Transaction index is required to get to block header
        if (!fTxIndex)
            return false;  // Transaction index not available

        CDiskTxPos postx;
        CTransaction txPrev;
        if (pblocktree->ReadTxIndex(prevout.hash, postx))
        {
            CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
            CBlockHeader header;
            try {
                file >> header;
                fseek(file, postx.nTxOffset, SEEK_CUR);
                file >> txPrev;
            } catch (std::exception &e) {
                return error("%s() : deserialize or I/O error in GetCoinAge()", __PRETTY_FUNCTION__);
            }
            if (txPrev.GetHash() != prevout.hash)
                return error("%s() : txid mismatch in GetCoinAge()", __PRETTY_FUNCTION__);

            if (header.GetBlockTime() + nStakeMinAge > nTime)
                continue; // only count coins meeting min age requirement

            int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
            bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

            if (fDebug && GetBoolArg("-printcoinage"))
                printf("coin age nValueIn=%-12" PRId64" nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
        }
        else
            return error("%s() : tx missing in tx index in GetCoinAge()", __PRETTY_FUNCTION__);
    }

    CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);
    if (fDebug && GetBoolArg("-printcoinage"))
        printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
    nCoinAge = bnCoinDay.getuint64();
    return true;
}

// ppcoin: total coin age spent in block, in the unit of coin-days.
bool CBlock::GetCoinAge(uint64_t& nCoinAge) const
{
    nCoinAge = 0;

    CCoinsViewCache view(*pcoinsTip, true);
    BOOST_FOREACH(const CTransaction& tx, vtx)
    {
        uint64_t nTxCoinAge;
        CValidationState state;
        if (tx.GetCoinAge(state, view, nTxCoinAge))
            nCoinAge += nTxCoinAge;
        else
            return false;
    }

    if (nCoinAge == 0) // block coin age minimum 1 coin-day
        nCoinAge = 1;
    if (fDebug && GetBoolArg("-printcoinage"))
        printf("block coin age total nCoinDays=%" PRId64"\n", nCoinAge);
    return true;
}
#endif /* PPCOINSTAKE */
