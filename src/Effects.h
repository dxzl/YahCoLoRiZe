//---------------------------------------------------------------------------
#ifndef EffectsH
#define EffectsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "Utils.h"
//---------------------------------------------------------------------------
class TProcessEffect
{
  friend class TDTSColor;

private:

  struct SPACEITEM
  {
    int Index;
    int BgColor;

    // constructor
    __fastcall SPACEITEM(int Index, int BgColor);
  };

  int __fastcall Rfg(void);
  int __fastcall Rbg(void);
  bool __fastcall IrcColorClash(int c1, int c2);
  bool __fastcall IsConflict(int fg, int bg);
  void __fastcall AddInitialCodes(int mode, PUSHSTRUCT &State,
                                                  WideString &TempStr);
  bool __fastcall IsFontTypeEffect(void);
  bool __fastcall IsFontSizeEffect(void);
  bool __fastcall IsBgColorEffect(int mode = -1);
  bool __fastcall IsFgColorEffect(int mode = -1);
  void __fastcall WriteColorsReplace(WideString &S, int fg, int bg);
  void __fastcall WriteColorsAlt(WideString &S, int Mode, int &fg, int &bg);
  void __fastcall WriteColorsRand(WideString &S, int Mode, int fg, int bg);
  void __fastcall WriteColorsInc(WideString &S, int Mode);
  void __fastcall Error(int code, wchar_t* pBuf1 = NULL, wchar_t* pBuf2 = NULL);

  bool __fastcall CheckExistingCodes(int &idx,  wchar_t buf[],
                                      WideString &Output, int NextParIdx);
  void __fastcall LimitBC(BlendColor &bc);
//  void __fastcall RestoreFont(String &TempStr);
//  void __fastcall RestoreColors(String &TempStr, PUSHSTRUCT ps, int mode=2);
//  void __fastcall RestoreFgColor(String &TempStr, PUSHSTRUCT ps);
//  void __fastcall RestoreBgColor(String &TempStr, PUSHSTRUCT ps);
  bool __fastcall CheckForOverlappingPushPop(wchar_t* pBuf, int iSize);
  void __fastcall StripSingleCode(wchar_t* pBuf, int &iSize, wchar_t Code);
  void __fastcall StripExistingColorCodes(wchar_t* pBuf, int &iSize);
  void __fastcall StripFGandUnderlineCodes(wchar_t* pBuf, int &iSize);
  void __fastcall StripFontSize(wchar_t* pBuf, int &iSize);
  void __fastcall StripFontType(wchar_t* pBuf, int &iSize);
  void __fastcall StripCodes(wchar_t* pBuf, int &iSize,
                                             bool bKeepProtectedCodes);
  WideString __fastcall ApplySingleCode(WideString S, wchar_t c);
  WideString __fastcall ApplyPushPop(WideString S);
  TList* __fastcall RecordSpaceBgColors(wchar_t* pBuf, int iSize);
  void __fastcall DeleteSpaceBgColors(TList* l);
  bool __fastcall ProcessStyle(TStringsW* sl, int ii, wchar_t c);

  bool bColorOn, bUnderlineOn, bBoldOn, bItalicsOn, bFontOn;
  bool bEP2On, bEP4On, bEPColorOn;
  bool bSkipWhitespace, bFirstChar, bInitEveryLine;
  bool bIsAllSpaces;

  PUSHSTRUCT InitialState, LeadingState, TrailingState;

  // E_MORPH_FGBG Effect's BlendColor structures
  BlendColor fgToBC, fgFromBC, bgToBC, bgFromBC;
  double fgRed, fgGreen, fgBlue; // +/- fraction to add to accumulated color
  double bgRed, bgGreen, bgBlue;
  double fgRedAcc, fgGreenAcc, fgBlueAcc;
  double bgRedAcc, bgGreenAcc, bgBlueAcc;

  int PrevFG, PrevBG;
  int Pushes;

  int effect;
  int colorCounter, colorCount;

  // Internal vars for properties
  bool m_bFullSpectFg, m_bFullSpectBg, m_allowUndo;
  int m_returnValue, m_returnDelta, m_maxColors;
  int m_ep0, m_ep1, m_ep2, m_ep3, m_ep4;
  double m_ep5;

  TList* SpaceBgColors;

  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TProcessEffect(TComponent* Owner, int Effect);
  __fastcall ~TProcessEffect();

  void __fastcall Execute(bool bShowStatus);

  __property bool AllowUndo = { write = m_allowUndo };
  __property bool FullSpectFg = { write = m_bFullSpectFg };
  __property bool FullSpectBg = { write = m_bFullSpectBg };
  __property int ReturnValue = { read = m_returnValue };
  __property int ReturnDelta = { read = m_returnDelta };
  __property int EP0 = { write = m_ep0 };
  __property int EP1 = { write = m_ep1 };
  __property int EP2 = { write = m_ep2 };
  __property int EP3 = { write = m_ep3 };
  __property int EP4 = { write = m_ep4 };
  __property double EP5 = { write = m_ep5 };
  __property int MaxColors = { read = m_maxColors, write = m_maxColors };
};
#endif

