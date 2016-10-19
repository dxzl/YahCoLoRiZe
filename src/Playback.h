//---------------------------------------------------------------------------
#ifndef PlaybackH
#define PlaybackH

#include <Classes.hpp>
//---------------------------------------------------------------------------
// Forward reference...
class TPlay;
//---------------------------------------------------------------------------

typedef long (*tColorStart)(LPTSTR Service, LPTSTR Channel,
                              LPTSTR Filename, int PlayTime, bool bUseFile);
typedef long (*tColor)(void);

class TPlayback
{
  private:
    bool __fastcall InitLocalPlayback(int Time);
    bool __fastcall MoveTextToGPlayBuffer(void);
    bool __fastcall MoveTextToYahooBuffer(wchar_t* &pBuf, int &iSize);

    TDTSColor* dts; // easy pointer to main form...

  public:
    __fastcall TPlayback(TComponent* Owner); // constructor
    __fastcall ~TPlayback(void); // destructor

    bool __fastcall DoStartPlay(int Time);
    String __fastcall GetDLLVersion(void);

    TPlay* play;
};
//---------------------------------------------------------------------------
#endif
