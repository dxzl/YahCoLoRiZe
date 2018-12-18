//---------------------------------------------------------------------------
#ifndef UtilsH
#define UtilsH

#include <Classes.hpp>
#include "PUSHSTRUCT.h"
#include "PASTESTRUCT.h"
//---------------------------------------------------------------------------

// Strip Search/Replace Modes for StripModeForm.cpp and StripCodesClick()
#define STRIP_ALL           0
#define STRIP_ALL_PRESERVE  1  // Keep codes within push/pop
#define STRIP_BOLD          2
#define STRIP_UNDERLINE     3
#define STRIP_ITALICS       4
#define STRIP_FG_COLOR      5
#define STRIP_BG_COLOR      6
#define STRIP_ALL_COLOR     7
#define STRIP_FONT_CODES    8
#define STRIP_PUSHPOP       9
#define STRIP_CTRL_O        10
#define STRIP_TAB           11
#define STRIP_FF            12

// these are in seperate menu items, not in DlgStrip.cpp
#define STRIP_TRIM_SPACES           30
#define STRIP_TRIM_TRAILING_SPACES  31
#define STRIP_PAD_SPACES            32
#define STRIP_CRLF                  33

#define S_NOCODE     0 // Returns 0 if no special code at ii (must be 0!)
#define S_COLOR      1 // Returns 1 if Color
#define S_FONTTYPE   2 // Returns 2 if FontType
#define S_FONTSIZE   3 // Returns 3 if FontSize
#define S_CRLF       4 // Returns 4 if CR/LF
#define S_NULL       5 // Returns 5 if C_NULL '\0'
#define S_TAB        6 // Returns 6 if C_TAB
#define S_FF         7 // Returns 7 if C_FF
#define S_OTHER      8 // Returns 8 if any other control-char

#define STATE_MODE_FIRSTCHAR  0
#define STATE_MODE_LASTCHAR  -1
#define STATE_MODE_ENDOFBUF  -2

#define DO_NOTHING 5000 // MUST NOT BE ANY COLOR OR SPECIAL COLOR
                        // DESIGNATOR SUCH AS IRCCANCEL or RGBCOLOR
                        // (as defined in Main.h)

// Constants for SHGetFolderPathW
#define SHGFP_TYPE_CURRENT 0 // current value for user, verify it exists
#define SHGFP_TYPE_DEFAULT 1 // default value, may not exist

// FlattenTabs modes
#define FLATTEN_TABS_SINGLE_SPACE       0
#define FLATTEN_TABS_STRIP              1
#define FLATTEN_TABS_MULTIPLE_SPACES    2

class TUtils
{
  friend class TStringsW;

private:
  bool __fastcall SelectedIsHidden(TForm* f);
  TColor __fastcall GetFirstEnabledBlendButtonColor(BlendColor* BC);
  String __fastcall PrintFgA(int c);
  String __fastcall PrintBgA(int c);

  unsigned short cbHTML, cbRTF; // Registered clipboard format

  TDTSColor* dts; // easy pointer to main form...

public:
    __fastcall TUtils(TComponent* Owner); // constructor

  int __fastcall min(int a, int b);
  int __fastcall max(int a, int b);

  ONCHANGEW __fastcall GetInfoOC(TTaeRichEdit* re, TStringsW* sl);
  ONCHANGEW __fastcall GetInfoOC(TTaeRichEdit* re);
  ONCHANGEW __fastcall GetInfoOC(void);
  void __fastcall SetOldLineVars(TTaeRichEdit* re, bool bInit=false);

//  void __fastcall PrintWindow(HWND hwnd);

