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

#include "DlgFgBgColors.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFgBgColorsForm *FgBgColorsForm;
//---------------------------------------------------------------------------
__fastcall TFgBgColorsForm::TFgBgColorsForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  fgColor = NO_COLOR;
  bgColor = NO_COLOR;
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::FormShow(TObject *Sender)
{
  fgColor = utils->ConvertColor(dts->Foreground, false);
  SetColors2FgPanel->Color = utils->YcToTColor(fgColor);
  SetColors2FgPanel->Caption = "";
  bgColor = utils->ConvertColor(dts->Background, false);
  SetColors2BgPanel->Color = utils->YcToTColor(bgColor);
  SetColors2BgPanel->Caption = "";
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::SetColors2FgPanelMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(SetColors2FgPanel, COLORDIALOGMSG[10],
                                                      clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      fgColor = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::SetColors2BgPanelMouseDown(TObject *Sender,
                          TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(SetColors2BgPanel, COLORDIALOGMSG[11],
                                                        clAqua, COLOR_FORM_EBG);
    if (C != IRCCANCEL)
      bgColor = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::SetFgColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(SetColors2FgPanel, val))
    fgColor = val;
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::SetBgColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(SetColors2BgPanel, val))
    bgColor = val;
}
//---------------------------------------------------------------------------
void __fastcall TFgBgColorsForm::FormKeyDown(TObject *Sender, WORD &Key,
                                                            TShiftState Shift)
{
  if (Key == VK_ESCAPE)
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
