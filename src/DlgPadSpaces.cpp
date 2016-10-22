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
#pragma hdrstop

#include "DlgPadSpaces.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPadSpacesForm *PadSpacesForm;
//---------------------------------------------------------------------------
__fastcall TPadSpacesForm::TPadSpacesForm(TComponent* Owner)
  : TForm(Owner)
{
  FAuto = true;
  FMode = PS_PAD_R;
  FWidth = 0;
  MaskEditWidth->Enabled = false;
  this->Caption = "Pad Spaces";
}
//---------------------------------------------------------------------------
void __fastcall TPadSpacesForm::FormShow(TObject *Sender)
{
  CheckBoxAutoWidth->Checked = FAuto;
  RadioGroupMode->ItemIndex = FMode;
}
//---------------------------------------------------------------------------
void __fastcall TPadSpacesForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  FMode = RadioGroupMode->ItemIndex;
}
//---------------------------------------------------------------------------
void __fastcall TPadSpacesForm::CheckBoxAutoWidthClick(TObject *Sender)
{
  FAuto = CheckBoxAutoWidth->Checked;
  MaskEditWidth->Enabled = FAuto;
}
//---------------------------------------------------------------------------

