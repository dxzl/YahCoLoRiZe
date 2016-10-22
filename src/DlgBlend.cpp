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

#include "DlgBlend.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TBlendChoiceForm *BlendChoiceForm;
//---------------------------------------------------------------------------
__fastcall TBlendChoiceForm::TBlendChoiceForm(TComponent* Owner)
  : TForm(Owner)
{
  DTSColor->GBlendPreset = -1; // No selection
  BC = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::PresetPanelClick(TObject *Sender)
{
  TPanel * P = (TPanel *)Sender;

  // Check the tag
  if (!BC || P->Tag < 0 || P->Tag >= MAXBLENDPRESETS) return;

  // See if user tried to load an empty preset
  if (!DTSColor->LoadBlendPresetFromRegistry(BC, P->Tag))
    utils->ShowMessageU(this->Handle, DS[221], MB_ICONWARNING|MB_OK);
  else
  {
    DTSColor->GBlendPreset = P->Tag; // Made a selection
    PaintPanels(BC);
  }
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::OKClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  Release();
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::FormShow(TObject *Sender)
{
  try
  {
    if (Caption == String(DS[65])) // Background
    {
      BC = DTSColor->BG_BlendColors;
      DTSColor->GBlendPreset = DTSColor->BG_BlendPreset;
    }
    else if (Caption == String(DS[63])) // Foreground
    {
      BC = DTSColor->FG_BlendColors;
      DTSColor->GBlendPreset = DTSColor->FG_BlendPreset;
    }

    if (!BC)
    {
      Close();
      return;
    }
  }
  catch(...)
  {
    Close();
    return;
  }

  PaintPanels(BC);
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::PaintPanels(BlendColor * BC)
{
  for (int ii = 0 ; ii < ControlCount ; ii++)
  {
    try
    {
      TPanel * P = dynamic_cast<TPanel *>(Controls[ii]);

      if (P && P->Name.SubString(0, 5) == "Blend")
      {
        DTSColor->SetBlendButtonColor(P, BC);

        if (P->Tag == DTSColor->GBlendPreset)
        {
          P->BevelOuter = bvRaised;
          P->BevelInner = bvNone;
        }
        else
        {
          P->BevelOuter = bvLowered;
          P->BevelInner = bvRaised;
        }

        P->Caption = String(P->Tag+1);
      }
    }
    catch(...) {continue;}
  }
}
//---------------------------------------------------------------------------
void __fastcall TBlendChoiceForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_RETURN || Key == VK_ESCAPE)
  {
    Key = 0;
    Close();
  }
}
//---------------------------------------------------------------------------

