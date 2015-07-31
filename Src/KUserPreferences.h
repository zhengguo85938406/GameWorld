#ifndef _KUSER_PREFERENCES_
#define _KUSER_PREFERENCES_

#include "GlobalEnum.h"

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KUserPreferences
{
public:
	KUserPreferences(void);
	~KUserPreferences(void);

	BOOL  Init(KPlayer* pPlayer);
	void  UnInit();

    BOOL  LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL  SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL  SetUserPreferences(int nOffset, int nLength, BYTE* pbyData);
	BOOL  GetUserPreferences(int nOffset, int nLength, BYTE* pbyData);
	BYTE* GetUserPreferences(int nOffset);
	BYTE* GetData();
private:
	BYTE        m_byUserPrefences[MAX_USER_PREFERENCES_LEN];
    KPlayer*    m_pPlayer;
};

#endif //_KUSER_PREFERENCES_
