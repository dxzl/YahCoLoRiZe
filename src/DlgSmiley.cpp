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

#include "DlgSmiley.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSmileyForm* SmileyForm;

//---------------------------------------------------------------------------
__fastcall TSmileyForm::TSmileyForm(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::FormClose(TObject* Sender, TCloseAction &Action)
{
  try {delete EmoteStrings;} catch(...) {}
  this->Release();
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::FormShow(TObject *Sender)
{
  if (!sTitle.IsEmpty())
    this->Caption = sTitle;
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::FormKeyDown(TObject* Sender, WORD &Key,
      TShiftState Shift)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::FormCreate(TObject *Sender)
{
  try
  {
    WideString SmileyDir = utils->ExtractFilePathW(utils->GetExeNameW());

    WideString SmileyFile = SmileyDir + utils->Utf8ToWide(SMILEYFILE); // Smileys.txt

    if (!utils->FileExistsW(SmileyFile))
    {
      // "Cannot find... Please reinstall YahCoLoRiZe..."
      utils->ShowMessageU(String(DS[222]) + "\n" + SmileyFile +
                                        "\n\n" + String(DS[223]));
      this->Release();
      return;
    }

    int NewTop = 2;
    int NewLeft = 2;

    EmoteStrings = new TStringsW();
    TempStrings = new TStringsW();

    TempStrings->LoadFromFile(SmileyFile);

    // Clear blank lines and comments
    for (int ii = 0; ii < TempStrings->Count; ii++)
    {
      WideString wTemp = TempStrings->GetString(ii);

      wTemp = utils->TrimW(wTemp);

      if (!wTemp.IsEmpty() && wTemp.Pos("//") != 1)
        EmoteStrings->Add(wTemp);
    }

    delete TempStrings;

    if (EmoteStrings->Count < SMILEY_SPARES ||
            ((EmoteStrings->Count-SMILEY_SPARES) % 3))
    {
      // "Corrupt find... Please reinstall YahCoLoRiZe..."
      utils->ShowMessageU(String(DS[226]) + "\n" + SmileyFile +
                                        "\n\n" + String(DS[223]));
      this->Release();
      return;
    }

    int Count = 0;
    SmileyDir += utils->Utf8ToWide(SMILEYDIR);

    for (int ii = 0; ii < 1000; ii++)
    {
      WideString S = SmileyDir + String(ii+1) + ".jpg";

      if (!utils->FileExistsW(S))
        continue;

      TImage*  I = new TImage(this);

      I->Name = "I" + String(ii+1);
      I->Parent = this;
      I->AutoSize = true;
      I->Picture->LoadFromFile(S);
      I->Left = NewLeft;
      I->Top = NewTop;
      I->OnClick = MouseClick;

      // Add the hint
      for (int jj = SMILEY_SPARES; jj < EmoteStrings->Count; jj += 3)
      {
        int temp = utils->ToIntDefW(EmoteStrings->GetString(jj+0), -1);

        if (temp == ii+1)
        {
          WideString s = EmoteStrings->GetString(jj+2);

          if (!s.IsEmpty())
          {
            I->Hint = s;
            I->ShowHint = true;
          }

          break;
        }
      }

      NewLeft += I->Width + 2;
      if (NewLeft > Width-(2*I->Width+2))
      {
        NewLeft = 2;
        NewTop += I->Height + 2;
      }

      Count++;
    }

    if (Count == 0)
    {
      utils->ShowMessageU("No emoticon image (.jpg) files found at:\n"
                     "\"" + SmileyDir + "\"");
      Release();
    }
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::MouseClick(TObject* Sender)
{
  TImage*  p = (TImage* )Sender;

  // Extract a tag from the object's name-string
  if (p->Name.Length() > 1)
  {
    String S = p->Name.SubString(2, p->Name.Length()-1);
    int tag = S.ToIntDef(-1);

    if (tag >= 0)
    {
      for (int ii = SMILEY_SPARES; ii < EmoteStrings->Count; ii += 3)
      {
        int temp = utils->ToIntDefW(EmoteStrings->GetString(ii+0), -1);

        if (temp != -1 && temp == tag)
        {
          WideString wTemp = EmoteStrings->GetString(ii+1);

          PASTESTRUCT ps = DTSColor->TaeEditPaste(false, wTemp);

          if (ps.error == 0)
          {
            tae->SelLength = wTemp.Length();

            // Relocate this dialog if it's on top of the new selection!
            if (this->Visible)
              utils->RelocateDialog(this);
          }
          break;
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TSmileyForm::FormClick(TObject* Sender)
{
  Close();
}
//---------------------------------------------------------------------------

