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

#include "ColorDlg.h"
#include "LicenseKey.h"
#include "DlgAbout.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
  : TForm(Owner)
{
//  Panel2->Color = TColor(0x696969);
  RevLabel->Caption = String(REVISION) + "." + String(DEF_PRODUCT_ID) +
                            "." + String(DEF_SUPER_REV);
//  String Year = String(__DATE__);
  Label1->Caption = "Authored by:\nMister Swift\n" + String(OUR_COMPANY) +
                      "\n" + String(__DATE__);
#if LICENSE_ON
  EnterKeyButton->Enabled = true;
#else
  EnterKeyButton->Enabled = false;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::EnterKeyButtonClick(TObject *Sender)
{
  try
  {
#if LICENSE_ON
    TLicenseKey * lk = new TLicenseKey();
    bool bOK = lk->DoKey( false ); // don't terminate on Cancel

    if ( !bOK )
      bOK = lk->ValidateLicenseKey(false); // Re-evaluate old key

    delete lk;

    if ( bOK )
      DisplayDaysRemaining();
    else
      utils->ShowMessageU(KEYSTRINGS[18]);
#else
    DisplayDaysRemaining();
#endif
  }
  catch(...)
  {
    ShowMessage("Critical Error in About Dialog");
    this->Release();
    Application->Terminate();
  }
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::WebSiteClick(TObject *Sender)
{
  // launch default web-browser
  //ShellExecute(Handle, "open", "iexplore.exe",
  //                 Iftf->Strings[INFO_WEB_SITE].c_str(), NULL, SW_SHOW);
  ShellExecute(NULL, "open", DTSColor->Iftf->Strings[INFO_WEB_SITE].c_str(),
                                                  NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormShow(TObject *Sender)
{
  DisplayDaysRemaining();
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::DisplayDaysRemaining( void )
{
  String S = "YahCoLoRiZe ";

#if LICENSE_ON
  int DaysRem = PK->ComputeDaysRemaining();

  // -100 if failure
  if ( DaysRem >= 0 || DaysRem > LK_DISPLAY_UNLIMITTED_DAYS )
    {
    if ( DaysRem >= LK_DISPLAY_UNLIMITTED_DAYS )
      S += String(KEYSTRINGS[19]); // (Unlimited License)
    else if ( DaysRem == 0 )
      S += String(KEYSTRINGS[1]); // (Invalid License)
    else
      S += String(KEYSTRINGS[0]) + String(DaysRem); // License Days:
    }
  else
    S = String(KEYSTRINGS[1]); // (Invalid License)
#else
  S += String(KEYSTRINGS[19]); // (Unlimited License)
#endif

  Caption = S;
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if ( Key == VK_RETURN || Key == VK_ESCAPE )
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------

