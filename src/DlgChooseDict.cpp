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

#include "DlgChooseDict.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TChooseDictForm *ChooseDictForm;
//---------------------------------------------------------------------------
__fastcall TChooseDictForm::TChooseDictForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  this->Caption = SPELLINGMSG[2]; // Choose Dictionary

  OkButton->Caption = SPELLINGMSG[21];

  // Dictionary(s) will be in the install directory Dict sub-directory...
  WideString DictPath = utils->ExtractFilePathW(utils->GetExeNameW()) +
                                               utils->Utf8ToWide(DICTDIR);

  //WideString DictPath = utils->GetSpecialFolder(CSIDL_APPDATA) +
  //        "\\" + utils->Utf8ToWide(OUR_COMPANY) + "\\" +
  //        utils->Utf8ToWide(OUR_NAME) + "\\" + utils->Utf8ToWide(DICTDIR);

  struct _wffblk sr;

  // sr.ff_name will have the filename and extension, without the path!
  if (_wfindfirst(WideString(DictPath + "*.dic").c_bstr(), &sr, 0) == 0)
  {
    CheckFile(DictPath, WideString(sr.ff_name));

    while (_wfindnext(&sr) == 0)
      CheckFile(DictPath, WideString(sr.ff_name));

    _wfindclose(&sr);
  }

  count = DictListBox->Items->Count; // set property-var...

  if (count == 0)
  {
    Close();
    return;
  }

  wDict = utils->Utf8ToWide(DictListBox->Items->Strings[DictListBox->ItemIndex]);
}
//---------------------------------------------------------------------------
void __fastcall TChooseDictForm::CheckFile(WideString wDictPath, WideString wFile)
{
  // Get just the file-name without the extension...
  int pos = wFile.Pos(utils->ExtractFileExtW(wFile));
  if (pos >= 0)
    wFile = wFile.SubString(1, pos-1);

  if (utils->FileExistsW(wDictPath + wFile + ".aff"))
  {
    DictListBox->Items->Add(utils->WideToUtf8(wFile)); // put utf-8 in the ansi control for now...

    if (utils->LowerCaseW(wFile) == utils->LowerCaseW(dts->GDictionary))
      DictListBox->ItemIndex = DictListBox->Items->Count-1;
  }
}
//---------------------------------------------------------------------------
void __fastcall TChooseDictForm::FormShow(TObject *Sender)
{
  DictListBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TChooseDictForm::OkButtonClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TChooseDictForm::DictListBoxClick(TObject *Sender)
{
  if (DictListBox->ItemIndex >= 0 &&
         DictListBox->ItemIndex < DictListBox->Items->Count)
    wDict = utils->Utf8ToWide(DictListBox->Items->Strings[DictListBox->ItemIndex]);
}
//---------------------------------------------------------------------------

void __fastcall TChooseDictForm::DictListBoxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{
  String sIn = DictListBox->Items->Strings[Index];

  if (!DictListBox->Canvas->TryLock())
    return;

  WideString sOut;

  if (!sIn.IsEmpty())
    sOut = utils->Utf8ToWide(sIn);

  ::TextOutW(DictListBox->Canvas->Handle, Rect.Left, Rect.Top,
                                                sOut.c_bstr(), sOut.Length());

  if (State.Contains(odFocused))
    DictListBox->Canvas->DrawFocusRect(DictListBox->ItemRect(Index));

  DictListBox->Canvas->Unlock();
}
//---------------------------------------------------------------------------