  void __fastcall ShowHex(WideString S);
  void __fastcall ShowHex(void);
  void __fastcall ShowHex(wchar_t* pBuf, int iSize);
  WideString __fastcall GetHex(WideString S);
  WideString __fastcall GetHex(wchar_t* pBuf, int iSize);
//  void __fastcall SetScrollRange(TTaeRichEdit* re, int L, int H,
//                                                    bool bHoriz = false);
//  int __fastcall GetScrollPos(TTaeRichEdit* re);
//  void __fastcall SetScrollPos(TTaeRichEdit* re, int Pos, bool bReDraw);
  void __fastcall WaitForThread(void);
  bool __fastcall LoadFromStringList(TTaeRichEdit* re, TStringsW* sl,
                                                      bool bPlainText=false);
  bool __fastcall StringToStream(WideString S, TMemoryStream* &ms);
  char* __fastcall StreamToBufferA(TMemoryStream* ms);
  wchar_t* __fastcall StreamToBufferW(TMemoryStream* ms);
  AnsiString __fastcall StreamToStringA(TMemoryStream* ms);
  WideString __fastcall StreamToStringW(TMemoryStream* ms);
  WideString __fastcall HtmlToIrc(WideString FileName, bool bShowStatus);
//  WideString __fastcall OneLineIRCtoHtml(WideString S);
  String __fastcall GoFormat(String FormatStr, int Parm1 );
  bool __fastcall IsAnyOtherFontType(wchar_t * pBuf, int iSize, int ft);
  bool __fastcall IsAnyOtherFontSize(wchar_t* pBuf, int iSize, int fs);
  PASTESTRUCT __fastcall CutCopyRTF(bool bCut, bool bCopy,
                                          TTaeRichEdit* re, TStringsW* sl);
  PASTESTRUCT __fastcall CutCopyRTF(bool bCut, bool bCopy,
                                      TTaeRichEdit* re, WideString &wText);
  PASTESTRUCT __fastcall CutCopyIRC(bool bCut, bool bCopy,
                                              TTaeRichEdit* re, TStringsW* sl);
  PASTESTRUCT __fastcall CutCopyIRC(bool bCut, bool bCopy,
                                    TTaeRichEdit* re, WideString &wText);
  int __fastcall WildcardDelete(WideString S);
  int __fastcall SkipCodes(WideString S);
  int __fastcall SkipCodes(WideString S, int iBufStart, wchar_t &c);
  int __fastcall SkipCodes(wchar_t* pBuf, int iSize, int iBufStart, wchar_t &c);
  int __fastcall SkipCode(WideString wIn, int &ii);
  int __fastcall SkipCode(wchar_t* pBuf, int iSize, int &ii);
  int __fastcall SkipAllCodes(WideString S);
  int __fastcall SkipAllCodes(wchar_t* pBuf, int ii, int iSize);

  bool __fastcall GetSelectedZoneIndices(TTaeRichEdit* re, int &First,
                                                                int &Last);
  WideString __fastcall MoveMainTextToString(void);
  bool __fastcall MoveMainTextToBuffer(wchar_t* &pBuf, int &iSize);
  wchar_t* __fastcall MoveTextToBuffer(wchar_t* pSource, int iFirst,
                                                      int iLast, int &iSize);

  // Clipboard
  WideString __fastcall ClipboardHtmlToIrc(bool bShowStatus);
  String __fastcall LoadHtmlFromClipboard(void);
  WideString __fastcall GetClipboardText(void);
  bool __fastcall CopyRtfToClipboard(String sIn);
  bool __fastcall CopyTextToClipboard(WideString wIn);
  void __fastcall ClearClipboard(void);

  void __fastcall ShowState(PUSHSTRUCT ps, int iRow=-1, int iColumn=-1);
  WideString __fastcall PrintStateString(PUSHSTRUCT ps, bool bLeading);

  bool __fastcall GetIndicesAndStates(TStringsW* sl, TTaeRichEdit* re,
                              PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast );
  bool __fastcall GetIndicesAndStates(wchar_t* pBuf, int iSize,
              TTaeRichEdit * re, PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast);
  bool __fastcall GetIndicesAndStates(WideString S, TTaeRichEdit* re,
                               PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast);
  bool __fastcall GetIndicesAndStates(wchar_t* pBuf, int iSize,
                              PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast);
  bool __fastcall GetIndicesAndStates(WideString S,
                                 PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast);

