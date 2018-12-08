//---------------------------------------------------------------------------
#ifndef ConvertToRTFH
#define ConvertToRTFH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------

#define DEF_FONT "Arial"
#define DEF_FONT_NUM 0
#define RESET_HIGHLIGHT_STR "\\highlight0"

class TConvertToRTF
{
private:
  String __fastcall TerminateLine(String sLine, PUSHSTRUCT State);
  int __fastcall AddColor(int C, TList* cList);
  int __fastcall AddOrResolveColor(int C, TList* cList);
  int __fastcall FindColorIndex(int C, TList* cList);
  int __fastcall ResolveColor(int C, TList* cList);
  String __fastcall RgbToRtfColorTable(TList* cList);
  String __fastcall YcToRtfColorString(int C);

  TStringsW* __fastcall CreateFontList(wchar_t* buf, int size);
  String __fastcall GetFontTable(TStringsW* slRtfFonts);
  int __fastcall Convert(wchar_t* buf, int size,
               TMemoryStream* msSource, TStringsW* slRtfFonts);
  int __fastcall WriteRtfToStream(String sBody, String ColorTable,
                                        String FontTable, TMemoryStream* ms);

  // properties
  int m_defaultFgColor, m_defaultBgColor, m_linesInHeader, m_maxColors, m_tabTwips;
  bool m_bShowStatus, m_bStripFontType, m_bStripFontSize;

  TTaeRichEdit* re;

  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TConvertToRTF(TComponent* Owner, TTaeRichEdit* re,
                                                bool bShowStatus);
  __fastcall ~TConvertToRTF(void);

  TMemoryStream* __fastcall Execute(wchar_t* buf, int size, int &retVal);
  TMemoryStream* __fastcall Execute(WideString S, int &retVal);
  TMemoryStream* __fastcall Execute(TMemoryStream* msIn, int &retVal);
  TMemoryStream* __fastcall Execute(TStringsW* sl, int &retVal);

  __property bool StripFontType = {read = m_bStripFontType, write = m_bStripFontType};
  __property bool StripFontSize = {read = m_bStripFontSize, write = m_bStripFontSize};

  __property int DefaultFgColor = {read = m_defaultFgColor, write = m_defaultFgColor};
  __property int DefaultBgColor = {read = m_defaultBgColor, write = m_defaultBgColor};

  // Reports back the header's # lines so we can position the cursor on
  // the appropriare line for the previous view
  __property int LinesInHeader = {read = m_linesInHeader};

  // Ran into a max of 7 colors for PalTalk so set this to MaxColors
  // (-1 is no-limit)
  __property int MaxColors = {read = m_maxColors, write = m_maxColors};
  __property int TabTwips = {read = m_tabTwips, write = m_tabTwips};
};
//---------------------------------------------------------------------------
#endif
