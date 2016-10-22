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

#include "DlgUnderline.h"
#include "DlgIncDec.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TUnderlineForm *UnderlineForm;
//---------------------------------------------------------------------------
__fastcall TUnderlineForm::TUnderlineForm(TComponent* Owner)
  : TForm(Owner)
{
  // Init local property vars
  dlgColorA = DTSColor->Afg;
  dlgColorB = random(DTSColor->PaletteSize)+1;

  while (dlgColorB == dlgColorA)
    dlgColorB = random(DTSColor->PaletteSize)+1;

  mode = AL_ULCOLOR;
  dlgCaption = "Alternate Character";
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::FormShow(TObject *Sender)
{
  this->Caption = DlgCaption;
  UnderlinePanelColorA->Color = utils->YcToTColor(dlgColorA);
  UnderlinePanelColorB->Color = utils->YcToTColor(dlgColorB);
  RadioGroup->ItemIndex = mode;
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  mode = RadioGroup->ItemIndex;
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::UnderlinePanelColorAMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(UnderlinePanelColorA, COLORDIALOGMSG[17],
                                                        clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      dlgColorA = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::UnderlinePanelColorBMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(UnderlinePanelColorB, COLORDIALOGMSG[18],
                                                        clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      dlgColorB = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::SetColorA(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(UnderlinePanelColorA, val))
    dlgColorA = val;
}
//---------------------------------------------------------------------------
void __fastcall TUnderlineForm::SetColorB(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(UnderlinePanelColorB, val))
    dlgColorB = val;
}
//---------------------------------------------------------------------------