  WideString __fastcall GetTrailingState(WideString &wInOut);
  PUSHSTRUCT __fastcall GetLeadingState(WideString wIn, int &iEnd);
  PUSHSTRUCT __fastcall GetTrailingState(WideString wIn, int &iStart,
                                                            int &iEnd);

  int __fastcall GetState(WideString wIn, PUSHSTRUCT &psState, int Idx,
                           bool bCopyAfgAbg, bool bTrackSpaceColorChanges=true);
  int __fastcall GetState(wchar_t* pBuf, int iSize,
         PUSHSTRUCT &psState, int Idx, bool bCopyAfgAbg, bool bTrackSpaceColorChanges=true);

  int __fastcall GetState(WideString wIn, WideString &State, int Idx,
                     bool bLeading, bool bCopyAfgAbg, bool bTrackSpaceColorChanges=true);
  int __fastcall GetState(wchar_t* pBuf, int iSize, WideString &State,
                  int Idx, bool bLeading, bool bCopyAfgAbg, bool bTrackSpaceColorChanges=true);

  int __fastcall SetStateFlags(wchar_t* pBuf, int iSize, int iBufStart,
          int iCharStopAt, PUSHSTRUCT &ps, bool bTrackSpaceColorChanges=true);
  int __fastcall SetStateFlags(wchar_t* pBuf, int iSize, int iCharStopAt,
                            PUSHSTRUCT &ps, bool bTrackSpaceColorChanges=true);
  int __fastcall SetStateFlags(WideString S, int iCharStopAt,
                         PUSHSTRUCT &ps, bool bTrackSpaceColorChanges=true);
  int __fastcall SetStateFlags(WideString S, int iBufStart,
              int iCharStopAt, PUSHSTRUCT &ps, bool bTrackSpaceColorChanges=true);

  bool __fastcall ResolveState(PUSHSTRUCT TrailingState, WideString &TempStr);
  int __fastcall ResolveStateForPaste(TPoint p, WideString &wInOut,
                                                      WideString wPaste);
//  bool __fastcall ResolveStateForPaste(TTaeRichEdit* re,
//                                WideString DestStr, WideString &PasteStr);
  WideString __fastcall ResolveStateForCut(PUSHSTRUCT TrailingState,
                                                        PUSHSTRUCT NewState);

  bool __fastcall TerminateEffects(PUSHSTRUCT EndState, WideString &Str);

  int __fastcall GetRealIndex(TStringsW* sl, int ii);
  int __fastcall GetRealIndex(WideString S, int ii);
  int __fastcall GetRealIndex(wchar_t* pBuf, int iSize, int ii);

  int __fastcall GetCodeIndex(TStringsW* sl, int realIdx, int startLine,
                                                          bool bCodes=false);
  int __fastcall GetCodeIndex(wchar_t* pBuf, int iSize, int RealStart,
                                                           bool bCodes=false);
  int __fastcall GetCodeIndex(TStringsW* sl, int RealStart,
                                                     bool bCodeIdx=false);
  int __fastcall GetCodeIndex(TTaeRichEdit* re, bool bCodeIdx=false);
  int __fastcall GetCodeIndex(WideString S, int RealStart, bool bCodes=false);

  bool __fastcall GetCodeIndices(TStringsW* sl, int &FirstIdx, int &LastIdx,
                                           int &CodeIdx, TTaeRichEdit* re);
  bool __fastcall GetCodeIndices(TStringsW* sl, int &FirstIdx, int &LastIdx,
                               int &CodeIdx, int RealStart, int RealCount);
  bool __fastcall GetCodeIndices(int &FirstIdx, int &LastIdx, int &CodeIdx,
                                                           TTaeRichEdit* re);
  bool __fastcall GetCodeIndices(WideString S, int &FirstIdx, int &LastIdx,
                                 int &CodeIdx, int RealStart, int RealCount);
  bool __fastcall GetCodeIndices(WideString S, int &FirstIdx, int &LastIdx,
                                             int &CodeIdx, TTaeRichEdit* re);
  bool __fastcall GetCodeIndices(wchar_t* pBuf, int iSize,
                  int &FirstIdx, int &LastIdx, int &CodeIdx, TTaeRichEdit* re);
  bool __fastcall GetCodeIndices(wchar_t* pBuf, int iSize,
      int &FirstIdx, int &LastIdx, int &CodeIdx, int RealStart, int RealCount);

