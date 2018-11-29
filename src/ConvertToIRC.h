//---------------------------------------------------------------------------
#ifndef ConvertToIRCH
#define ConvertToIRCH

#include <Classes.hpp>
//---------------------------------------------------------------------------

class TConvertToIRC
{
private:
  int __fastcall GetSideBorderTotalWidth(void);
  void __fastcall InitBoldUnderlineReverseForNextLine(void);
  void __fastcall AdjustMultiplesOfTopBorderSegment(int &TotalWidth);
  void __fastcall InitPushPop(void);
  void __fastcall DoPop(PUSHSTRUCT TempState, WideString &TempStr);
  void __fastcall Pop(PUSHSTRUCT &p);
  void __fastcall Push(PUSHSTRUCT p);

  void __fastcall RestoreFGBG(int fg, int bg, WideString &TempStr);
  void __fastcall RestoreFGBG(WideString &TempStr);

  void __fastcall RestoreFontSize(WideString &TempStr, int fs = -1);
  void __fastcall RestoreFontType(WideString &TempStr, int ft = -1);
  void __fastcall RestoreBoldUnderlineItalics(PUSHSTRUCT NewState,
                                                    WideString &TempStr);

  bool __fastcall PrintTopBottomBorders(TStringsW* EditStrings);
  bool __fastcall AddColorLine(bool bInhibitColors);
  bool __fastcall AddLine(void);
  bool __fastcall InitNextLine(bool bInhibitColors = false);
  void __fastcall SetLineWidth(int Lmargin, int Rmargin, int Indent);
  int __fastcall SelectWings(int NextWordLen);
  void __fastcall ProperlyTerminateEffects(void);
  void __fastcall ProperlyTerminateBoldUnderlineItalics(WideString &TempStr);
  int __fastcall AddScriptingChars(WideString ColorFormatStr);
  void __fastcall AddWingToText(WideString ColorFormatStr);
  int __fastcall LargestWingLength(void);

  wchar_t* pBuf;
  int iSize, iBuf;
  int IRCLineCount, TotalWingLength, NextWordLength;

  bool bWingActive, bLeftHasColor, bRightHasColor;
  bool bOrigTextHasColor, bAddLineOnHyphen;
  bool bFoundCRLF;
  bool bIndentEnabled, bIsFirstLineOfPar;
  bool bOrigTextHasBold, bOrigTextHasUnderline, bOrigTextHasItalics;
  bool bPadLeftWing;

  // Used in vertical-blend
  int OrigLineCount, OrigLineWidth;
  int ActualLineCount, Iterations, MaxIterations;
  int SideLength, TopLength;

  // Need this to keep wrong size from being saved
  // in registry when Auto-Linesize is forced on for
  // !AddColor.
  int LineWidth; // property private to this class
  int HeightCounter, LineHeight; // property private to this class
  int LineSpacing; // property private to this class
  int Lmargin, Rmargin, Indent; // properties private to this class
  int Tmargin, Bmargin; // properties private to this class

  WideString TextLine, PrintableText, TopStr, WingPadding;

  PUSHSTRUCT State; // Current text attributes

  // Push/Pop ************************************
  PUSHSTRUCT* PUSHSTRUCTS;
  int PushIdx, PushPopLineCounter;

  WideString LeftWingStr, RightWingStr;

  TStringsW* EditStrings;

  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TConvertToIRC(TComponent* Owner);
  __fastcall ~TConvertToIRC(void);

  int __fastcall Execute(bool bShowStatus);
};
//---------------------------------------------------------------------------
 extern TConvertToIRC* ProcessIRC;
//---------------------------------------------------------------------------
#endif

