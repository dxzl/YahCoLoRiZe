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

#include "DlgSetColors.h"
#include "DlgIncDec.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSetColorsForm *SetColorsForm;
//---------------------------------------------------------------------------
__fastcall TSetColorsForm::TSetColorsForm(TComponent* Owner)
  : TForm(Owner)
{
  fgColor = NO_COLOR;
  bgColor = NO_COLOR;
  mode = EM_FGBG;
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::FormShow(TObject *Sender)
{
  RadioGroup->ItemIndex = mode;

  if (DTSColor->Foreground == IRCRANDOM)
  {
    fgColor = IRCRANDOM;
    SetColorsFgPanel->Color = clWhite;
    SetColorsFgPanel->Caption = "R";
  }
  else
  {
    fgColor = utils->ConvertColor(DTSColor->Foreground, false);
    SetColorsFgPanel->Color = utils->YcToTColor(fgColor);
    SetColorsFgPanel->Caption = "";
  }

  if (DTSColor->Background == IRCRANDOM)
  {
    bgColor = IRCRANDOM;
    SetColorsBgPanel->Color = clWhite;
    SetColorsBgPanel->Caption = "R";
  }
  else
  {
    bgColor = utils->ConvertColor(DTSColor->Background, false);
    SetColorsBgPanel->Color = utils->YcToTColor(bgColor);
    SetColorsBgPanel->Caption = "";
  }

  RadioGroupClick(NULL); // Set panel enable/disable
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::SetColorsFgPanelMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(SetColorsFgPanel, COLORDIALOGMSG[10],
                                                      clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      fgColor = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::SetColorsBgPanelMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(SetColorsBgPanel, COLORDIALOGMSG[11],
                                                        clAqua, COLOR_FORM_EBG);
    if (C != IRCCANCEL)
      bgColor = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::SetFgColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(SetColorsFgPanel, val))
    fgColor = val;
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::SetBgColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(SetColorsBgPanel, val))
    bgColor = val;
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::RadioGroupClick(TObject *Sender)
{
  if (RadioGroup->ItemIndex == EM_FGBG)
  {
    SetColorsFgPanel->Visible = true;
    SetColorsBgPanel->Visible = true;
    FgLabel->Visible = true;
    BgLabel->Visible = true;
    mode = EM_FGBG;
  }
  else if (RadioGroup->ItemIndex == EM_FG)
  {
    SetColorsFgPanel->Visible = true;
    SetColorsBgPanel->Visible = false;
    FgLabel->Visible = true;
    BgLabel->Visible = false;
    mode = EM_FG;
  }
  else
  {
    SetColorsFgPanel->Visible = false;
    SetColorsBgPanel->Visible = true;
    FgLabel->Visible = false;
    BgLabel->Visible = true;
    mode = EM_BG;
  }
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::FormClose(TObject *Sender,
                                                  TCloseAction &Action)
{
  if (ModalResult == mrOk)
    SaveSettings();
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::SaveSettings(void)
{
  mode = RadioGroup->ItemIndex;

  // Only one color? Set other to NO_COLOR...
  if (mode == EM_FG)
    bgColor = NO_COLOR;
  else if (mode == EM_BG)
    fgColor = NO_COLOR;
}
//---------------------------------------------------------------------------
void __fastcall TSetColorsForm::FormKeyDown(TObject *Sender, WORD &Key,
                                                          TShiftState Shift)
{
  if (Key == VK_ESCAPE)
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