  int __fastcall GetRealLineLengthFromSL(TStringsW* sl, int LineNumber);

  int __fastcall GetRealLength(WideString S);
  bool __fastcall GetRealLength(WideString sIn, int &Length);
  int __fastcall GetRealLength(wchar_t* pBuf, int iSize);
  int __fastcall GetRealLength(WideString S, int idx, wchar_t c,
                                     bool bForceEmotesToBeIncludedInCount);
  int __fastcall GetRealLength(wchar_t* pBuf, int iSize, int idx, wchar_t c,
                                        bool bForceEmotesToBeIncludedInCount);

  int __fastcall GetWordLength(wchar_t* pBuf, int iSize, int idx, int &hyphidx);

  WideString __fastcall DecodeHighlight(TTaeRichEdit* re);
  bool __fastcall EncodeHighlight(TStringsW* sl, TTaeRichEdit* re,
                                                        bool bShowStatus=false);
  bool __fastcall EncodeHighlight(WideString s, TTaeRichEdit* re,
                                                      bool bShowStatus=false);
  bool __fastcall EncodeHighlight(TTaeRichEdit* re, bool bShowStatus=false);
  wchar_t __fastcall GetHighlightCode(wchar_t in);
  wchar_t __fastcall GetHighlightLetter(wchar_t in);

  void __fastcall SetTabs(TTaeRichEdit* re, int width);
  void __fastcall SetFont(TTaeRichEdit* re);
  bool __fastcall SetRichEditFont(TTaeRichEdit* re, WideString FontName = "");
  bool __fastcall SetFontStyle(TFontStyles cStyle, TTaeRichEdit* re);
  bool __fastcall SetDefaultFont(TTaeRichEdit* re);
  WideString __fastcall Optimize(WideString S, bool bShowStatus,
                                                      int fgc=DO_NOTHING);
  bool __fastcall Optimize(TStringsW* sl, bool bShowStatus, int fgc=DO_NOTHING);
  bool __fastcall Optimize(bool bShowStatus);

  void __fastcall LoadMenu(TMenu* M, char** array);
  bool __fastcall AnySpecialChar(wchar_t c);

  // Invisible-spaces algorithm
  bool __fastcall ReplaceSpaces(TStringsW* sl);
  WideString __fastcall ReplaceSpaces(WideString sIn);
  bool __fastcall ReplaceSpaces(wchar_t* &pBuf, int &iSize);

  void __fastcall PadTrailingSpaces(wchar_t* &pBuf, int &iSize, int iWidth);
  void __fastcall PadTrailingSpaces(TStringsW* sl, int iWidth);
  WideString __fastcall PadTrailingSpaces(WideString wIn, int iWidth);

