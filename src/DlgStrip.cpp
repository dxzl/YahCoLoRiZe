//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "DlgStrip.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma resource "*.dfm"

// STRIP_ALL           0
// STRIP_ALL_PRESERVE  1  // Keep codes within push/pop
// STRIP_BOLD          2
// STRIP_UNDERLINE     3
// STRIP_ITALICS       4
// STRIP_FG_COLOR      5
// STRIP_BG_COLOR      6
// STRIP_ALL_COLOR     7
// STRIP_FONT_CODES    8
// STRIP_PUSHPOP       9
// STRIP_CTRL_O        10
// STRIP_TAB           11
// STRIP_FF            12

TStripModeForm *StripModeForm;
//---------------------------------------------------------------------------

__fastcall TStripModeForm::TStripModeForm(TComponent* Owner)
  : TForm(Owner)
{
  Caption = STRIPDLG[0]; // "Strip"
  RadioGroup->Caption = STRIPDLG[1]; // "Strip Codes:"

  RadioGroup->Items->Clear();

  for (int ii = 0 ; ii < RADIO_COUNT; ii++)
    RadioGroup->Items->Add(STRIPDLG[ii+RADIO_OFFSET]);
}
//---------------------------------------------------------------------------

void __fastcall TStripModeForm::FormShow(TObject *Sender)
{
  RadioGroup->ItemIndex = stripMode;
}
//---------------------------------------------------------------------------

void __fastcall TStripModeForm::OkButtonClick(TObject *Sender)
{
  stripMode = RadioGroup->ItemIndex;
  ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TStripModeForm::CancelButtonClick(TObject *Sender)
{
  ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

void __fastcall TStripModeForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if ( Key == VK_ESCAPE )
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

