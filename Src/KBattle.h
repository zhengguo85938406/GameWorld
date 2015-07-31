#pragma once
class KScene;
class KHero;
class KBattle
{
public:
    KBattle(void);
    virtual ~KBattle(void);

    BOOL Init(KScene* pScene);
    void UnInit();

    void Activate(int nCurLoop);
    BOOL IsFinished() const;

    void Start(int nStartFrame);
    int  GetStartedFrame();
    void ApplyTemplate(DWORD dwTemplateID);
    BOOL SetBattleTotalFrame(int nTotalFrame, BOOL bReTiming);

    void OnAddScore(int nNewScore);

    KScene* m_pScene;
    
    KBATTLE_MODE m_eMode;

    int     m_nFramePerBattle;
    int     m_nCurFrame;
    int     m_nStartFrame;

    int     m_nTotalScore;
    BOOL    m_bUptoTotalScore;

private:
    void    BroadcastTotalFrame();
};

