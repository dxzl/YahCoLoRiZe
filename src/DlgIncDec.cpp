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

#include "DlgIncDec.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TIncDecForm *IncDecForm;
//---------------------------------------------------------------------------
__fastcall TIncDecForm::TIncDecForm(TComponent* Owner) : TForm(Owner)
{
  // Init local property vars
  mode = EM_FGBG;
  bEnaEdits = true;
  bSkipSpaces = false;

  redDelta = INCDEC_RED;
  greenDelta = INCDEC_GREEN;
  blueDelta = INCDEC_BLUE;

  this->paletteSize = DTSColor->PaletteSize; // 16 colors to be safe!
  offset = paletteSize/2;
}
//---------------------------------------------------------------------------
void __fastcall TIncDecForm::FormShow(TObject *Sender)
{
  UpDownRed->Position = (short)redDelta;
  UpDownGreen->Position = (short)greenDelta;
  UpDownBlue->Position = (short)blueDelta;

  UpDownOffset->Max = (short)(paletteSize-1);
  UpDownOffset->Position = (short)offset;

  RadioGroup->ItemIndex = mode; // Process Fg, Bg or Both?

  SkipSpacesCheckBox->Checked = bSkipSpaces;

  if ( bEnaEdits )
  {
    EditRed->Enabled = true;
    EditGreen->Enabled = true;
    EditBlue->Enabled = true;
  }
  else
  {
    EditRed->Enabled = false;
    EditGreen->Enabled = false;
    EditBlue->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TIncDecForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  redDelta = UpDownRed->Position;
  greenDelta = UpDownGreen->Position;
  blueDelta = UpDownBlue->Position;
  offset = UpDownOffset->Position;
  mode = RadioGroup->ItemIndex;
  bSkipSpaces = SkipSpacesCheckBox->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TIncDecForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
  if ( Key == VK_ESCAPE ) ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

