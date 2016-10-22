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

#include "DlgMonkey.h"
#include "LicenseKey.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMonkeyForm *MonkeyForm;
//---------------------------------------------------------------------------
__fastcall TMonkeyForm::TMonkeyForm(TComponent* Owner)
  : TForm(Owner)
{
  xInc = (double)this->Width/100.;
  yInc = (double)this->Height/100.;

  EndRight = DTSColor->Left + DTSColor->Width - this->Width;
  if (EndRight < Width) EndRight = Width;
  if (EndRight > Screen->Width - this->Width) EndRight = Screen->Width - this->Width;
  EndBottom = DTSColor->Top+DTSColor->Height - this->Height;
  if (EndBottom < Height) EndBottom = Height;

  Left = DTSColor->Left;
  if (Left < 0) Left = 0;
  left = (double)Left;
  Top = DTSColor->Top;
  if (Top < 0) Top = 0;

  top = (double)Top;

  DTSColor->Freeze = true; // freeze title-bar color-changes

  RevPanel->Caption = String(DS[177]) + String(REVISION) + "." +
      String(DEF_PRODUCT_ID) + "." + String(DEF_SUPER_REV);

  // Set top panel's BG color the same as the main fg/bg colors
  RevPanel->Color = utils->YcToTColor(DTSColor->Background);
  RevPanel->Font->Color = utils->YcToTColor(DTSColor->Foreground);

  bReverse = false;

  Timer1->Interval = 10;
  Timer1->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TMonkeyForm::Image1Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TMonkeyForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_RETURN || Key == VK_ESCAPE)
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TMonkeyForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  DTSColor->Freeze = false; // un-freeze title-bar color-changes
  Release();
}
//---------------------------------------------------------------------------

void __fastcall TMonkeyForm::Timer1Timer(TObject *Sender)
{
  // Timer scrolls form over YahCoLoRiZe

  if (bReverse)
  {
    left -= 2*xInc;
    if (left <= (double)DTSColor->Left)
    {
      Timer1->Enabled = false;
      Close();
    }
    else
    {
      top -= 2*yInc;
      if (top > (double)DTSColor->Top)
        this->Top = (int)top;
      else
      {
        Timer1->Enabled = false;
        Close();
      }
    }
  }
  else if (this->Left > EndRight ||
           this->Top > DTSColor->Top + DTSColor->Height ||
           this->Top > Screen->Height - this->Height)
    bReverse = true;
  else
  {
    left += xInc;
    this->Left = (int)left;
    top += yInc;
    this->Top = (int)top;
  }
}
//---------------------------------------------------------------------------

