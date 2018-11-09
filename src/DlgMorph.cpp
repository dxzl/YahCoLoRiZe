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

#include "DlgMorph.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMorphForm *MorphForm;
//---------------------------------------------------------------------------
__fastcall TMorphForm::TMorphForm(TComponent* Owner)
  : TForm(Owner)
{
  // Set defaults
  fgChecked = true;
  bgChecked = true;

  fgFromColor = 0; // black
  fgToColor = -0xffffff; // white

  bgFromColor = -0xffffff; // white
  bgToColor = 0; // black

  maxColors = -1; // no limit

  FTitle = "Morph";
  FHideMaxColors = false;
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::FormShow(TObject *Sender)
{
  // Set form's controls to the property-values
  FgCheckBox->Checked = fgChecked;
  BgCheckBox->Checked = bgChecked;

  // Init with a color crossfade if no properties were preset...
  if (fgFromColor == NO_COLOR)
    FgFromColor = DTSColor->Afg;
  if (bgFromColor == NO_COLOR)
    BgFromColor = DTSColor->Abg;
  if (fgToColor == NO_COLOR)
    FgToColor = DTSColor->Abg;
  if (bgToColor == NO_COLOR)
    BgToColor = DTSColor->Afg;

  MaxColorsEdit->Visible = !FHideMaxColors;
  MaxColorsUpDown->Visible = !FHideMaxColors;
  MaxColorsLabel->Visible = !FHideMaxColors;

  MaxColorsUpDown->Position = (short)maxColors;

  this->Caption = FTitle;
  this->MorphGroupBox->Caption = FTitle;
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  // Set properties to current control values
  fgChecked = FgCheckBox->Checked;
  bgChecked = BgCheckBox->Checked;
  maxColors = MaxColorsUpDown->Position;
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::MorphFgFromPanelMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Right-click opens DTSColor->ColorCopyPasteMenu
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(MorphFgFromPanel, COLORDIALOGMSG[10],
                                                    clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      fgFromColor = utils->YcToRgb(C);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::MorphBgFromPanelMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Right-click opens DTSColor->ColorCopyPasteMenu
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(MorphBgFromPanel, COLORDIALOGMSG[11],
                                                    clAqua, COLOR_FORM_EBG);
    if (C != IRCCANCEL)
      bgFromColor = utils->YcToRgb(C);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::MorphFgToPanelMouseDown(TObject *Sender,
                    TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Right-click opens DTSColor->ColorCopyPasteMenu
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(MorphFgToPanel, COLORDIALOGMSG[10],
                                                  clAqua, COLOR_FORM_EFG);
    if (C != IRCCANCEL)
      fgToColor = utils->YcToRgb(C);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::MorphBgToPanelMouseDown(TObject *Sender,
                        TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Right-click opens DTSColor->ColorCopyPasteMenu
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(MorphBgToPanel, COLORDIALOGMSG[11],
                                                  clAqua, COLOR_FORM_EBG);
    if (C != IRCCANCEL)
      bgToColor = utils->YcToRgb(C);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::SetFgFromColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(MorphFgFromPanel, val))
    fgFromColor = utils->YcToRgb(val);
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::SetBgFromColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(MorphBgFromPanel, val))
    bgFromColor = utils->YcToRgb(val);
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::SetFgToColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(MorphFgToPanel, val))
    fgToColor = utils->YcToRgb(val);
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::SetBgToColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(MorphBgToPanel, val))
    bgToColor = utils->YcToRgb(val);
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::FgCheckBoxClick(TObject *Sender)
{
  if (FgCheckBox->Checked)
  {
    MorphFgFromPanel->Enabled = true;
    MorphFgToPanel->Enabled = true;
  }
  else
  {
    MorphFgFromPanel->Enabled = false;
    MorphFgToPanel->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::BgCheckBoxClick(TObject *Sender)
{
  if (BgCheckBox->Checked)
  {
    MorphBgFromPanel->Enabled = true;
    MorphBgToPanel->Enabled = true;
  }
  else
  {
    MorphBgFromPanel->Enabled = false;
    MorphBgToPanel->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMorphForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
  if (Key == VK_ESCAPE)
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

