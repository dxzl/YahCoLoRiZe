//---------------------------------------------------------------------------
#ifndef ConvertToHTMLH
#define ConvertToHTMLH
//---------------------------------------------------------------------------
#include "Main.h"
//---------------------------------------------------------------------------

#define BUTTON_FONT_SIZE "12pt"
#define HOME_BUTTON_TEXT "HOME"
//#define HTML_GENERATOR "YahCoLoRiZe 7.17.2.3"
//#define CHARSET "iso-8859-1"
#define CHARSET "UTF-8"

// Bg Image Styles
#define REPEAT_ON  0
#define REPEAT_X   1
#define REPEAT_Y   2
#define REPEAT_OFF 3

// White-Space Styles
#define WS_NORMAL  0
#define WS_NOWRAP  1
#define WS_PRE     2
#define WS_PREWRAP 3
#define WS_PRELINE 4
#define WS_INHERIT 5
#define WS_INITIAL 6
#define WS_UNSET   7

#define NON_BREAKING_SPACE "&nbsp;"

#define STARTHTML "<!DOCTYPE HTML>\r\n<html>\r\n"
#define STARTFRAGMENT "<!--StartFragment-->"
#define ENDFRAGMENT "<!--EndFragment-->"
#define FRAGMENTHEADER "Version:0.9\r\nStartHTML:33333333\r\n"\
 "EndHTML:44444444\r\nStartFragment:11111111\r\nEndFragment:22222222\r\n"
#define ENDHTML "\r\n</body>\r\n</html>"

class TConvertToHTML
{
private:
  void __fastcall WriteSpan(WideString wIn);
  WideString __fastcall HtmlSpecialCharEncode(wchar_t c);
  void __fastcall CopyHtmlToClipBoard(String htmlStr);
  String __fastcall FormatHeader(String HTMLText);
  String __fastcall FormatColor(int c);
  void __fastcall DoStyleSpan(int &Spans, String &sOut, char c);

  WideString FFile, wOut;

  bool bSourceBold, bSourceUnderline, bSourceItalics, bWroteFirstBold;
  bool bWroteFirstUnderline, bWroteFirstItalics, bDestBold, bDestUnderline;
  bool bDestItalics;

  unsigned short cbHTML; // Registered clipboard format

  int HTMLCHARSLEN, Spans;

  // byte offsets just after <!--StartFragment--> and just at start of the
  // <!--EndFragment--> marker. note: these are NOT character offsets!
  int GFragStart, GFragEnd, GHTMLStart, GHTMLEnd;

  TDTSColor* dts; // easy pointer to main form...

  // Vars set after invoking DlgWebExport.cpp
  int leftMargin, topMargin, whiteSpaceStyle, bgImageStyle, bgColor;
  String bgImageUrl, homeButtonUrl, title, author;

  // Properties
  bool clip, savehtml, saveplain, savestream, nonbreakingspaces, showstatus;

public:
  __fastcall TConvertToHTML(TComponent* Owner, WideString fName = "");
  __fastcall ~TConvertToHTML(void);

  int __fastcall Convert(TStringsW* sl, bool bCreateDialog=true);
  int __fastcall Convert(WideString S, bool bCreateDialog=true);
  String __fastcall ConvertLineToHTML(WideString sIn);

  __property WideString FileName = {read = FFile};
  __property bool bClip = {read = clip, write = clip};

  // These determine which format(s) to write to the clipboard
  __property bool bShowStatus = {read = showstatus, write = showstatus};
  __property bool bWriteClipHtml = {read = savehtml, write = savehtml};
  __property bool bWriteClipPlain = {read = saveplain, write = saveplain};
  __property bool bWriteStream = {read = savestream, write = savestream};
  __property bool bNonBreakingSpaces = {read = nonbreakingspaces,
                                                  write = nonbreakingspaces};
};
//---------------------------------------------------------------------------
#endif