  bool __fastcall ContainsChar(WideString S, wchar_t c);
  WideString __fastcall StripChar(WideString S, wchar_t c);
  WideString __fastcall StripChar(WideString S, wchar_t c, int iStart, int &iEnd);
  bool __fastcall StripParagraphCRLF(wchar_t* &pBuf, int &iSize);
  bool __fastcall StripLeadingAndTrailingSpaces(wchar_t * &pBuf, int &iSize);
  WideString __fastcall GetTrailingCodes(WideString S);
  WideString __fastcall StripTrailingCodes(WideString S);
  bool __fastcall StripTrailingCodes(wchar_t* &pBuf, int &iSize);
  void __fastcall StripTrailingSpaces(wchar_t* &pBuf, int &iSize);
  void __fastcall StripTrailingSpaces(TStringsW* sl);
  WideString __fastcall StripTrailingSpaces(WideString S);
  WideString __fastcall StripLTspaces(WideString S);
  bool __fastcall StripLTspaces(wchar_t* &pBuf, int &iSize);
  WideString __fastcall StripFont(WideString s, int iStart, int &iEnd, wchar_t c);
  void __fastcall StripFont(wchar_t* &pBuf, int &iSize, int iStart, int &iEnd, wchar_t c);
  WideString __fastcall StripColorCodes(WideString S, int iStart, int &iEnd);
  void __fastcall StripColorCodes(wchar_t* &pBuf, int &iSize, int iStart, int &iEnd);
  WideString __fastcall StripFgCodes(WideString S, bool bIgnorePushPop=false);
  void __fastcall StripFgCodes(wchar_t* &pBuf, int &iSize, bool bIgnorePushPop=false);
  WideString __fastcall StripBgCodes(WideString S, bool bIgnorePushPop=false);
  void __fastcall StripBgCodes(wchar_t* &pBuf, int &iSize, bool bIgnorePushPop=false);
  WideString __fastcall StripAllCodes(WideString S);
  void __fastcall StripAllCodes(wchar_t* &pBuf, int &iSize);
  int __fastcall StripLastCrLfAndCodes(wchar_t* pBuf, int iSize);
  String __fastcall AnsiStripCRLF(String s);
  WideString __fastcall StripCRLF(WideString S);
  void __fastcall StripCRLF(wchar_t* &pBuf, int &iSize, int iStart=0);
  String __fastcall StripTrailingCRLFsA(String s);
  bool __fastcall StripTrailingCRLFs(wchar_t* &pBuf, int &iSize);
  WideString __fastcall StripTrailingCRLFs(WideString s);

  bool __fastcall FlattenTabs(wchar_t* &pBuf, int &iSize, int mode);
  WideString __fastcall FlattenTabs(WideString S, int mode);
  WideString __fastcall FlattenPushPop(WideString sIn);

  bool __fastcall PageBreaksToFormFeed(wchar_t* &pBuf, int &iSize);
  WideString __fastcall PageBreaksToFormFeed(WideString S);
  void __fastcall PageBreaksToRtf(TTaeRichEdit* re);
  bool __fastcall FormFeedToPageBreaks(wchar_t* &pBuf, int &iSize);
  WideString __fastcall FormFeedToPageBreaks(WideString S);

  void __fastcall ReplaceChar(wchar_t* &pBuf, int iSize,
                                                  wchar_t cOld, wchar_t cNew);
  WideString __fastcall ReplaceChar(WideString wIn,
                                                  wchar_t cOld, wchar_t cNew);

