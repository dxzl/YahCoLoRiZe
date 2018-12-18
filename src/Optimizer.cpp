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

#include "Optimizer.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Optimizer class functions
//---------------------------------------------------------------------------
__fastcall TOptimizer::TOptimizer(TComponent* Owner)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);

  // Default for initial FG color is Afg (we change this property to
  // NO_COLOR for WingEdit!)
  initialFG = DTSColor->Afg;
}
//---------------------------------------------------------------------------
bool __fastcall TOptimizer::StripRedundantFgRGB(WideString &S, bool bShowStatus)
// This Function is called from TUtils::Optimize
{
  bool bRet = false; // Assume Error

  try
  {
    int iSize = S.Length();
    wchar_t* pBuf = new wchar_t[iSize+1];
    wcsncpy(pBuf, S.c_bstr(), iSize+1);

    bRet = StripRedundantFgRGB(pBuf, iSize, bShowStatus);

    S = WideString(pBuf, iSize);

    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: TOptimizer::StripRedundantFgRGB()\r\n");
#endif
  }

  return bRet;
}
//---------------------------------------------------------------------------
bool __fastcall TOptimizer::StripRedundantFgRGB(wchar_t* pOld, int &OldSize, bool bShowStatus)
// Call this after the main Execute routine to analyze the text for
// unneccessary RGB Foreground colors in front of spaces.

