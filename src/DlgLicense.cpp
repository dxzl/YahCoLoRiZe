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

#include "DlgLicense.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLicenseForm *LicenseForm;
//---------------------------------------------------------------------------
__fastcall TLicenseForm::TLicenseForm(TComponent* Owner)
  : TForm(Owner)
{
  Color = TColor(0xF5CFB8);
}
//---------------------------------------------------------------------------

void __fastcall TLicenseForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if ( Key == VK_ESCAPE )
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