  bool __fastcall LongestWidthAndLineCount(wchar_t* pBuf, int iSize,
                                                    int &Width, int &Height);
  int __fastcall GetNextNonspaceRealIdx(wchar_t* pBuf, int iSize, int Idx);
  int __fastcall GetNextNonspaceRealIdx(WideString S, int Idx);
  int __fastcall GetLastNoncodeBufferIdx(wchar_t* pBuf, int iSize);
  int __fastcall GetLastNoncodeBufferIdx(WideString S);
  long __fastcall GetTextLength(TTaeRichEdit* re);
  int __fastcall GetLine(TTaeRichEdit* re);
  int __fastcall CountChar(WideString S, wchar_t Code);
  int __fastcall CountChar(wchar_t* pBuf, int iSize, wchar_t Code);
  int __fastcall CountCRs(WideString S);
//  int __fastcall CountSelectedCRs(WideString S);
  int __fastcall CountCRs(wchar_t* pBuf, int iSize);
  int __fastcall SkipCRLF(wchar_t* pBuf, int iSize, int iStart=0);
  bool __fastcall FoundCRLF(WideString S, int &ii, bool bNoInc=false);
  bool __fastcall FoundCRLF(wchar_t* buf, int iSize, int &ii, bool bNoInc=false);
  bool __fastcall StripTrailingCRLFsA(char* buf, int &iSize);
  int __fastcall CountTrailingCRLFs(WideString s);
  bool __fastcall SkipSmileys(int &idx, wchar_t buf[], WideString &TempStr, int Size);
  int __fastcall FindNextParagraph(int iIndex, int iCharsRead, wchar_t* pBuf);
  bool __fastcall IsNextPrintableSequence(wchar_t* pBuf, int ii, int iSize, WideString S);
  bool __fastcall IsAnySpace(WideString S);
  bool __fastcall IsAnySpace(wchar_t* pBuf, int iSize);
  bool __fastcall IsAllSpaces(WideString S);
  bool __fastcall IsAllSpaces(wchar_t* pBuf, int iSize);
  int __fastcall GetNumberOfLinesSelected(bool &bPartialSelection, TTaeRichEdit* re);
  int __fastcall GetNumberOfLinesSelected(bool &bPartialSelection);
  WideString __fastcall DeleteRealRange(WideString S, int iRealStart, int iRealLength);
  WideString __fastcall DeleteRealRange(wchar_t* pBuf, int iSize, int iRealStart, int iRealLength);
  bool __fastcall TextContainsFormatCodes(TStringsW* sl);
  bool __fastcall TextContainsFormatCodes(WideString S);
  bool __fastcall TextContainsFormatCodes(wchar_t* pBuf, int iSize);

  // Font
  int __fastcall CountFontSequence(wchar_t* pBuf, int ii, int iSize);
  WideString __fastcall FontSizeToString(int val);
  WideString __fastcall FontTypeToString(int val);
  String __fastcall FontSizeToStringA(int val);
  String __fastcall FontTypeToStringA(int val);
  //bool __fastcall WriteFont(int Size, WideString &FontStr);

  // Strip-mode search/replace dialog
  int __fastcall SelectStripMode(int InitMode=STRIP_ALL);

  // Colors
  int __fastcall PanelColorDialog(TPanel* p, WideString s, TColor c, int mode);
  bool __fastcall SetPanelColorAndCaption(TPanel* p, int c);
  void __fastcall ShowColor(TColor Color);
  void __fastcall ShowColor(int Color);
  bool __fastcall IsDarkColor(int color);
  int __fastcall ConvertColor(int color, bool bFullSpectrumRand);
  int __fastcall SelectCustomColor(WideString Topic, TColor InitialColor, int ID);
  int __fastcall CountColorSequence(wchar_t* pBuf, int ii, int iSize);
  int __fastcall CountColorSequence(wchar_t* pBuf, int ii, int iSize, int &fg, int &bg);
  // RGB
  int __fastcall GetRandomRGB(void);
//  bool __fastcall AddRGB(int c);
//  int __fastcall FindRGB(int c);
//  int __fastcall MakeRGBPalette(WideString S, bool bShowStatus);
//  int __fastcall MakeRGBPalette(wchar_t * pBuf, int iSize, bool bShowStatus);
//  void __fastcall MakeMediumPalette(void);
//  void __fastcall MakeSmallPalette(void);
//  int __fastcall GetRgbPaletteIndex(int c);
//  int __fastcall ResolveRGB(int Color);
//  int __fastcall ResolveRGB(BlendColor BC);
  int __fastcall ResolveRGBLocal(int Color);
  int __fastcall ResolveRGBLocal(BlendColor BC);
  int __fastcall ResolveRGBExact(TColor c);
  int __fastcall ResolveRGBExact(int c);

  void __fastcall WritePrevBlendColors(int PrevFG, int PrevBG, WideString &TempStr);

  int __fastcall WriteColors(int fg, int bg, WideString &S);
  int __fastcall WriteColorsA(int fg, int bg, AnsiString &S);
  AnsiString __fastcall GetColorStringA(int fg, int bg);

