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

#include "DlgFontSize.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFontSizeForm *FontSizeForm;
//---------------------------------------------------------------------------
__fastcall TFontSizeForm::TFontSizeForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  fontSize = dts->cSize;
  Label3->Caption = "*" + String(TrackBar1->Position) + "*";
}
//---------------------------------------------------------------------------
void __fastcall TFontSizeForm::FormShow(TObject *Sender)
{
  TrackBar1->Position = fontSize;
}
//---------------------------------------------------------------------------
void __fastcall TFontSizeForm::CheckBox1Click(TObject *Sender)
{
  if (CheckBox1->Checked) TrackBar1->Enabled = false;
  else TrackBar1->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TFontSizeForm::TrackBar1Change(TObject *Sender)
{
  String Temp;
  if (TrackBar1->Position < 10) Temp = "0";
  Temp += String(TrackBar1->Position);
  Label3->Caption = "*" + Temp + "*";
}
//---------------------------------------------------------------------------
void __fastcall TFontSizeForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
  if (Key == VK_ESCAPE) ModalResult = mrCancel; // initiate Close()
}
//---------------------------------------------------------------------------
void __fastcall TFontSizeForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  if (CheckBox1->Checked)
    fontSize = 0;
  else
    fontSize = TrackBar1->Position;
}
//---------------------------------------------------------------------------