// Returns the same buffer but with a smaller size via reference and true if
// success.
{
  bool bRet = false;
  TList* ColorIndexes = NULL;

  try
  {
    try
    {
      ColorIndexes = new TList();

      bool bPrevColorSpace = false, bColor = false;
      int PrevColorIdx = -1, ColorIdx = -1;

      for(int ii = 0; ii < OldSize; ii++)
      {
        if (pOld[ii] == CTRL_K)
        {
          int fg = NO_COLOR;
          int bg = NO_COLOR;

          int len = utils->CountColorSequence(pOld, ii, OldSize, fg, bg);

          if (fg < 0) // Foreground RGB color?
          {
            ColorIdx = ii;
            bColor = true;
          }

          ii += len;

          continue;
        }

        // !!!!!fix bug 11/8/2018 - ii and OldSize reversed below!!!!!!
        int iRet = utils->SkipCode(pOld, OldSize, ii);

        if (iRet == S_NULL)
          break;

        if (iRet == S_CRLF)
        {
          bPrevColorSpace = false;
          bColor = false;
          PrevColorIdx = -1;
          continue;
        }

        if (iRet != S_NOCODE)
          continue;

        // Have a real char, set flags
        bool bSpace = (pOld[ii] == ' ') ? true : false;

        // If this wchar_t (space or non-space) has a RGB foreground color in
        // front of it and the previous wchar_t was a space with a  rgb fg
        // color, queue the previous space char's color to be deleted.
        // Add index of color (8 chars) to delete...
        if (bPrevColorSpace && bColor && PrevColorIdx >= 0)
          ColorIndexes->Add((void*)PrevColorIdx);

        // This logic holds the bPrevColorSpace flag over a consecutive
        // sequence of spaces, until a real char
        if (!bPrevColorSpace)
          bPrevColorSpace = bColor && bSpace;
        else if (!bSpace)
          bPrevColorSpace = false;

        if (bColor)
          PrevColorIdx = ColorIdx;

        bColor = false;
      }

      // delete each redundant color whose index is in ColorIndexes.
      // (each list-entry is an index to an 8 character color-code, C#rrggbb)
      // Go backward through the list overwriting memory the unnecessary
      // color-code occupies.
      if (ColorIndexes->Count > 0)
      {
        for(int ii = ColorIndexes->Count-1; ii >= 0; ii--, OldSize -= 8)
          for (int jj = (int)ColorIndexes->Items[ii]; jj < OldSize; jj++)
            pOld[jj] = pOld[jj+8];
#if DEBUG_ON
        dts->CWrite("\r\nStripRedundantFgRGB(): new size: " + String(OldSize) + "\r\n");
#endif
      }

      bRet = true;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nException thrown in TOptimizer::StripRedundantFgRGB(wchar_t*, int &, bool)\r\n");
#endif
    }
  }
  __finally
  {
    if (ColorIndexes)
      delete ColorIndexes;
  }
  return bRet;
}
//---------------------------------------------------------------------------
WideString __fastcall TOptimizer::Execute(WideString sIn, bool bShowStatus)
// This Function is called from TUtils::Optimize Returns: Empty string if error
{
  WideString sOut;

  if (bShowStatus)
    DTSColor->CpShow(STATUS[7], DS[70]); // "Optimizing text... Press ESC to quit"

  try
  {
    try
    {
      int OldSize = sIn.Length();

      if (OldSize == 0)
        return "";

      wchar_t* pOld = sIn.c_bstr();

      WideString S;

      SaveState.Clear();
      State.Clear();

      dts->CpSetMaxIterations(OldSize);

      while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

      for(int ii = 0; ii < OldSize; ii++)
      {
        if (pOld[ii] == C_NULL)
          break;

        // User abort???
        Application->ProcessMessages();
        if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
          return ""; // Cancel

        if (pOld[ii] == CTRL_B)
          State.bBold = !State.bBold;
        else if (pOld[ii] == CTRL_U)
          State.bUnderline = !State.bUnderline;
        else if (pOld[ii] == CTRL_R)
          State.bItalics = !State.bItalics;
        else if (pOld[ii] == CTRL_F)
        {
          int ft = utils->CountFontSequence(pOld, ii, OldSize);

          if (ft >= 0)
          {
            ii += 2; // skip over 2-digit decimal font-size code

            if (ft == 0)
              State.fontType = dts->cType;
            //else if (ft == 1)
            //  State.fontType = prevType < 0 ? dts->cType : prevType;
            else
              State.fontType = ft;
          }
          else if (DTSColor->IsYahTrinPal()) // need to write CTRL-F
            S += WideString(CTRL_F);
        }
        else if (pOld[ii] == CTRL_S)
        {
          int fs = utils->CountFontSequence(pOld, ii, OldSize);

          if (fs >= 0)
          {
            ii += 2; // skip over 2-digit decimal font-size code

            if (fs == 0)
              State.fontSize = dts->cSize;
            //else if (fs == 1)
            //  State.fontSize = prevSize < 0 ? dts->cSize : prevSize;
            else
              State.fontSize = fs;
          }
          else if (DTSColor->IsYahTrinPal()) // need to write CTRL-S
            S += WideString(CTRL_S);
        }
        else if (pOld[ii] == CTRL_K)
        {
          int fg = NO_COLOR;
          int bg = NO_COLOR;

          ii += utils->CountColorSequence(pOld, ii, OldSize, fg, bg);

          if (fg == NO_COLOR && bg == NO_COLOR)
          {
            // CTRL_K by itself means "go back to colors preceeding
            // previous color-code" - ProcessIRC() needs to handle that,
            // or the chat-client handles it... here we just let it pass
            // through...

            // Weed-out double Ks!
            if (S[S.Length()] != CTRL_K)
              S += WideString(CTRL_K);
          }
          else
          {
            bool bFgNoColor = (fg == NO_COLOR || fg == IRCNOCOLOR);
            bool bBgNoColor = (bg == NO_COLOR || bg == IRCNOCOLOR);

            // Keep changing FG/BG up to the next printable
            // character -- so we will have the latest FG/BG
            if (!bFgNoColor && !bBgNoColor)
            {
              if (State.fg != fg && State.bg != bg)
              {
                // both colors
                State.fg = fg;
                State.bg = bg;
              }
              else if (State.fg != fg)
                State.fg = fg;
              else if (State.bg != bg)
                State.bg = bg;
            }
            else if (!bFgNoColor)
            {
              // fg only
              if (State.fg != fg)
                State.fg = fg;
            }
            else
            {
              // bg only
              if (State.bg != bg)
                State.bg = bg;
            }
          }
        }
        else
        {
          if (utils->FoundCRLF(pOld, OldSize, ii)) // may increment ii
          {
            // Terminate effect at end of line
            if (!S.IsEmpty())
            {
              if (State.bBold)
                S += WideString(CTRL_B);

              if (State.bUnderline)
                S += WideString(CTRL_U);

              if (State.bItalics)
                S += WideString(CTRL_R);
            }

            // Reset for new line
            SaveState.Clear();
            State.Clear();

            sOut += S + CRLF; // Add previous line to sOut
            S.Empty();

            continue;
          }

          // Copy colors or fontsize and regular text wchar_t to new buffer
          S += SetFormatCodes(State, SaveState, pOld[ii]) + WideString(pOld[ii]);
        }

        // Advance progress-bar
        DTSColor->CpUpdate(ii);
      }

      // add last line (if any)
      if (S.Length() > 0)
        sOut += S;

      // add trailing codes if last line didn't end with a cr/lf
      int len = sOut.Length();

      if (len > 0 && sOut[len] != C_LF && sOut[len] != C_CR)
        sOut += SetFormatCodes(State, SaveState, C_NULL);
    }
    catch(...){}
  }
  __finally
  {
    dts->CpHide();
  }

  return sOut;
}
//---------------------------------------------------------------------------
WideString __fastcall TOptimizer::SetFormatCodes(PUSHSTRUCT State,
                                          PUSHSTRUCT &SaveState, wchar_t c)
{
  WideString s;

  try
  {
    // Here, we print the bold/underline/reverse if it has changed since the
    // last character was printed... (IMPORTANT: when we convert
    // this to HTML, the color tag for a SPAN must enclose the bold
    // tag. Like this: <span><b>X</b><span>
    //
    // Effect OFF
    if (State.bBold != SaveState.bBold && !State.bBold)
      s += WideString(CTRL_B);

    if (State.bUnderline != SaveState.bUnderline && !State.bUnderline)
      s += WideString(CTRL_U);

    if (State.bItalics != SaveState.bItalics && !State.bItalics)
      s += WideString(CTRL_R);

    // Here, we print the color if it has changed since the
    // last character was printed...
    if (State.fg != SaveState.fg && State.bg != SaveState.bg)
    {
// Adding effects later, like underline, we want a fg color assigned in
// front of spaces!
      // Write BG only if next wchar_t is a space
      //if ( pOld[ii] == ' ' )
      //{
      //  if ( DTSColor->IsYahTrin() )
      //    utils->WriteSingle( State.bg, s, false );
      //  else // mIRC/PIRCH/Vortec do not support CTRL-C ,XX format!!!!!!!
      //  {
      //    if ( SaveState.fg == -(NO_COLOR) )
      //    {
      //      utils->WriteColors( initialFG, State.bg, s );
      //      SaveState.fg = initialFG;
      //    }
      //    else
      //      utils->WriteColors( SaveState.fg, State.bg, s );
      //  }
      //
      //  SaveState.bg = State.bg;
      //}
      //else // Write both Fg/BG
      //{
        utils->WriteColors(State.fg, State.bg, s);
        SaveState.fg = State.fg;
        SaveState.bg = State.bg;
      //}
    }
    else if (State.fg != SaveState.fg)
    {
      if (c == L',')
      {
        // Write both Fg/BG to avoid an ambiguous condition
        // (added because a wing that starts with " ," was causing
        // XiRCON to think a BG color existed and the "," was
        // not printed)
        utils->WriteColors(State.fg, State.bg, s);
        SaveState.fg = State.fg;
        SaveState.bg = State.bg;
      }
// Adding effects later, like underline, we want a fg color assigned in
// front of spaces!
//        else if ( pOld[ii] != ' ' )
      else
      {
        utils->WriteSingle(State.fg, s, true);
        SaveState.fg = State.fg;
      }
    }
    else if (State.bg != SaveState.bg)
    {
      // If an rgb color, just write one color, otherwise write two
      if (DTSColor->IsYahTrinPal() && State.bg <= 0)
        utils->WriteSingle(State.bg, s, false);
      else // mIRC/IceChat/Vortec do not support CTRL-C ,XX format!
      {
        if (SaveState.fg == -(NO_COLOR))
        {
          utils->WriteColors(initialFG, State.bg, s);
          SaveState.fg = initialFG;
        }
        else
          utils->WriteColors(SaveState.fg, State.bg, s);
      }

      SaveState.bg = State.bg;
    }

    // Here, we print the fontsize/fonttype if it has changed since the last character was printed...
    if (DTSColor->IsYahTrinPal())
    {
      if (State.fontType != SaveState.fontType)
      {
        s += utils->FontTypeToString(State.fontType);
        SaveState.fontType = State.fontType;
      }

      if (State.fontSize != SaveState.fontSize)
      {
        s += utils->FontSizeToString(State.fontSize);
        SaveState.fontSize = State.fontSize;
      }
    }

    // Here, we print the bold/underline/reverse if it has changed since the last character was printed... (IMPORTANT: when we convert
    // this to HTML, the color tag for a SPAN must enclose the bold tag. Like this: <span><b>X</b><span>
    //
    // Effect ON
    if (State.bBold != SaveState.bBold)
    {
      if (State.bBold)
        s += WideString(CTRL_B);

      SaveState.bBold = State.bBold;
    }

    if (State.bUnderline != SaveState.bUnderline)
    {
      if (State.bUnderline)
        s += WideString(CTRL_U);

      SaveState.bUnderline = State.bUnderline;
    }

    if (State.bItalics != SaveState.bItalics)
    {
      if (State.bItalics)
        s += WideString(CTRL_R);

      SaveState.bItalics = State.bItalics;
    }
  }
  catch(...){}
  return s;
}
//---------------------------------------------------------------------------