  int __fastcall WriteSingle(int cl, WideString &S, bool bForeground);
  int __fastcall WriteSingleA(int cl, AnsiString &S, bool bForeground);
  int __fastcall WriteSingle(int cl, wchar_t* pBuf, bool bForeground);
  AnsiString __fastcall GetSingleColorStringA(int cl, bool bForeground);

  bool __fastcall GetTaeEditColors(int &fg, int &bg, bool bAtCaret);
  bool __fastcall GetTColorDialog(TColor &Color);
  bool __fastcall ValidIRC(void);
  bool __fastcall ValidORG(void);
  bool __fastcall IsIrcView(void);
  bool __fastcall IsOrgView(void);
  bool __fastcall IsRtfView(void);
  bool __fastcall IsIrcOrgView(void);
  bool __fastcall IsRtfIrcView(void);
  bool __fastcall IsRtfIrcOrgView(void);

  void __fastcall InitPushPop(void);
  void __fastcall Pop(PUSHSTRUCT &p);
  void __fastcall Push(PUSHSTRUCT p);

  // Color conversions
  int __fastcall RgbToYc(int C);
  TColor __fastcall RgbToTColor(int C);
  BlendColor __fastcall RgbToBlendColor(int C);
  TColor __fastcall YcToTColor(int C);
  BlendColor __fastcall YcToBlendColor(int C);
  int __fastcall YcToRgb(int c);
  int __fastcall TColorToRgb(TColor C);
  int __fastcall TColorToYc(TColor C);
  BlendColor __fastcall TColorToBlendColor(TColor C);
  WideString __fastcall TColorToHex(TColor C);
  TColor __fastcall BlendColorToTColor(BlendColor C);
  int __fastcall BlendColorToRgb(BlendColor C);
  WideString __fastcall BlendColorToHex(BlendColor C);
  BlendColor __fastcall HexToBlendColor(WideString Hex);
  TColor __fastcall HexToTColor(WideString Hex);
  void __fastcall SetCustomColorsDialogFromPalette(TColorDialog* CD);
  bool __fastcall GetPaletteFromCustomColorsDialog(TColorDialog* CD);

  void __fastcall PopOnChange(TTaeRichEdit* re);
  void __fastcall PushOnChange(TTaeRichEdit* re);

  void __fastcall InitOnChange(TTaeRichEdit* re, TNotifyEvent oce);
//  void __fastcall InitOnChange(TTaeRichEdit* re, TTaeRichEditChange oce);
  void __fastcall SetLine(TTaeRichEdit* re, int idx);

  // The first two prototypes are just so we can pass "0" as a TMemoryStream*
  int __fastcall ConvertToRtf(TStringsW* slIn, TMemoryStream* &msOut);
  int __fastcall ConvertToRtf(WideString S, TMemoryStream* &msOut);
  String __fastcall ConvertToRtf(WideString S, int maxColors, int fg, int bg,
                           bool bStripFontType=false, bool bShowStatus=false);
  int __fastcall ConvertToRtf(TStringsW* slIn, int msOut, TTaeRichEdit* re,
                    bool bSetBlackWhiteColors=false, bool bShowStatus=false);
  int __fastcall ConvertToRtf(TStringsW* slIn, TMemoryStream* &msOut,
    TTaeRichEdit* re, bool bSetBlackWhiteColors=false, bool bShowStatus=false);
  int __fastcall ConvertToRtf(WideString S, int msOut, TTaeRichEdit* re,
                  bool bSetBlackWhiteColors=false, bool bShowStatus=false);
  int __fastcall ConvertToRtf(WideString S, TMemoryStream* &msOut,
    TTaeRichEdit* re, bool bSetBlackWhiteColors=false, bool bShowStatus=false);
  WideString __fastcall GetSpecialFolder(int csidl);
  String __fastcall GetDeskDir(void);

