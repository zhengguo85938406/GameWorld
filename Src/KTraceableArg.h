// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KTraceableArg.h
//  Creator 	: Hanruofei  
//  Date		: 12/16/2011
//  Comment		: 
// ***************************************************************

#pragma once

template <class T, int nCount>
class KTraceableArg
{
private:
    T   m_aValues[nCount];
    int m_nRecordedCount;
public:
    KTraceableArg()
    {
        m_nRecordedCount = 0;
        memset(m_aValues, 0, sizeof(m_aValues));
    }

    ~KTraceableArg()
    {
   
    }

    void Record(int nIndex, const T& tValue)
    {
        nIndex = nIndex % nCount;
        m_aValues[nIndex] = tValue;
        if(m_nRecordedCount < nCount)
            ++m_nRecordedCount;
    }

    const T& GetRecord(int nIndex) const
    {
        assert(nIndex >= 0 && nIndex < nCount);
        return m_aValues[nIndex];
    }

    int GetRecordedCount() const {return m_nRecordedCount;}

    void ClearRecord() {m_nRecordedCount = 0;}
    
    int GetMaxRecordCount() const {return nCount;}
};
