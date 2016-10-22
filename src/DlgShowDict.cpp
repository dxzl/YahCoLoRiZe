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

#include "DlgShowDict.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TShowDictForm *ShowDictForm;
//---------------------------------------------------------------------------
__fastcall TShowDictForm::TShowDictForm(TComponent* Owner)
  : TForm(Owner)
{
  // Load the listbox
  for (int ii = 0 ; ii < DTSColor->GDictList->Count; ii++)
    DictListBox->Items->Add(DTSColor->GDictList->Strings[ii]);

  OriginalItemCount = DictListBox->Items->Count;

  bItemsDeleted = false;

  this->Caption = SPELLINGMSG[1]; // My Dictionary
  DelButton->Caption = SPELLINGMSG[18];
  HelpButton->Caption = SPELLINGMSG[19];
  CloseButton->Caption = SPELLINGMSG[20];

  DelButton->Hint = SPELLINGMSG[40];
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::FormShow(TObject *Sender)
{
  if (DictListBox->Items->Count > 0)
  {
    DictListBox->Selected[0] = true;
    DictListBox->ItemIndex = 0;
  }

  DictListBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::DelButtonClick(TObject *Sender)
{
  int LastDeletedIdx = -1;

  // delete backward to avoid an index-conflict
  if (DictListBox->Items->Count > 0)
  {
    for (int ii = DictListBox->Items->Count-1; ii >= 0 ; ii--)
    {
      if (DictListBox->Selected[ii])
      {
        DictListBox->Items->Delete(ii);
        LastDeletedIdx = ii;
      }
    }
  }

  // Count still > 0 after delete?
  if (DictListBox->Items->Count > 0)
  {
    if (LastDeletedIdx >= DictListBox->Items->Count)
      LastDeletedIdx = DictListBox->Items->Count-1;

    if (LastDeletedIdx >= 0)
      DictListBox->Selected[LastDeletedIdx] = true;
    else
      DictListBox->Selected[0] = true;
  }
  else
    DictListBox->ItemIndex = -1; // off


  DictListBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::CloseButtonClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  try
  {
    if (DictListBox->Items->Count < OriginalItemCount)
    {
      DTSColor->GDictList->Clear();

      // Add the current DictListBox items (automatically sorts as we Add...)
      for (int ii = 0 ; ii < DictListBox->Items->Count; ii++)
        DTSColor->GDictList->Add(DictListBox->Items->Strings[ii]);

      bItemsDeleted = true;

      // set global list-changed flag
      if (!DTSColor->MyDictChanged) DTSColor->MyDictChanged = true;
  }
  }
  catch(...)
  {
    ShowMessage("Unable to update MyDictionary!");
  }
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::HelpButtonClick(TObject *Sender)
{
  utils->ShowMessageU(SPELLINGMSG[46]);
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::DictListBoxClick(TObject *Sender)
{
   if (DictListBox->SelCount > 0 && !DelButton->Default)
   {
     DelButton->Default = true;
     CloseButton->Default = false;
   }
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::DictListBoxKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
  if (Key == VK_DELETE)
    DelButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TShowDictForm::DictListBoxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{
  if (!DictListBox->Canvas->TryLock())
    return;

  WideString sOut;

  String sIn = DictListBox->Items->Strings[Index];

  if (!sIn.IsEmpty())
    sOut = utils->Utf8ToWide(sIn);

  ::TextOutW(DictListBox->Canvas->Handle, Rect.Left, Rect.Top,
                                                sOut.c_bstr(), sOut.Length());

  if (State.Contains(odFocused))
    DictListBox->Canvas->DrawFocusRect(DictListBox->ItemRect(Index));

  DictListBox->Canvas->Unlock();
}
//---------------------------------------------------------------------------