  // WideString Functions
  int __fastcall ShowMessageU(HWND hwnd, String sUtf8, int flags=0);
  int __fastcall ShowMessageU(String sUtf8, int flags=0);
  int __fastcall ShowMessageW(HWND hwnd, WideString sIn, int flags=0);
  int __fastcall ShowMessageW(WideString wIn, int flags=0);
//  WideString __fastcall ReplaceAllW(WideString sIn, WideString S1, WideString S2);
//  WideString __fastcall ReplaceAllW(String sIn, wchar_t c1, wchar_t c2);
  WideString __fastcall UpperCaseW(WideString s);
  WideString __fastcall LowerCaseW(WideString s);
  void __fastcall ReplaceW(TStringsW* sl, WideString wNew, int startIdx);
  WideString __fastcall ReplaceW(WideString wIn, WideString wNew,
                                                              int startIdx);
  WideString __fastcall InsertW(WideString wDest, WideString wSource, int pos);
  void __fastcall DeleteW(TStringsW* sl, int startIdx, int numChars);
  WideString __fastcall DeleteW(WideString wIn, int startIdx, int numChars);
  wchar_t* __fastcall StrNewW(WideString sIn);
  wchar_t* __fastcall StrNewW(wchar_t* pIn);
  wchar_t* __fastcall StrNewW(wchar_t* pIn, int len);
  int __fastcall ToIntDefW(WideString s, int def);
  WideString __fastcall TrimW(WideString s);
  WideString __fastcall StringOfCharW(WideChar c, int count);
  String __fastcall GetTabString(int count);
  WideString __fastcall GetTabStringW(int count);

  // WideString file-functions
  bool __fastcall FileExistsW(WideString wPath);
  bool __fastcall DirectoryExistsW(WideString wPath);
  bool __fastcall SetCurrentDirW(WideString wDir);
  WideString __fastcall ExtractFileExtW(WideString sIn);
  WideString __fastcall ExtractFileNameW(WideString sIn);
  WideString __fastcall ExtractFilePathW(WideString sIn);
  String __fastcall ReadStringFromFileW(WideString wPath); // Reads UTF-8 file with wide path into string
  bool __fastcall WriteStringToFileW(WideString wPath, String sInfo);
  WideString __fastcall GetExeNameW(void);
  String __fastcall GetAnsiPathW(WideString wPath);
  WideString __fastcall FindFileW(WideString wFile, WideString wPath);
  WideString __fastcall GetTempFileW(WideString FileName);

  // UTF-8 Conversion
  String __fastcall WideToUtf8(WideString sIn);
  WideString __fastcall Utf8ToWide(char* buf);
  WideString __fastcall Utf8ToWide(char* buf, int len);
  WideString __fastcall Utf8ToWide(String sIn);
  String __fastcall Utf8ToAnsi(String sIn);
  String __fastcall AnsiToUtf8(String sIn);

  // Used to determine if a dialog occludes selected text in the main edit
  // control and relocate the dialog...
  void __fastcall RelocateDialog(TForm* f);

  AnsiString __fastcall GetLocalFontStringA(int idx);
  WideString __fastcall GetLocalFontStringW(int idx);
  String __fastcall GetLocalFontString(int idx); // utf-8
  int __fastcall GetLocalFontIndex(WideString sFont);

  bool __fastcall mishexw(wchar_t c);
  bool __fastcall mishex(char c);

  // Save file dialog...
  WideString __fastcall GetSaveFileName(WideString &wFilters, WideString wFile,
                                          WideString wDir, String uTitle);
  // Open file dialog...
  WideString __fastcall GetOpenFileName(WideString &wFilters, int iFilter,
                                         WideString wDir, String uTitle);

  long __fastcall filesize(FILE *fs);

  TIniFile* __fastcall OpenIniFile(WideString wFilePath="", bool bShow=true);
};
//---------------------------------------------------------------------------
extern TUtils* utils;

typedef HRESULT __stdcall (*tGetFolderPath)(HWND hwndOwner, int nFolder,
    HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);
//---------------------------------------------------------------------------
#endif
