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

#include "DlgColor.h"
#include "LicenseKey.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TColorForm *ColorForm = NULL;
//---------------------------------------------------------------------------
__fastcall TColorForm::TColorForm(TComponent* Owner) : TForm(Owner)
{
  // TColorForm's callerID constants used to define
  // what color is being set.

  // COLOR_FORM_NONE    0
  // COLOR_FORM_FG      1
  // COLOR_FORM_BG      2
  // COLOR_FORM_EFG     3 // Effect Fg
  // COLOR_FORM_EBG     4 // Effect Bg
  // COLOR_FORM_BLEND   5
  // COLOR_FORM_WING    6
  // COLOR_FORM_BORDER  7
  // COLOR_FORM_HTML    8

  callerID = COLOR_FORM_NONE;
  dlgColor = IRCCANCEL; // default
  randRGB = false;
  initColor = IRCWHITE-1; // Property to set initial color panel focus
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::FormCreate(TObject *Sender)
{

  int iLeft = 0;
  int iTop = 0;

  TPanel *p;

  ColorPanel->AutoSize = false;
  ButtonsPanel->AutoSize = false;

  for (int ii = 0; ii < DTSColor->PaletteSize; ii++)
  {
    p = new TPanel(ColorPanel->Handle); // pass in the TComponent * owner responsible for freeing it

    if (p == NULL) break;

    p->Parent = ColorPanel;
    p->Width = 25;
    p->Height = 25;
    p->Left = iLeft;
    p->Top = iTop;
    p->Color = utils->RgbToTColor(DTSColor->Palette[ii]);
    p->Tag = ii;
    p->Visible = true;
    p->Enabled = true;
    // tried to print the color as a hint but it lags 1-behind!
    //p->ParentShowHint = true;
    p->OnMouseMove = FormMouseMove;
    p->OnEnter = PanelEnter;
    p->OnExit = PanelExit;
    p->OnClick = ColorPanelClick;
    p->PopupMenu = DTSColor->ColorCopyPasteMenu;
    ColorPanel->InsertComponent(p);

    if (((ii+1) % 8) == 0)
    {
      iTop += p->Height;
      iLeft = 0;
    }
    else iLeft += p->Width;
  }

  ColorPanel->AutoSize = true;
  ColorPanel->Update();

  int w = ColorPanel->Width/2;

  iLeft = 0;
  iTop = 0;

  for (int ii = P_BEGIN; ii < P_BEGIN+6; ii++) // six buttons
  {
    p = new TPanel(ButtonsPanel->Handle);

    if (p == NULL) break;

    p->Parent = ButtonsPanel;
    p->Width = w;
    p->Height = 25;
    p->Left = iLeft;
    p->Top = iTop;

    int c;
    switch(ii)
    {
      case P_VBLEND: c = IRCSILVER; break;
      case P_RGB: c = IRCSILVER; break;
      case P_HBLEND: c = IRCSILVER; break;
      case P_RANDOM: c = IRCSILVER; break;
      case P_TRANSPARENT: c = IRCSILVER; break;
      case P_CANCEL: c = IRCWHITE; break;
    };

    p->Caption = COLORDIALOGMSG[ii-P_BEGIN];
    p->Color = utils->YcToTColor(c);

    p->Tag = ii;
    p->Visible = true;
    p->Enabled = true;
    p->OnMouseMove = FormMouseMove;
    p->OnEnter = PanelEnter;
    p->OnExit = PanelExit;
    p->OnClick = ColorPanelClick;
    ColorPanel->InsertComponent(p);

    if (((ii+1) % 2) == 0)
    {
      iTop += p->Height;
      iLeft = 0;
    }
    else iLeft += p->Width;
  }

  ButtonsPanel->AutoSize = true;
  ButtonsPanel->Left = 0;
  ButtonsPanel->Top = ColorPanel->Height;

  ColorForm->AutoSize = true;
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::FormShow(TObject *Sender)
{
  try
  {
    // Set bevels for initial appearance
    for (int ii = 0; ii < ColorPanel->ComponentCount; ii++ )
    {
      TPanel* p = dynamic_cast<TPanel *>(ColorPanel->Components[ii]);
      if (p != NULL)
      {
        p->BevelInner = bvLowered;
        p->BevelOuter = bvRaised;
      }
    }

    for (int ii = 0; ii < ColorPanel->ComponentCount; ii++ )
    {
      if (ColorPanel->Components[ii]->Tag == initColor)
      {
        TPanel* p = dynamic_cast<TPanel *>(ColorPanel->Components[ii]);
        if (p != NULL) p->SetFocus();
        break;
      }
    }
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::ColorPanelClick(TObject *Sender)
// Handler for the 20 color-buttons
{
  TPanel* p = dynamic_cast<TPanel *>(Sender);
  if (p == NULL) return;

  // P_BEGIN        100
  // P_VBLEND      (P_BEGIN+0)
  // P_RGB         (P_BEGIN+1)
  // P_HBLEND      (P_BEGIN+2)
  // P_RANDOM      (P_BEGIN+3)
  // P_TRANSPARENT (P_BEGIN+4)
  // P_CANCEL      (P_BEGIN+5)
  if (p->Tag == P_CANCEL)
  {
    dlgColor = IRCCANCEL;
    Close();
  }
  else if (p->Tag == P_VBLEND)
  {
    // Wing, Border or Blend Background Color, return
    if (callerID == COLOR_FORM_WING || callerID == COLOR_FORM_BORDER ||
          callerID == COLOR_FORM_HTML || callerID == COLOR_FORM_BLEND ||
              callerID == COLOR_FORM_EFG || callerID == COLOR_FORM_EBG)
      return;

#if LICENSE_ON
    if (callerID == COLOR_FORM_BG && PK->ComputeDaysRemaining() <= 0)
    {
      utils->ShowMessageU(KEYSTRINGS[4] +
                DTSColor->Iftf->Strings[INFO_WEB_SITE] + KEYSTRINGS[5]);
      return;
    }
#endif

    // Forground/Background Blend button
    if (DTSColor->IsYahTrinPal())
    {
      // Get the desired blend preset to load from
      DTSColor->SelectBlendPreset(Caption);

      if (DTSColor->GBlendPreset >= 0)
      {
        dlgColor = IRCVERTBLEND;
        Close();
      }
    }
    // "Go to the Client Tab and choose Trinity or\n" "YahELite to access color blending!", // 75
    else
      utils->ShowMessageU(DS[75]);
  }
  else if (p->Tag == P_RGB)
  {
    if (DTSColor->IsYahTrinPal() || callerID == COLOR_FORM_HTML)
    {
      dlgColor = RGBCOLOR;
      Close();
    }
    else
      // "RGB colors are not allowed for IRC chat-clients."
      // "Try selecting Client->Trinity to enable RGB colors...", // 21
      utils->ShowMessageU(DS[21]);

  }
  else if (p->Tag == P_HBLEND)
  {
    // Wing, Border or Blend Background Color, return
    if (callerID == COLOR_FORM_WING || callerID == COLOR_FORM_BORDER ||
        callerID == COLOR_FORM_HTML || callerID == COLOR_FORM_BLEND ||
             callerID == COLOR_FORM_EFG || callerID == COLOR_FORM_EBG)
      return;

#if LICENSE_ON
    if (callerID == COLOR_FORM_BG && PK->ComputeDaysRemaining() <= 0)
    {
      utils->ShowMessageU(KEYSTRINGS[4] +
                        DTSColor->Iftf->Strings[INFO_WEB_SITE] + KEYSTRINGS[5]);
      return;
    }
#endif

    // Forground/Background Blend button
    if (DTSColor->IsYahTrinPal())
    {
      // Get the desired blend preset to load from
      DTSColor->SelectBlendPreset(Caption);

      if (DTSColor->GBlendPreset >= 0)
      {
        dlgColor = IRCHORIZBLEND;
        Close();
      }
    }
    // "Go to the Client Tab and choose Trinity or\n" "YahELite to access
    // color blending!", // 75
    else
      utils->ShowMessageU(DS[75]);
  }
  else if (p->Tag == P_RANDOM)
  {
    // Border BG Color or HTML web-page BG color, return
    if (callerID == COLOR_FORM_BORDER || callerID == COLOR_FORM_HTML)
      return;

    dlgColor = IRCRANDOM;
    Close();
  }
  else if (p->Tag == P_TRANSPARENT)
  {
    // HTML can't seem to render foreground "cut-out" transparency... lock it
    // out
    //if (callerID == COLOR_FORM_FG || callerID == COLOR_FORM_EFG)
    //{
    //  // "Transparent foreground text won't properly render to HTML, allow
    //  //  anyway?", // 229
    //  if (utils->ShowMessageU(Handle, DS[229],
    //               MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
    //    return;
    //}

    dlgColor = IRCTRANSPARENT;
    Close();
  }
  else // color panel clicked...
  {
    dlgColor = p->Tag+1;
    Close();
  }
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::FormKeyDown(TObject *Sender, WORD &Key,
                                                        TShiftState Shift)
{
  if (Key == VK_RETURN)
  {
    ColorPanelClick(ActiveControl);
    Key = 0;
  }
  else if (Key == VK_ESCAPE)
  {
    dlgColor = IRCCANCEL;
    Key = 0;
    Close();
  }
  //else if (Shift.Contains(ssCtrl) && Key == 'C')
  //{
  //}
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::PanelEnter(TObject *Sender)
{
  TPanel* p = dynamic_cast<TPanel *>(Sender);
  if (p != NULL)
  {
    p->BevelInner = bvRaised;
    p->BevelOuter = bvLowered;
    // tried to print the color as a hint but it lags 1-behind!
    ////ColorPanel->Hint = "(" + String(GetRValue(p->Color)) + ", " + String(GetGValue(p->Color)) + ", " + String(GetBValue(p->Color)) + ")";
    //BlendColor bc = utils->YcToBlendColor(-DTSColor->Palette[p->Tag]);
    //ColorPanel->Hint = "(" + String(bc.red) + ", " + String(bc.green) + ", " + String(bc.blue) + ")";
  }
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::PanelExit(TObject *Sender)
{
  TPanel* p = dynamic_cast<TPanel *>(Sender);
  if (p != NULL)
  {
    p->BevelInner = bvLowered;
    p->BevelOuter = bvRaised;
  }
}
//---------------------------------------------------------------------------
void __fastcall TColorForm::FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
  TPanel* p = dynamic_cast<TPanel *>(Sender);
  if (p != NULL && !p->Focused()) p->SetFocus();
}
//---------------------------------------------------------------------------

