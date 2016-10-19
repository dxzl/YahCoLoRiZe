//---------------------------------------------------------------------------
#ifndef ConvertToYahooH
#define ConvertToYahooH
//---------------------------------------------------------------------------

// Note: "backtick" char is <colorfast> in YahELite
#define YAHEXTENDSTR (WideString("/extend T"))

#define YAHBASICCOLOR(c) (WideString('\033') + WideString("[3") + WideString(c) + WideString("m"))

#define YAHINITSTR(f) (WideString("<font face=\"") + WideString(f) + WideString("\">"))

#define YAHCOLORINITSTR(f) (WideString("<font face=\"") + WideString(f) + WideString(L"\" bgcolor=") +\
YahELiteColor(DTSColor->Abg, false) + WideString(">"))

#define YAHFONTSIZE(s) (WideString("<font size=\"") + WideString(s) + WideString("\">"))
#define YAHFONTTYPE(s) (WideString("<font face=\"") + WideString(s) + WideString("\">"))

#define YAH_RGB(s) (WideString("\033[#") + WideString(s) + WideString("m")) // where s is RRGGBB, 2 digit hex 00 to FF
#define YAH_COLOR(c) (WideString("\033[3") + WideString(c) + WideString("m")) // Selects a color, c is 0-9

// Yahoo codes
#define YAH_x ("\033[m") // Resets ??? attributes.
#define YAH_X ("\033[0m") // Resets all attributes.
#define YAH_B_ON ("\033[1m") // Bold
#define YAH_B_OFF ("\033[x1m") // unset bold.
#define YAH_I_ON ("\033[2m") // Italic
#define YAH_I_OFF ("\033[x2m") // unset italic.
#define YAH_U_ON ("\033[4m") // Underline
#define YAH_U_OFF ("\033[x4m") // unset underlne.

//---------------------------------------------------------------------------

class TConvertToYahoo
{
  friend class TDTSColor;

private:
  WideString  __fastcall ConvertToYahooColorString(int c);
  WideString __fastcall YahELiteColor(int Color, bool StrType);

  int PrevFg;
  
  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TConvertToYahoo(TComponent* Owner);
  int __fastcall Execute(bool bShowStatus);
  WideString __fastcall ConvertLineToYahoo(WideString sIn);

  WideString  __fastcall BoldOnOff(bool bAllowOn, bool &bBoldOn);
  WideString  __fastcall UnderlineOnOff(bool bAllowOn, bool &bUnderlineOn);
  WideString  __fastcall ItalicsOnOff(bool bAllowOn, bool &bItalicsOn);
};
//---------------------------------------------------------------------------
 extern TConvertToYahoo* ConvertToYahoo;
//---------------------------------------------------------------------------
#endif
