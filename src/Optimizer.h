//---------------------------------------------------------------------------
#ifndef OptimizerH
#define OptimizerH

#include <Classes.hpp>
//---------------------------------------------------------------------------

class TOptimizer
{
  private:

    PUSHSTRUCT State, SaveState;
    int initialFG, initialBG;

    WideString __fastcall SetFormatCodes(PUSHSTRUCT State,
                                     PUSHSTRUCT &SaveState, wchar_t c);

    TDTSColor* dts; // easy pointer to main form...

  public:
    __fastcall TOptimizer(TComponent* Owner); // constructor

    WideString __fastcall Execute(WideString sIn, bool bShowStatus=false);
    bool __fastcall StripRedundantFgRGB(wchar_t* &pOld,
                                     int &OldSize, bool bShowStatus);
    bool __fastcall StripRedundantFgRGB(WideString &S, bool bShowStatus);

    __property int InitialFG = { read = initialFG, write = initialFG };
};
//---------------------------------------------------------------------------
#endif
