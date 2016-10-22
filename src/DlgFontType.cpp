//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "DlgFontType.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFontTypeForm *FontTypeForm;
//---------------------------------------------------------------------------
__fastcall TFontTypeForm::TFontTypeForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  fontType = dts->cType;
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::FormShow(TObject *Sender)
{
  int iFont = fontType-1;
  if (iFont >= 0 && iFont <= FONTITEMS-1)
    FontTypeListBox->ItemIndex = FontTypeListBox->Items->IndexOf(FONTS[iFont]);
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::FormCreate(TObject *Sender)
{
  g_sl = new TStringList();
//  g_sl->Sorted = true;
//  g_sl->Duplicates = dupIgnore;

  PopulateListBox();
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::FormDestroy(TObject *Sender)
{
  if (g_sl != NULL)
    try {delete g_sl;} catch (...) {}
}
#if WIN32LISTPOPULATION
//---------------------------------------------------------------------------
// This Callback is only if we use EnumFontFamiliesEx!!!!!
// it must be located in front of PopulateListBoxWin32
// ENUMLOGFONTEX:
// LOGFONT elfLogFont;
// TCHAR   elfFullName[LF_FULLFACESIZE];
// TCHAR   elfStyle[LF_FACESIZE];
// TCHAR   elfScript[LF_FACESIZE];

// NEWTEXTMETRICEX:
// NEWTEXTMETRIC ntmTm;
// FONTSIGNATURE ntmFontSig;

// NEWTEXTMETRIC:
// LONG  tmHeight;
// LONG  tmAscent;
// LONG  tmDescent;
// LONG  tmInternalLeading;
// LONG  tmExternalLeading;
// LONG  tmAveCharWidth;
// LONG  tmMaxCharWidth;
// LONG  tmWeight;
// LONG  tmOverhang;
// LONG  tmDigitizedAspectX;
// LONG  tmDigitizedAspectY;
// TCHAR tmFirstChar;
// TCHAR tmLastChar;
// TCHAR tmDefaultChar;
// TCHAR tmBreakChar;
// BYTE  tmItalic;
// BYTE  tmUnderlined;
// BYTE  tmStruckOut;
// BYTE  tmPitchAndFamily;
// BYTE  tmCharSet;
// DWORD ntmFlags;
// UINT  ntmSizeEM;
// UINT  ntmCellHeight;
// UINT  ntmAvgWidth;
//
// FONTSIGNATURE
// DWORD fsUsb[4];
// DWORD fsCsb[2];
//
//fsUsb
//A 128-bit Unicode subset bitfield (USB) identifying up to 126 Unicode
// subranges. Each bit, except the two most significant bits, represents a
// single subrange. The most significant bit is always 1 and identifies the
// bitfield as a font signature; the second most significant bit is reserved
// and must be 0. Unicode subranges are numbered in accordance with the ISO 10646
// standard. For more information, see Unicode Subset Bitfields.
//fsCsb
// A 64-bit, code-page bitfield (CPB) that identifies a specific character
// set or code page. Code pages are in the lower 32 bits of this bitfield.
// The high 32 are used for non-Windows code pages. For more information,
// see Code Page Bitfields.
//

int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam);

int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam)
{
  TStringList* sl = (TStringList*)lParam;

  //(char*)lpelfe->elfStyle, (char*)lpelfe->elfScript
  if (sl != NULL)
    sl->Add(String((char*)lpelfe->elfFullName));

  return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::PopulateListBox(void)
{
  HDC hDC = GetDC(NULL);

  //DEFAULT_CHARSET
  //ANSI_CHARSET
  //ARABIC_CHARSET
  //BALTIC_CHARSET
  //CHINESEBIG5_CHARSET
  //DEFAULT_CHARSET
  //EASTEUROPE_CHARSET
  //GB2312_CHARSET
  //GREEK_CHARSET
  //HANGUEL_CHARSET
  //HANGUL_CHARSET
  //HEBREW_CHARSET
  //JOHAB_CHARSET
  //MAC_CHARSET
  //OEM_CHARSET
  //RUSSIAN_CHARSET
  //SHIFTJIS_CHARSET
  //SYMBOL_CHARSET
  //THAI_CHARSET
  //TURKISH_CHARSET
  //VIETNAMESE_CHARSET
  //
  // LOGFONT:
  // LONG  lfHeight;
  // LONG  lfWidth;
  // LONG  lfEscapement;
  // LONG  lfOrientation;
  // LONG  lfWeight;
  // BYTE  lfItalic;
  // BYTE  lfUnderline;
  // BYTE  lfStrikeOut;
  // BYTE  lfCharSet;
  // BYTE  lfOutPrecision;
  // BYTE  lfClipPrecision;
  // BYTE  lfQuality;
  // BYTE  lfPitchAndFamily;
  // TCHAR lfFaceName[LF_FACESIZE];
  //
  // lfCharSet If set to DEFAULT_CHARSET, the function enumerates all
  //   uniquely-named fonts in all character sets. (If there are two fonts
  //   with the same name, only one is enumerated.) If set to a valid character
  //   set value, the function enumerates only fonts in the specified
  //   character set.
  // lfFaceName If set to an empty string, the function enumerates one font in
  //   each available typeface name. If set to a valid typeface name, the
  //   function enumerates all fonts with the specified name.
  // lfPitchAndFamily Must be set to zero for all language versions of the
  //   operating system.
  LOGFONT lf = {0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, ""};
//  LOGFONT lf = {0, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, ""};

//  LOGFONT lf = {0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, "Courier New"};
//  EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, (LPARAM)FontTypeListBox);
  EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, (LPARAM)g_sl, 0);
  ReleaseDC(NULL, hDC);

//  if (g_sl != NULL) ShowMessage(g_sl->Count);
  FontTypeListBox->Items = g_sl;
}
#else
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::PopulateListBox(void)
{
  if (g_sl == NULL || FontTypeListBox == NULL)
    return;

  try
  {
    FontTypeListBox->Items->Clear();
    g_sl->Clear();

    for (int ii = 0; ii < FONTITEMS; ii++)
    {
      int idx = Screen->Fonts->IndexOf(String(FONTS[ii]));

      if (idx > 0)
      {
        FontTypeListBox->Items->Add(Screen->Fonts->Strings[idx]);
        g_sl->Add(FONTS[ii]); // add lower case version to string-list
      }
    }

    FontTypeListBox->ItemIndex = FontTypeListBox->Items->IndexOf(dts->cFont);

    if (FontTypeListBox->ItemIndex >= 0)
      fontType = FontTypeListBox->ItemIndex+1;
    else
      fontType = 1;
  }
  catch(...)
  {
    ShowMessage("Error in PopulateListBox()!");
  }
}
#endif
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
  if (Key == VK_ESCAPE) ModalResult = mrCancel; // initiate Close()
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  String sTemp;

  if (FontTypeListBox->ItemIndex >= 0 &&
             FontTypeListBox->ItemIndex < FontTypeListBox->Items->Count)
  {
    sTemp = FontTypeListBox->Items->Strings[FontTypeListBox->ItemIndex];
    fontType = utils->GetLocalFontIndex(sTemp);
  }
  else
    fontType = -1;

  if (fontType <= 0)
  {
    utils->ShowMessageU("Bad font-index!");
    fontType = 1;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFontTypeForm::Copy1Click(TObject *Sender)
{
  utils->CopyTextToClipboard(utils->Utf8ToWide(FontTypeListBox->Items->Text));
}
//---------------------------------------------------------------------------

