//---------------------------------------------------------------------------
#ifndef ConvertFromHTMLH
#define ConvertFromHTMLH
//---------------------------------------------------------------------------
#ifndef CHARSET
#define CHARSET "UTF-8"
#endif

#define TABSTRING ("    ") // 4 spaces per tab

class TConvertFromHTML
{
private:
  int __fastcall SkipColonOrEqual(int pos, WideString wIn);
  wchar_t __fastcall HtmlSpecialCharDecode(WideString wIn);
  WideString __fastcall ReplaceSpecial(WideString wIn);
  int __fastcall GetRgb(WideString S);
  WideString __fastcall GetFontSizeAndType(WideString wIn, float &size);
  WideString __fastcall ParseArg(int &pos, WideString wTag);
  bool __fastcall HtmlToIrcFontSize(WideString wIn, WideString &wOut);
  bool __fastcall HtmlToIrcPageBreak(WideString wIn, WideString &wOut);
  bool __fastcall ProcessTag(WideString wTag, WideString &wOut);
  WideString __fastcall GetTag(int &pos, WideString &OutText);
  WideString __fastcall GetTag(int &pos);
  bool __fastcall GetBody(String sRaw);

  WideString fileName, G_URL, G_In, G_InLC;

  unsigned short cbHTML; // Registered clipboard format
  int HTMLCHARSLEN, G_InLen;
  bool bHaveURL, bPreOn;

  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TConvertFromHTML(TComponent* Owner, WideString fName = "");
  __fastcall ~TConvertFromHTML(void);

  WideString __fastcall Execute(bool bShowStatus);

  __property WideString FileName = {read = fileName};
};
//---------------------------------------------------------------------------
#endif
