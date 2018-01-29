/* this ugly hack copyright AGPLv3 2018 Catoshi's catbox */

#include <index.h>

/* We have no mempool, but TxToUniv needs this, so return false */
#if defined(FEATURE_INDEX)
bool GetSpentIndex(CSpentIndexKey &key, CSpentIndexValue &value)
{
	return false;
}
#endif
