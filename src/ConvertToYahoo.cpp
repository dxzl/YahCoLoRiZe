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

#include "ConvertToYahoo.h"

#pragma package(smart_init)

// Yahoo has special codes for these 10 colors (sends shorter
// text...
// black, blue, cyan, gray, green, pink, purple, orange, red, yellow
const int YahooSubs[10] = { 0x000000,0x008000,0x00c000,0x808080,
    0x008000,0xff80c0,0x800080,0xff8000,0xff0000,0xffff0 };

//---------------------------------------------------------------------------
TConvertToYahoo* ConvertToYahoo = NULL;
//---------------------------------------------------------------------------

__fastcall TConvertToYahoo::TConvertToYahoo(TComponent* Owner)
{
  this->dts = static_cast<TDTSColor*>(Owner);
}
//---------------------------------------------------------------------------
int __fastcall TConvertToYahoo::Execute(bool bShowStatus)
// Take SL_IRC stream's text and convert it to Yahoo color format in SL_HTM
{
  if (dts->SL_IRC == NULL || dts->SL_HTM == NULL ||
                                         dts->SL_IRC->Count == 0)
    return 2; // Error

  int ReturnValue = 0;

  // Change processing status panel... Processing to Yahoo format...
  if (bShowStatus)
    dts->CpShow(STATUS[8], DS[70]);

  try
  {
    try
    {
      // Clear target-stream
      dts->SL_HTM->Clear();

      // Flatten tabs and copy string-list
      TStringsW* sl = new TStringsW();
      sl->Text = utils->FlattenTabs(dts->SL_IRC->Text, dts->RegTabMode);

      // Optimize copy
      utils->Optimize(sl, false, NO_COLOR);

      // Set global private used to prevent re-writing the same fg color
      // that's already in effect - optimizer did not know we would
      // ignore background colors here...
      PrevFg = NO_COLOR;

      while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

      int count = sl->Count;

      dts->CpSetMaxIterations(count);

      WideString Temp; // Put it all in one String

      // For each line...
      for (int ii = 0; ii < count; ii++)
      {
        // User abort???
        Application->ProcessMessages();
        if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
        {
          ReturnValue = 1;
          break; // Cancel
        }

        Temp += ConvertLineToYahoo(sl->GetString(ii));

        // Advance progress-bar
        dts->CpUpdate(ii);
      }

      delete sl;

      // Unless user cancelled we are ok
      if (ReturnValue == 0) // Move new strings to MS_HTM
        dts->SL_HTM->Text = Temp;
    }
    catch(...)
    {
      ReturnValue = 2;
    }
  }
  __finally
  {
    dts->CpHide();
  }

  return ReturnValue;
}
//---------------------------------------------------------------------------
// PUBLIC!
WideString __fastcall TConvertToYahoo::ConvertLineToYahoo(WideString sIn)
{
  try
  {
    // Get effective size
    int ESize;
    utils->GetRealLength(sIn, ESize); // Return # real-chars as a reference

    // return if no printable chars or all spaces
    if (!ESize)
      return CRLF;

    WideString sOut;

    // initialize new string
    if (!dts->StripFont)
    {
      // White background, no bgcolor
      if (dts->Abg == IRCWHITE || dts->Abg == 0xffffff)
        sOut = YAHINITSTR(dts->cFont);
      else
        sOut = YAHCOLORINITSTR(dts->cFont); // Include bgcolor tag
    }
    else
      sOut = "";

    int size = sIn.Length();

    // Mechanism to avoid initial <font = "xx"> tag
    // if it is redundant to the main tag which has
    // the default font size and face already!

    bool bItalicsOn = false, bBoldOn = false, bUnderlineOn = false;

    // For each char...
    for (int jj = 1; jj <= size; jj++)
    {
      if (sIn[jj] == CTRL_F)
      {
        int ft = utils->CountFontSequence(sIn.c_bstr(), jj-1, size);

        if (ft >= 0)
        {
          jj += 2;

          if (!dts->StripFont)
          {
            // <span style="font-family: 'Courier New'">
            int idx;
            if (ft == 0) idx = dts->cType;
            else idx = ft;

            WideString sTemp = utils->GetLocalFontStringW(idx);

            if (!sTemp.IsEmpty())
              sOut += YAHFONTTYPE(sTemp);
          }
        }

        continue;
      }

      if (sIn[jj] == CTRL_S)
      {
        int fs = utils->CountFontSequence(sIn.c_bstr(), jj-1, size);

        if (fs >= 0)
        {
          jj += 2;

          if (!dts->StripFont)
          {
            if (fs == 0)
              sOut += YAHFONTSIZE(dts->cSize); // Size of 0 restores original font
            else
              sOut += YAHFONTSIZE(fs);
          }
        }

        continue;
      }

      if (sIn[jj] == CTRL_K)
      {
        int fg = NO_COLOR, bg = NO_COLOR;

        jj += utils->CountColorSequence(sIn.c_bstr(), jj-1, size, fg, bg);

        bool bFgNoColor = (fg == NO_COLOR || fg == IRCNOCOLOR);

        if (!bFgNoColor)
        {
          if (fg != PrevFg)
            sOut += YahELiteColor(fg, true);

          PrevFg = fg;
        }

        // May add the background color if yahoo ever has a
        // command for it
        continue;
      }

      if (sIn[jj] == CTRL_B)
        sOut += BoldOnOff(true, bBoldOn);
      else if (sIn[jj] == CTRL_U)
        sOut += UnderlineOnOff(true, bUnderlineOn);
      else if (sIn[jj] == CTRL_R)
        sOut += ItalicsOnOff(true, bItalicsOn);
      else if (sIn[jj] == CTRL_O)
      {
        sOut += BoldOnOff(false, bBoldOn);
        sOut += UnderlineOnOff(false, bUnderlineOn);
        sOut += ItalicsOnOff(false, bItalicsOn);
      }
      else if (!utils->AnySpecialChar(sIn[jj]))
        sOut += WideString(sIn[jj]);
    }

    sOut += CRLF;

    return sOut;
  }
  catch(...)
  {
    return CRLF;
  }
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertToYahoo::YahELiteColor(int Color, bool StrType)
// Returns the RGB color string given an integer Palette index (offset up by 1),
// or given a negative or 0 integer representing an RGB color
{
    if (dts->Palette == NULL)
      return "";

    WideString NewString;
    int NewInt;

    // By the time this is called, ConvertColor has set
    // Color to either the foreground color index or a random index
    // in the Palette (1-16). Transparent is Fuchsia for "yahoo" mode.
    if (Color <= 0)
      NewInt = -Color; // RGB Colors are negative numbers
    else if (Color <= dts->PaletteSize)
      NewInt = dts->Palette[Color-1];
    else
      NewInt = dts->Palette[IRCGREEN-1]; // Green is error default

    if (StrType)
      NewString = ConvertToYahooColorString(NewInt);
    else // This format is used in the "font" bgcolor tag
      NewString = WideString("\"#") +
            WideString(IntToHex(NewInt, 6)) + WideString("\"");

    return NewString;
}
//---------------------------------------------------------------------------
WideString  __fastcall TConvertToYahoo::ConvertToYahooColorString(int c)
// Given a RGB color as a positive integer,
// returns an optimized string for Yahoo-chat.
{
  for (int ii = 0 ; ii < 10 ; ii++)
    if (YahooSubs[ii] == c)
      return YAHBASICCOLOR(ii); // return Yahoo basic color string

  // Return Yahoo RGB color-string
  return YAH_RGB(IntToHex(c, 6));
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertToYahoo::BoldOnOff(bool bAllowOn, bool &bBoldOn)
{
  WideString TempStr;

  if (bBoldOn)
  {
    bBoldOn = false;
    TempStr = YAH_B_OFF;
  }
  else if (bAllowOn)
  {
    bBoldOn = true;
    TempStr = YAH_B_ON;
  }

  return TempStr;
}
//---------------------------------------------------------------------------
WideString  __fastcall TConvertToYahoo::UnderlineOnOff(bool bAllowOn,  bool &bUnderlineOn)
{
  WideString TempStr;

  if (bUnderlineOn)
  {
    bUnderlineOn = false;
    TempStr = YAH_U_OFF;
  }
  else if (bAllowOn)
  {
    bUnderlineOn = true;
    TempStr = YAH_U_ON;
  }

  return TempStr;
}
//---------------------------------------------------------------------------
WideString  __fastcall TConvertToYahoo::ItalicsOnOff(bool bAllowOn, bool &bItalicsOn)
{
  WideString TempStr;

  if (bItalicsOn)
  {
    bItalicsOn = false;
    TempStr = YAH_I_OFF;
  }
  else if (bAllowOn)
  {
    bItalicsOn = true;
    TempStr = YAH_I_ON;
  }

  return TempStr;
}
//---------------------------------------------------------------------------

