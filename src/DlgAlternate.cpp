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

#include "DlgAlternate.h"
#include "DlgIncDec.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAlternateForm *AlternateForm;
//---------------------------------------------------------------------------
__fastcall TAlternateForm::TAlternateForm(TComponent* Owner)
  : TForm(Owner)
{
  // Init local property vars - pick safe colors
  colorA = DTSColor->Afg;
  colorB = DTSColor->Abg;

  mode = EM_FGBG;
  bSkipSpaces = true;
  bShowColors = true; // Show the color-panels
  dlgCaption = "Alternate Colors";
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::FormShow(TObject *Sender)
{
  this->Caption = DlgCaption;

  if (mode != EM_FGBG)
  {
    colorB = random(DTSColor->PaletteSize)+1;
    while (colorB == colorA) colorB = random(DTSColor->PaletteSize)+1;
  }

  if (bShowColors)
  {
    AlternatePanelColorA->Color = utils->YcToTColor(colorA);
    AlternatePanelColorA->Visible = true;
    LabelColorA->Visible = true;
    AlternatePanelColorB->Color = utils->YcToTColor(colorB);
    AlternatePanelColorB->Visible = true;
    LabelColorB->Visible = true;
  }
  else
  {
    AlternatePanelColorA->Visible = false;
    LabelColorA->Visible = false;
    AlternatePanelColorB->Visible = false;
    LabelColorB->Visible = false;
  }

  RadioGroup->ItemIndex = mode; // Process Fg, Bg or Both?
  SkipSpacesCheckBox->Checked = bSkipSpaces;
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  mode = RadioGroup->ItemIndex;
  bSkipSpaces = SkipSpacesCheckBox->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::AlternatePanelColorAMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (!bShowColors)
    return;

  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(AlternatePanelColorA, COLORDIALOGMSG[12],
                                                        clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      colorA = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::AlternatePanelColorBMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (!bShowColors)
    return;

  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(AlternatePanelColorB, COLORDIALOGMSG[12],
                                                      clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      colorB = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::SetColorA(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(AlternatePanelColorA, val)) colorA = val;
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::SetColorB(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(AlternatePanelColorB, val)) colorB = val;
}
//---------------------------------------------------------------------------
void __fastcall TAlternateForm::FormKeyDown(TObject *Sender, WORD &Key,
                                                          TShiftState Shift)
{
  if (Key == VK_RETURN || Key == VK_ESCAPE)
  {
    Key = 0;
    Close();
  }
}
//---------------------------------------------------------------------------

