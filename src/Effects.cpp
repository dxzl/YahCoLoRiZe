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

#pragma package(smart_init)
//---------------------------------------------------------------------------
// SPACEITEM struct functions
//---------------------------------------------------------------------------
__fastcall TProcessEffect::SPACEITEM::SPACEITEM(int Index, int BgColor)
// constructor for struct inside class TProcessEffect...
{
  this->Index = Index;
  this->BgColor = BgColor;
}
//---------------------------------------------------------------------------
__fastcall TProcessEffect::TProcessEffect(TComponent* Owner, int Effect)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);

  this->effect = Effect;

  // Set other properties as needed before
  // calling Execute()

  // Property vars
  returnValue = -1; // user cancel
  returnDelta = 0;

  colorCounter = 1;
  colorCount = 0;
  maxColors = -1; // no limit

  SpaceBgColors = new TList();
}
//---------------------------------------------------------------------------
__fastcall TProcessEffect::~TProcessEffect()
{
  if (SpaceBgColors != NULL) DeleteSpaceBgColors(SpaceBgColors);
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::Execute(bool bShowStatus)
{
  wchar_t* pSaveBuf = NULL;
  wchar_t* pBuf = NULL;

  // error, no V_IRC, V_ORG or V_RTF views!
  if (!utils->IsRtfIrcOrgView())
  {
    Error(-3, pSaveBuf, pBuf); // Error
    return;
  }

  // Property vars
  returnValue = 0; // No error to begin...
  returnDelta = 0;

  bool bHaveSelText = tae->SelLength ? true : false;

  // process buffered text
  WideString TempStr;

  int ii, iSize, iSaveSize, iPresentPar, iNextPar;
  int iFirst, iLast, CI;
  int TotalLength, OldFont, CharCount;
  double Radians;
  wchar_t PreviousChar = C_NULL;
  time_t t;

  int View = tae->View;

  // Change processing status panel... Loading memory-stream
  if (bShowStatus)
    dts->CpShow(STATUS[1]);

  try
  {
    try
    {
      // Get Info for UNDO_EFFECT
      ONCHANGEW oc = utils->GetInfoOC(tae, NULL);

      if (View == V_ORG)
      {
        // Move the MS_ORG Stream to a buffer
        iSaveSize = dts->SL_ORG->TotalLength;
        pSaveBuf = dts->SL_ORG->GetTextBuf();
        oc.p = dts->SL_ORG;
      }
      else // IRC or RTF
      {
        // Move the MS_IRC Stream to a buffer
        iSaveSize = dts->SL_IRC->TotalLength;
        pSaveBuf = dts->SL_IRC->GetTextBuf();
        oc.p = dts->SL_IRC;
      }

      pSaveBuf[iSaveSize] = C_NULL;

      // Change processing status panel... Initializing text-effect
      if (bShowStatus)
        dts->CpShow(STATUS[5]);

      InitialState.Clear();
      LeadingState.Clear();
      TrailingState.Clear();

      if (View == V_RTF)
      {
        // Map the start and end indices of the RTF selected text to the
        // text in the buffer. iFirst points to the First IRC-buffer
        // index corresponding to the Selected Region in TaeEdit, iLast
        // points to the last+1.  CI is the same as iFirst but it points
        // to the start of any preceeding formatting codes... "Code Index"
        if (!utils->GetCodeIndices(pSaveBuf, iSaveSize,
                                                iFirst, iLast, CI, tae))
        {
          Error(-4, pSaveBuf, pBuf); // Error
          return;
        }

        // If the effect is E_BOLD, E_UNDERLINE, E_REVERSE or
        // E_PUSHPOP, we need to capture the format control chars PAST
        // the end of the selection but we do not want the cr/lf from
        // this line's end!
        if (effect == E_BOLD || effect == E_UNDERLINE || effect == E_ITALIC ||
                                                            effect == E_PUSHPOP)
        {
          int idx = iLast - 1;
          if (idx >= 0 && idx < iSaveSize && pSaveBuf[idx] != C_LF &&
                                                      pSaveBuf[idx] != C_CR)
          {
            wchar_t c;
            iLast += utils->SkipCodes(pSaveBuf, iSaveSize, iLast, c);
          }
        }

        // We make iFirst CI here (initial code-start index). This will
        // put any leading codes in our processing buffer - needed for
        // applying bold, underline, italics and push/pop, for state
        // resolution!
        iFirst = CI;

  //      utils->ShowMessageU(
  //        "CI:" + String(CI) + "\n" +
  //        "iFirst:" + String(iFirst) + "\n" +
  //        "iLast:" + String(iLast));

        // Move the the mapped text/codes to a processing
        // buffer...
        iSize = iLast-iFirst;
        pBuf = new wchar_t[iSize+1];

        int jj = 0;
        for (int ii = iFirst; ii < iLast; ii++, jj++)
          pBuf[jj] = pSaveBuf[ii];
        pBuf[jj] = C_NULL;

        // At this point we have moved the chunk of text the user wants to add
        // an effect to, including the pre-existing color-codes, into a
        // buffer ready to be processed.  The text is processed into "TempStr".
        // TempStr will then be merged with pSaveBuf and written into either
        // MS_ORG or MS_IRC.
        //
        // If a single-code bold, underline or italics or push/pop effect, we
        // have also captured the codes at the end as a special case.

        int TempIdx;

        if (bHaveSelText)
          TempIdx = tae->SelStart-utils->GetLine(tae);
        else
          TempIdx = 0;

        // Get state before leading spaces (need this here for ApplyPushPop())
        utils->SetStateFlags(pSaveBuf, iSaveSize, TempIdx, InitialState, false);

        // Get the color/font and text-effect state that exists at the first
        // printable char in the selection...
        // (Background color on spaces is tracked! (bSkipLeadingSpaces set))
        utils->SetStateFlags(pSaveBuf, iSaveSize, TempIdx, LeadingState, true);

        // Obviously we can't leave the LeadingState with NO_COLOR. When we
        // restore colors between words, if there is NO_COLOR the program
        // will do the best it can and use Afg and Abg, but we still end up
        // with an unclear state at the start of the selection. So, we need
        // to write the colors...
        AddInitialCodes(ep3, LeadingState, TempStr);

        // Get state at iLast
        utils->SetStateFlags(pSaveBuf, iLast, STATE_MODE_ENDOFBUF, TrailingState);

        if (TrailingState.fg == NO_COLOR)
          TrailingState.fg = utils->ConvertColor(dts->Foreground, bFullSpectFg);

        if (TrailingState.bg == NO_COLOR)
          TrailingState.bg = utils->ConvertColor(dts->Background, bFullSpectBg);
      }
      else if (View == V_IRC || View == V_ORG)
      {
        if (!utils->GetSelectedZoneIndices(tae, iFirst, iLast))
        {
          Error(-7, pSaveBuf, pBuf); // Error
          return;
        }

        // Move the the mapped text/codes to a processing
        // buffer...
        iSize = iLast-iFirst;

        pBuf = new wchar_t[iSize+1];

        int jj = 0;
        for (int ii = iFirst ; ii < iLast ; ii++, jj++)
          pBuf[jj] = pSaveBuf[ii];
        pBuf[jj] = C_NULL;
      }
      else // Unknown... (move edit-text to buffer...)
        utils->MoveMainTextToBuffer(pBuf, iSize);

      if (pBuf == NULL || iSize == 0)
      {
        Error(-8, pSaveBuf, pBuf); // Error
        return;
      }

      // Set the first part of a chained Undo UNDO_EFFECT_ORIG_TEXT with
      // the text-zone to restore on Undo
      TOCUndo->Add(UNDO_EFFECT_ORIG_TEXT, iFirst, iSize, oc, WideString(pBuf));

      // convert \pagebreak into C_FF before processing (this avoids having
      // the effect applied to a "\pagebreak" string and allows us to strip
      // the C_FF for E_STRIP
      utils->PageBreaksToFormFeed(pBuf, iSize);

      // see if we have all spaces
      bIsAllSpaces = utils->IsAllSpaces(pBuf, iSize);

      // Scan the text to find selection length
      TotalLength = utils->GetRealLength(pBuf, iSize, 0, C_NULL, false);

      CharCount = 1; // Init to 1!

      // Change processing status panel... Stripping old formatting
      if (bShowStatus)
        dts->CpShow(STATUS[6]);

      // Set global flag to tell us we are going to record the color of space
      // chars for this effect.
      bool bIsSpaceEffect = (ep4 == true && (effect == E_INC_COLORS ||
                        effect == E_ALT_COLORS || effect == E_RAND_COLORS));

      if (effect == E_PUSHPOP)
      {
        TempStr = ApplyPushPop(WideString(pBuf));
        goto GetThisOver;
      }

      if (effect == E_BOLD)
      {
        TempStr = ApplySingleCode(WideString(pBuf), CTRL_B);
        goto GetThisOver;
      }

      if (effect == E_UNDERLINE)
      {
        TempStr = ApplySingleCode(WideString(pBuf), CTRL_U);
        goto GetThisOver;
      }

      if (effect == E_ITALIC)
      {
        TempStr = ApplySingleCode(WideString(pBuf), CTRL_R);
        goto GetThisOver;
      }

      if (effect == E_STRIP_CODES)
      {
        if (ep1 == STRIP_ALL)
          StripCodes(pBuf, iSize, false);
        else if (ep1 == STRIP_ALL_PRESERVE)
          StripCodes(pBuf, iSize, true);
        else if (ep1 == STRIP_FG_COLOR)
          utils->StripFgCodes(pBuf, iSize);
        else if (ep1 == STRIP_BG_COLOR)
          utils->StripBgCodes(pBuf, iSize);
        else if (ep1 == STRIP_ALL_COLOR)
          StripExistingColorCodes(pBuf, iSize);
        else if (ep1 == STRIP_FONT_CODES)
        {
          StripFontSize(pBuf, iSize);
          StripFontType(pBuf, iSize);
        }
        else if (ep1 == STRIP_CRLF)
        {
          utils->StripLeadingAndTrailingSpaces(pBuf, iSize);
          utils->StripParagraphCRLF(pBuf, iSize);
        }
        else if (ep1 == STRIP_TAB)
          utils->FlattenTabs(pBuf, iSize, dts->RegTabMode);
        else if (ep1 == STRIP_TRIM_SPACES)
          utils->StripLeadingAndTrailingSpaces(pBuf, iSize);
        else if (ep1 == STRIP_TRIM_TRAILING_SPACES)
          utils->StripTrailingSpaces(pBuf, iSize);
        else if (ep1 == STRIP_PAD_SPACES)
        {
          utils->StripTrailingSpaces(pBuf, iSize);

          int iLineCt, iWidth;
          utils->LongestWidthAndLineCount(pBuf, iSize, iWidth, iLineCt);

          // Use computed max line width if it's greater than tae->RegWidth
          if (iWidth < ep4)
            iWidth = ep4;

          utils->PadTrailingSpaces(pBuf, iSize, iWidth);
        }
        else if (ep1 == STRIP_PUSHPOP)
        {
          StripSingleCode(pBuf, iSize, CTRL_PUSH);
          StripSingleCode(pBuf, iSize, CTRL_POP);
        }
        else if (ep1 == STRIP_BOLD || ep1 == STRIP_UNDERLINE ||
                ep1 == STRIP_ITALICS || ep1 == STRIP_CTRL_O || ep1 == STRIP_FF)
          StripSingleCode(pBuf, iSize, (wchar_t)ep2);

        TempStr = WideString(pBuf, iSize);

        goto GetThisOver; // Always wanted to use a GOTO! :-)
      }

  // Problem with strip here is that you have no initial BG color
  // at the start of each line!
  //    if (effect == E_INC_FGBG ||
  //                       effect == E_ALT_FGBG || effect == E_RAND_FGBG)
  //      StripExistingColorCodes(pBuf, iSize);
  // else if
      if (effect == E_MORPH_FGBG)
      {
        // Populate BlendColor structs for this effect...
        fgToBC = utils->RgbToBlendColor(-dts->FgToColor);
        fgFromBC = utils->RgbToBlendColor(-dts->FgFromColor);
        bgToBC = utils->RgbToBlendColor(-dts->BgToColor);
        bgFromBC = utils->RgbToBlendColor(-dts->BgFromColor);

        // Init the +/- steps to add for each char
        fgRed = (fgToBC.red - fgFromBC.red)/(double)TotalLength;
        fgGreen = (fgToBC.green - fgFromBC.green)/(double)TotalLength;
        fgBlue = (fgToBC.blue - fgFromBC.blue)/(double)TotalLength;
        bgRed = (bgToBC.red - bgFromBC.red)/(double)TotalLength;
        bgGreen = (bgToBC.green - bgFromBC.green)/(double)TotalLength;
        bgBlue = (bgToBC.blue - bgFromBC.blue)/(double)TotalLength;

        // Int Accumulators for the added fractions above to the
        // starting "From" color values
        fgRedAcc = fgFromBC.red;
        fgGreenAcc = fgFromBC.green;
        fgBlueAcc = fgFromBC.blue;
        bgRedAcc = bgFromBC.red;
        bgGreenAcc = bgFromBC.green;
        bgBlueAcc = bgFromBC.blue;

        if (maxColors != -1)
        {
          if (TotalLength >= maxColors)
          {
            colorCount = TotalLength/maxColors;

            if (maxColors % TotalLength)
              colorCount++;
          }
          else colorCount = 1; // Write color on each character

          colorCounter = 1;
        }
      }
      else if (effect == E_RAND_COLORS)
      {
        if (ep3 == EM_FG)
          utils->StripFgCodes(pBuf, iSize); // Mode is Random Foreground?
  // No good!! lose initial bg-colors on each line!
  //    else if (ep3 == EM_BG) StripBGCodes(pBuf, iSize);
      }
      else if (effect == E_SET_COLORS)
      {
        if (ep3 == EM_FG)
          utils->StripFgCodes(pBuf, iSize); // Setting only FG Color
        else if (ep3 == EM_BG)
          utils->StripBgCodes(pBuf, iSize); // Setting only BG Color
        else
          StripExistingColorCodes(pBuf, iSize); // Setting FG/BG Colors
      }
      else if (effect == E_ALT_CHAR)
      {
        if (ep0 != NO_COLOR || ep1 != NO_COLOR)
          utils->StripFgCodes(pBuf, iSize);
        if (ep2 != C_NULL)
          StripSingleCode(pBuf, iSize, (wchar_t)ep2);
        if (ep4 != C_NULL)
          StripSingleCode(pBuf, iSize, (wchar_t)ep4);
      }
      else if (effect == E_FG_BLEND)
        utils->StripFgCodes(pBuf, iSize);
      else if (effect == E_BG_BLEND)
        // NOTE: interesting little problem I ran into - if we strip bg codes
        // in front of a ctrl-k,03 where ",03" is part of the text, you can
        // end up sticking a fg-color only in front of ",03" which turns it
        // into a real fg/bg color! Not cool...
        utils->StripBgCodes(pBuf, iSize);
      else if (effect == E_FONT_TYPE)
        StripFontType(pBuf, iSize);
      else if (IsFontSizeEffect())
        StripFontSize(pBuf, iSize);

      // Do this after codes are stripped so we record the proper indices
      if (bIsSpaceEffect)
      {
        // Get a TList of pointers to SPACEITEMs that each contains the index
        // and background color of every unbroken set of space-chars in pBuf.
        // This is used when the user has chosen NOT to apply the effect to
        // spaces.
        if ((SpaceBgColors = RecordSpaceBgColors(pBuf, iSize)) == NULL)
        {
          Error(-5, pSaveBuf, pBuf); // Error
          return;
        }
      }

      // Change processing status panel... Adding text-effect
      if (bShowStatus)
        dts->CpShow(STATUS[4], DS[70]);

      // See if the user selected an area and missed a push/pop
      if (effect != E_STRIP_CODES && CheckForOverlappingPushPop(pBuf, iSize))
        returnValue = 1; // uneven push/pops print warning

      iPresentPar = 0;

      Pushes = 0;
      PrevFG = NO_COLOR;
      PrevBG = NO_COLOR;

      // Vars for font effects
      Radians = 0;
      OldFont = 0;
      // Init random-number generator
      srand((unsigned) time(&t));

      // Init class vars (static globals)
      bColorOn = false;
      bFontOn = false;
      bUnderlineOn = false;
      bBoldOn = false;
      bItalicsOn = false;

      bSkipWhitespace = true;
      bFirstChar = true;

      // Used in E_ALT_CHAR effect
      bEP2On = false;
      bEP4On = false;
      bEPColorOn = false;

      // This flag is used to reset bFirstChar for every line of certain effects
      bInitEveryLine = (effect == E_SET_COLORS || effect == E_REPLACE_COLOR ||
                effect == E_ALT_COLORS || effect == E_ALT_CHAR ||
                     effect == E_FONT_SIZE || effect == E_FONT_TYPE);

      dts->CpSetMaxIterations(iSize);

      for(;;) // For each paragraph...
      {
        iNextPar = utils->FindNextParagraph(iPresentPar, iSize, pBuf);

        if (effect == E_FG_BLEND || effect == E_BG_BLEND)
          switch(dts->BlendScopeRadioButtons->ItemIndex)
          {
            case BLEND_BY_WORD: // Word
              // Scan the text to find first word length
              TotalLength = utils->GetRealLength(pBuf, iSize,
                                                  iPresentPar, ' ', false);
              bFirstChar = true; // reset blender
              CharCount = 1;
            break;

            case BLEND_BY_LINE: // Line
              // Scan the text to find line length
              TotalLength = utils->GetRealLength(pBuf, iSize,
                                                  iPresentPar, '\n', false);
              bFirstChar = true;
              CharCount = 1;
            break;

            case BLEND_BY_SENTENCE: // Sentence
              // Scan the text to find sentence length
              TotalLength = utils->GetRealLength(pBuf, iSize,
                                                    iPresentPar, '.', false);
              bFirstChar = true; // reset blender
              CharCount = 1;
            break;

            case BLEND_BY_PARAGRAPH: // Paragraph
              TotalLength = iNextPar-iPresentPar;
              bFirstChar = true; // reset blender
              CharCount = 1;
            break;

            default:
              // TotalLength already set above!
            break;
          };

        bool bFoundCRLF = false; // Also cleared in for loop (at end)

        while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

        for(ii = iPresentPar; ii < iNextPar; ii++) // for each char...
        {
          // User abort???
          Application->ProcessMessages();
          if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
          {
            returnValue = -1;
            break; // Cancel
          }

          if (ii >= iSize || pBuf[ii] == C_NULL)
            break;

          if (utils->FoundCRLF(pBuf, ii, true))
          {
            TempStr += pBuf[ii]; // Add CR or LF to new string

            // get the color/font and text-effect state that exists before
            // the 1st char of each new line...
            if (!bFoundCRLF)
            {
              // Get state before leading spaces
              utils->SetStateFlags(pBuf, iNextPar, ii,
                               STATE_MODE_FIRSTCHAR, InitialState, false);

              if (effect == E_FG_BLEND || effect == E_BG_BLEND)
              {
                switch(dts->BlendScopeRadioButtons->ItemIndex)
                {
                  case BLEND_BY_LINE: // Line
                    // Scan the text to find line length
                    TotalLength = utils->GetRealLength(pBuf, iSize, ii+1, '\n', false);
                    bFirstChar = true;
                    CharCount = 1;
                  break;

                  case BLEND_BY_WORD: // Word
                    // Scan the text to find word length
                    TotalLength = utils->GetRealLength(pBuf, iSize, ii+1, ' ', false);
                    bFirstChar = true; // reset blender
                    CharCount = 1;
                  break;

                  default:
                  break;
                };
              }
            }
            // Line-terminator found!
            else if (ii+1 < iNextPar && pBuf[ii+1] != C_CR && pBuf[ii+1] != C_LF)
            {
              // Carry the most recent blend-color to the start of the next line
              // (optimizer will weed out unnecessary codes!)
              if (effect == E_FG_BLEND)
                utils->WriteSingle(PrevFG, TempStr, true);
              else if (effect == E_BG_BLEND)
                utils->WriteSingle(PrevBG, TempStr, false);
              else if (effect == E_MORPH_FGBG)
                // Reset colorcounter for Morph in Paltalk (each line can have
                // 7 colors!) Setting colorCounter = colorCount causes a new
                // color to be written before the next char is printed
                colorCounter = colorCount;
              else if (View == V_RTF)
                // Write codes for cases where there is none
                AddInitialCodes(ep3, InitialState, TempStr);
            }

            // Print colors or font-type and size at start of every line
            if (bInitEveryLine)
              bFirstChar = true; // reset

            bFoundCRLF = true;

            continue;
          }

          if (pBuf[ii] == CTRL_PUSH)
          {
            // Write in the original colors before this effect began to be added
            if (Pushes == 0)
            {
              PUSHSTRUCT s;
              utils->SetStateFlags(pSaveBuf, iFirst+ii,
                                              STATE_MODE_LASTCHAR, s, true);

              //ShowMessage(utils->PrintStateString(s));

              utils->WriteColors(s.fg, s.bg, TempStr);
            }

            Pushes++;
            TempStr += pBuf[ii];
            continue;
          }

          if (pBuf[ii] == CTRL_POP)
          {
            TempStr += pBuf[ii];

            // For this to work, be sure we don't add any extra color-codes
            // way down below for E_PUSHPOP (don't call ResolveState)
            if (Pushes > 0)
              // Write in the most recent blend or morph colors after a pop
              if (--Pushes == 0)
                utils->WriteColors(PrevFG, PrevBG, TempStr);

            continue;
          }

          // other text-effects are applied, suspend this one
          if (Pushes > 0)
          {
            // Add all pre-existing text and codes to TempStr and include the
            // printable chars in CharCount...
            if (!CheckExistingCodes(ii, pBuf, TempStr, iNextPar))
            {
              CharCount++;
              TempStr += pBuf[ii]; // Add printable chars
            }

            continue;
          }

          //------------------------------------------------------------
          // NOTE: CheckExisting Codes will set unset the local bold,
          // underline flags, etc. !!!!!!!!!!!!!!!!!!!!!!!!!!!
          // Advance ii over existing color and font codes
          if (CheckExistingCodes(ii, pBuf, TempStr, iNextPar))
            continue;
          // bColorOn bFontOn bUnderlineOn bBoldOn bItalicsOn
          //------------------------------------------------------------

          // Advance ii over smilies
          if (dts->AutoDetectEmotes->Checked &&
                        utils->SkipSmileys(ii, pBuf, TempStr, iNextPar))
            continue;

          if (effect == E_FG_BLEND || effect == E_BG_BLEND)
          {
            switch(dts->BlendScopeRadioButtons->ItemIndex)
            {
              case BLEND_BY_SENTENCE: // Sentence
                if (PreviousChar == '.')
                {
                  // Scan the text to find next sentence length
                  TotalLength = utils->GetRealLength(pBuf, iSize,
                                                          ii, '.', false);
                  bFirstChar = true; // reset blender
                  CharCount = 1;
                }
              break;

              case BLEND_BY_WORD: // Word
                if (PreviousChar == ' ' && !bIsAllSpaces)
                {
                  // Scan the text to find word length
                  TotalLength = utils->GetRealLength(pBuf, iSize,
                                                        ii, ' ', false);
                  bFirstChar = true; // reset blender
                  CharCount = 1;
                }
              break;

              default:
              break;
            };
          }

          // What this does: When we add an effect to text with spaces between
          // words, etc., we need to restore their colors
          if (bIsSpaceEffect && pBuf[ii] == ' ')
          {
            if (SpaceBgColors->Count > 0)
            {
              SPACEITEM* p = (SPACEITEM*)SpaceBgColors->Items[0];
              if (ii >= p->Index)
              {
                utils->WriteSingle(p->BgColor, TempStr, false);
                delete p;
                SpaceBgColors->Delete(0);
              }
            }

            TempStr += ' ';
            continue;
          }

          // Add the character to the text line
          switch (effect)
          {
            case E_NONE: // placeholder for future effects

              TempStr += pBuf[ii];

            break;

            case E_MORPH_FGBG: // Morph Fg/Bg colors

              BlendColor TempBC;

              if (dts->bFgChecked)
              {
                fgRedAcc += fgRed;
                fgGreenAcc += fgGreen;
                fgBlueAcc += fgBlue;

                TempBC.red = fgRedAcc;
                TempBC.green = fgGreenAcc;
                TempBC.blue = fgBlueAcc;

                LimitBC(TempBC); // Check for 0-255 limits

                int Temp = -utils->BlendColorToRgb(TempBC);

                if (Temp != NO_COLOR)
                {
                  if (colorCounter >= colorCount)
                    utils->WriteSingle(Temp, TempStr, true);

                  PrevFG = Temp;
                }
              }

              if (dts->bBgChecked)
              {
                bgRedAcc += bgRed;
                bgBlueAcc += bgBlue;
                bgGreenAcc += bgGreen;

                TempBC.red = bgRedAcc;
                TempBC.green = bgGreenAcc;
                TempBC.blue = bgBlueAcc;

                LimitBC(TempBC); // Check for 0-255 limits

                int Temp = -utils->BlendColorToRgb(TempBC);

                if (Temp != NO_COLOR)
                {
                  if (colorCounter >= colorCount)
                    utils->WriteSingle(Temp, TempStr, false);

                  PrevBG = Temp;
                }
              }

              // This counter limits the total number of colors written during
              // processing of a block of text
              if (colorCounter >= colorCount)
                colorCounter = 1; // Reset
              else
                colorCounter++;

              TempStr += pBuf[ii];

            break;

            case E_REPLACE_COLOR:

              // Note: All of the replacing happens in CheckExistingCodes()!

              TempStr += pBuf[ii];

            break;

            case E_INC_COLORS: // Incriment Foreground Background

              // NOTE: Spaces in this effect are now handled above
              // (search "What this does:")
              WriteColorsInc(TempStr, ep3);
              TempStr += pBuf[ii];

            break;

            case E_ALT_COLORS: // Alternate Foreground Background

            {
              static int Fg, Bg;
              static bool bToggle = false;

              // NOTE: Spaces in this effect are now handled above (search "What this does:")
              if (bFirstChar)
              {
                // Reverse colors every-other line
                if (bToggle)
                {
                  Fg = ep1;
                  Bg = ep2;
                }
                else
                {
                  Fg = ep2;
                  Bg = ep1;
                }

                bToggle = !bToggle;
              }

              // ep3 = mode, ep1 = fg color, ep2 = bg color
              WriteColorsAlt(TempStr, ep3, Fg, Bg);
              TempStr += pBuf[ii];
            }

            break;

            case E_RAND_COLORS: // Random Foreground Background

            {
              int fg, bg;

              // NOTE: Spaces in this effect are now handled above (search "What this does:")
              if (bFirstChar) // first character?
              {
                fg = utils->ConvertColor(dts->Foreground, bFullSpectFg);
                bg = utils->ConvertColor(dts->Background, bFullSpectBg);
              }

              WriteColorsRand(TempStr, ep3, fg, bg);
              TempStr += pBuf[ii];
            }

            break;

            case E_ALT_CHAR: // Alternate

              // This will alternate with the color from the dialog and the current FG color - you can't really restore each individual
              // color very eaisly...
              if (bFirstChar || bSkipWhitespace) // first char?
              {
                if (pBuf[ii] != ' ' || bIsAllSpaces)
                {
                  if (ep2 != C_NULL)
                  {
                    TempStr += (wchar_t)ep2;
                    bEP2On = true;
                  }
                  else if (ep4 != C_NULL)
                  {
                    TempStr += (wchar_t)ep4;
                    bEP4On = true;
                  }

                  // Start with Color A
                  utils->WriteSingle(ep0, TempStr, true);
                  bEPColorOn = false;

                  // Add the character
                  TempStr += pBuf[ii];

                  if (ep1 != NO_COLOR)
                  {
                    int fg = ep1;

                    if (fg == IRCRANDOM)
                    {
                      fg = Rfg();

                      int timeout = 0;

                      while (++timeout < 10 && (fg == LeadingState.bg || fg == LeadingState.fg))
                        fg = Rfg();
                    }

                    utils->WriteSingle(fg, TempStr, true);
                    PrevFG = fg;
                    bEPColorOn = true;
                  }

                  bSkipWhitespace = false;
                }
                else
                {
                  // Add the character
                  TempStr += pBuf[ii];
                  bSkipWhitespace = true;
                }
              }
              else
              {
                if (pBuf[ii] == ' ' && !bIsAllSpaces)
                {
                  if (bEP2On)
                  {
                    TempStr += (wchar_t)ep2;
                    bEP2On = false;
                  }
                  if (bEP4On)
                  {
                    TempStr += (wchar_t)ep4;
                    bEP4On = false;
                  }

                  // Add the character
                  TempStr += pBuf[ii];

                  if (bEPColorOn)
                  {
                    utils->WriteSingle(ep0, TempStr, true);
                    bEPColorOn = false;
                  }

                  bSkipWhitespace = true; // go skip spaces
                }
                else // printable char
                {
                  if (bEP2On)
                  {
                    bEP2On = false;
                    TempStr += (wchar_t)ep2;

                    if (ep4 != C_NULL)
                    {
                      bEP4On = true;
                      TempStr += (wchar_t)ep4;
                    }

                    // Add the character
                    TempStr += pBuf[ii];
                  }
                  else
                  {
                    if (bEP4On)
                    {
                      bEP4On = false;
                      TempStr += (wchar_t)ep4;
                    }

                    if (ep2 != C_NULL)
                    {
                      bEP2On = true;
                      TempStr += (wchar_t)ep2;
                    }

                    // Add the character
                    TempStr += pBuf[ii];
                  }

                  if (bEPColorOn)
                  {
                    utils->WriteSingle(ep0, TempStr, true);
                    bEPColorOn = false;
                  }
                  else if (ep1 != NO_COLOR)
                  {
                    int fg = ep1;

                    if (fg == IRCRANDOM)
                    {
                      fg = Rfg();

                      int timeout = 0;
                      while (++timeout < 10 && (fg == LeadingState.bg || fg == LeadingState.fg || fg == PrevFG))
                        fg = Rfg();
                    }

                    // Write new color
                    utils->WriteSingle(fg, TempStr, true);
                    PrevFG = fg;
                    bEPColorOn = true;
                  }
                }
              }

              if (CharCount == TotalLength) // last character?
              {
                if (ep2 != C_NULL)
                {
                  bool flag;
                  if (ep2 == CTRL_B) flag = LeadingState.bBold;
                  else if (ep2 == CTRL_U) flag = LeadingState.bUnderline;
                  else flag = LeadingState.bItalics;

                  if ((bEP2On && !flag) || (!bEP2On && flag))
                  {
                    TempStr += (wchar_t)ep2;
                    bEP2On = flag;
                  }
                }

                if (ep4 != C_NULL)
                {
                  bool flag;

                  if (ep4 == CTRL_B)
                    flag = LeadingState.bBold;
                  else if (ep4 == CTRL_U)
                    flag = LeadingState.bUnderline;
                  else
                    flag = LeadingState.bItalics;

                  if ((bEP4On && !flag) || (!bEP4On && flag))
                  {
                    TempStr += (wchar_t)ep4;
                    bEP4On = flag;
                  }
                }
              }

            break;

            case E_SET_COLORS:

              if (bFirstChar) // Reset bFirstChar every new line!
              {
                int fg = ep1;
                int bg = ep2;

                if (ep1 == IRCRANDOM)
                  fg = Rfg();

                if (ep2 == IRCRANDOM)
                  bg = Rbg();

                utils->WriteColors(fg, bg, TempStr);

                // Need this so if we process a Pop, it writes our new
                // color after the pop!
                if (PrevFG != fg)
                  PrevFG = fg;
                if (PrevBG != bg)
                  PrevBG = bg;
              }

              TempStr += pBuf[ii];

            break;

            case E_FG_BLEND: // Blend text according to settings in Blend Tab

              if (dts->FG_BlendEngine && dts->FG_BlendEngine->EBC && dts->FG_BlendEngine->BlendEnabledCount >= 2)
              {
                int Temp;

                if (bFirstChar) Temp = dts->FG_BlendEngine->Init(TotalLength);
                else Temp = dts->FG_BlendEngine->Blend(CharCount);

                if (Temp == BLEND_ERR1) Temp = -utils->BlendColorToRgb(dts->FG_BlendEngine->EBC[0]);

                if (Temp != NO_COLOR)
                {
                  utils->WriteSingle(Temp, TempStr, true);
                  PrevFG = Temp;
                }
                // else color-change is below RGB-Threshold!
                // no color will be printed...
              }

              TempStr += pBuf[ii];

            break;

            case E_BG_BLEND: // Blend text according to settings in Blend Tab

              // Problem with BG blend is that this effect is applied 1st to "some text". *Then the text is run through ProcessIRC
              // which breaks up the lines and moves words - but right here we have no way of knowing where the line-breaks will be.
              // The placement of blend-color-codes depends on RGB Threshold alone. This effects processor scans for embedded Push/Pop
              // and won't blend in those "write-protected" zones.  One idea to fix this problem was to enclose the entire area
              // encompassed in the effect-zone in a Push/Pop. That has proven too complex when you get into the text processing
              // that is needed for all combinations of user settings.
              if (dts->BG_BlendEngine && dts->BG_BlendEngine->EBC && dts->BG_BlendEngine->BlendEnabledCount >= 2)
              {
                int Temp;

                if (bFirstChar) Temp = dts->BG_BlendEngine->Init(TotalLength);
                else Temp = dts->BG_BlendEngine->Blend(CharCount);

                if (Temp == BLEND_ERR1) Temp = dts->Abl; // Set bg to blend BG color

                if (Temp != NO_COLOR)
                {
                  utils->WriteSingle(Temp, TempStr, false);
                  PrevBG = Temp;
                }
                // else color-change is below RGB-Threshold!
                // no color will be printed...
              }

              TempStr += pBuf[ii];

            break;

            case E_FONT_TYPE: // Font type-change

              if (bFirstChar) TempStr += utils->FontTypeToString(ep1);
              TempStr += pBuf[ii];

              if (CharCount == TotalLength) // last character?
                TempStr += utils->FontTypeToString(TrailingState.fontType);

            break;

            case E_FONT_SIZE: // Font size-change

              if (bFirstChar) TempStr += utils->FontSizeToString(ep1);
              TempStr += pBuf[ii];

              if (CharCount == TotalLength) // last character?
                TempStr += utils->FontSizeToString(TrailingState.fontSize);

            break;

            case E_FONT_MOUNTAIN:

              {
                int temp, NewFont;

                // Radians begins at 0, sin(0) = 0;
                Radians = ep5 * ((double)CharCount/(double)TotalLength);

                temp = (ep2-ep1) * sin(Radians);
                if (temp < 0) temp = -temp;

                NewFont = ep1 + temp;

                if (NewFont < 6) NewFont = 6;
                if (NewFont > 72) NewFont = 72;

                //Radians += ep5/TotalLength;

                int diff = NewFont - OldFont;
                if (diff < 0) diff = -diff;

                //if (bFirstChar || diff > 2 || (diff > 0 && TotalLength < 10) || (diff > 1 && TotalLength < 20))
                if (bFirstChar || diff > 0)
                {
                  TempStr += utils->FontSizeToString(NewFont);
                  OldFont = NewFont;
                }

                TempStr += pBuf[ii];
              }

            break;

            case E_FONT_VALLEY:

              {
                int temp, NewFont;

                // Radians begins at 0, sin(0) = 0;
                Radians = ep5 * ((double)CharCount/(double)TotalLength);

                temp = (ep1-ep2) * sin(Radians);
                if (temp < 0) temp = -temp;

                NewFont = ep1 - temp;

                if (NewFont < 6) NewFont = 6;
                if (NewFont > 72) NewFont = 72;

                //Radians += ep5/TotalLength;

                int diff = NewFont - OldFont;
                if (diff < 0) diff = -diff;

                //if (bFirstChar || diff > 2 || (diff > 0 && TotalLength < 10) || (diff > 1 && TotalLength < 20))
                if (bFirstChar || diff > 0)
                {
                  TempStr += utils->FontSizeToString(NewFont);
                  OldFont = NewFont;
                }

                TempStr += pBuf[ii];
              }

            break;

            case E_FONT_SAWTOOTH:

              {
                if (bFirstChar) ep5 = ep1;
                else
                {
                  if (CharCount == TotalLength/2) ep5 -= 2*(ep2-ep1)/(double)TotalLength;
                  else ep5 += 2*(ep2-ep1)/(double)TotalLength;
                }

                int NewFont = (int)ep5;
                if (NewFont < 6) NewFont = 6;
                if (NewFont > 72) NewFont = 72;

                int diff = NewFont - OldFont;
                if (diff < 0) diff = -diff;

                //if (bFirstChar || diff > 2 || TotalLength < 10 || (TotalLength < 20 && diff > 1))
                if (bFirstChar || diff > 0)
                {
                  TempStr += utils->FontSizeToString(NewFont);
                  OldFont = NewFont;
                }

                TempStr += pBuf[ii];
              }

            break;

            case E_FONT_RANDOM:

              {
                ep5 = ep1 + random(ep2-ep1);

                int NewFont = (int)ep5;
                if (NewFont < 6) NewFont = 6;
                if (NewFont > 72) NewFont = 72;

                int diff = NewFont - OldFont;
                if (diff < 0) diff = -diff;

                //if (bFirstChar || TotalLength < 10 || (TotalLength < 20 && diff > 1) || diff > 2)
                if (bFirstChar || diff > 0)
                {
                  TempStr += utils->FontSizeToString(NewFont);
                  OldFont = NewFont;
                }

                TempStr += pBuf[ii];
              }

            break;

            default:
              TempStr += pBuf[ii];
          }

          // Used for resetting the blender on words and sentenses.
          PreviousChar = pBuf[ii];

          bFoundCRLF = false;

          // bypassing leading cr/lf changes ii to +number so effects that
          // look for ii=0 won't initialize!
          // Solution is this flag...
          bFirstChar = false;

          // Advance progress-bar
          dts->CpUpdate(ii);

          CharCount++;
        } // End For

        // Last paragraph or user cancelled?
        if (iNextPar >= iSize || returnValue == -1)
          break;

        iPresentPar = iNextPar;
      }

  // Goto point after processing E_STRIP_CODES
  GetThisOver:

      // Display "Merging buffers..."
      if (bShowStatus)
        dts->CpShow(STATUS[0]);

      // Convert the C_FF back to \pagebreak...
      TempStr = utils->FormFeedToPageBreaks(TempStr);

      // Init vars for last step of Undo
      oc.p = NULL; // Undo (no string-list!)
      int UndoIdx = 0;

      if (View == V_RTF || View == V_IRC || View == V_ORG)
      {
        try
        {
          // Now we need to merge our SaveBuffer with TempStr
          // and put the output into SL_IRC
          delete [] pBuf;

          if (effect != E_PUSHPOP && effect != E_UNDERLINE &&
                                    effect != E_BOLD && effect != E_ITALIC)
          {
            // Optimize (otherwise, codes codes build up!). Only optimize the
            // text we changed or we will not be able to undo just the
            // effect-zone...
            TempStr = utils->Optimize(TempStr, bShowStatus);

            // ResolveState will see if a font or color code exists in TempStr
            // (the effect string) and if the end state is different from
            // LeadingState - the LeadingState codes will be appended to TempStr
            //
            // Only resolve if we added an effect to a selected zone that does
            // not reach to the end of the document
            if (bHaveSelText && iLast < iSaveSize)
              utils->ResolveState(TrailingState, TempStr);

            //utils->ShowHex(TempStr);
            //utils->ShowHex(utils->PrintStateString(EffectTrailingState));
          }

          int Slen = TempStr.Length();
          wchar_t* pStr = TempStr.c_bstr();

          // Make a buffer more than big enough
          iSize = Slen + iSaveSize;
          pBuf = new wchar_t[iSize+1];

          int kk, ll;

          for (kk = 0; kk < iFirst; kk++)
            pBuf[kk] = pSaveBuf[kk];
          for (ll = 0; ll < Slen; kk++, ll++)
            pBuf[kk] = pStr[ll];
          for (ll = iLast; ll < iSaveSize; kk++, ll++)
          {
            if (pSaveBuf[ll] == C_NULL)
              break;

            pBuf[kk] = pSaveBuf[ll];
          }

          delete [] pSaveBuf;
          pSaveBuf = NULL;

          pBuf[kk] = C_NULL;

          iSize = kk;

          // The ReturnDelta property CAN be negative if the optimizer
          // strips a lot of original codes!
          returnDelta = iSize - iSaveSize - 1;

          // Save plain-text version of IRC-formatted data in MS_IRC
          if (iSize > 0)
            dts->SL_IRC->SetTextBuf(pBuf);
          else
            dts->SL_IRC->Clear();

          oc.p = dts->SL_IRC; // Undo SL_IRC
          UndoIdx = iFirst;
        }
        catch(...) { Error(-11, pSaveBuf, pBuf); } // Error
      }
      else // Other views
      {
        if (bHaveSelText)
        {
          tae->SelTextW = TempStr;
          UndoIdx = tae->SelStart;
        }
        else
          tae->TextW = TempStr;
      }

      // Set the second part of a chained Undo UNDO_EFFECT_NEW_TEXT with
      // the range of new effect-text to delete on Undo
      TOCUndo->Add(UNDO_EFFECT_NEW_TEXT, UndoIdx, TempStr.Length(),
                                                          oc, "", true);

      // returnValue is 0 if no errors or -1 if user pressed ESC
      // 1 if Push/Pop warning
      Error(returnValue, pSaveBuf, pBuf); // Delete buffers
    }
    catch(...)
    {
      Error(-12, pSaveBuf, pBuf); // Error
    }
  }
  __finally
  {
    dts->CpHide();
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::Error(int code, wchar_t* pBuf1, wchar_t* pBuf2)
// pBuf1 and pBuf2 default to NULL
{
  try { if (pBuf1) delete [] pBuf1; } catch(...) { code = -15; }
  try { if (pBuf2) delete [] pBuf2; } catch(...) { code = -16; }
  returnValue = code;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::CheckExistingCodes(int &idx,  wchar_t buf[],
                                           WideString &Output, int NextParIdx)
// Advances idx over existing font and color codes, adding them to Output.
// Returns true when a code was found and added.
{
    if (idx >= NextParIdx)
      return false;

    wchar_t c = buf[idx];

    if (c == CTRL_F || c == CTRL_S)
    {
      int fs = utils->CountFontSequence(buf, idx, NextParIdx);

      Output += c;

      if (fs >= 0 && NextParIdx > idx && NextParIdx-idx >= 1+2)
      {
        bFontOn = true;

        // Add the font-sequence to the current line (Or strip it out
        // if E_STRIP_CODES)
        if (fs == 0)
        {
          int val = (c == CTRL_F ? dts->cType : dts->cSize);
          Output += WideString(val);
        }
        else
        {
          for (int ii = 0 ; ii < 2 ; ii++, idx++)
            Output += buf[idx+1];
        }
      }
      else
        bFontOn = false;

      // don't process this code in main loop
      // since we just processed it here.
      return true;
    }

    if (buf[idx] == CTRL_K)
    {
      int fg = NO_COLOR;
      int bg = NO_COLOR;

      // Get length of color-codes not including this CTRL_K
      int Count = utils->CountColorSequence(buf, idx, NextParIdx, fg, bg);

      if (Count)
      {
        if (effect == E_REPLACE_COLOR)
          WriteColorsReplace(Output, fg, bg);
          
        idx += Count;
        bColorOn = true;
      }
      else
      {
        Output += CTRL_K;
        bColorOn = false;
      }

      // don't process this code in main loop
      // since we just processed it here
      return true;
    }

    if (buf[idx] == CTRL_B)
    {
      bBoldOn = !bBoldOn;

      Output += CTRL_B;

      // don't process this code in main loop
      // since we just processed it here
      return true;
    }

    if (buf[idx] == CTRL_U)
    {
      bUnderlineOn = !bUnderlineOn;

      Output += CTRL_U;

      // don't process this code in main loop
      // since we just processed it here
      return true;
    }

    if (buf[idx] == CTRL_R)
    {
      bItalicsOn = !bItalicsOn;

      Output += CTRL_R;

      // don't process this code in main loop
      // since we just processed it here
      return true;
    }

    if (buf[idx] == CTRL_O)
    {
      bColorOn = false;
      bFontOn = false;
      bUnderlineOn = false;
      bBoldOn = false;
      bItalicsOn = false;

      Output += CTRL_O;

      // don't process this code in main loop
      // since we just processed it here
      return true;
    }

    if (utils->AnySpecialChar(buf[idx]))
      return true;

    return false;
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripCodes(wchar_t* pBuf, int &iSize,
                                                  bool bKeepProtectedCodes)
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_PUSH)
    {
      if (bKeepProtectedCodes) Pushes++;
      else continue; // strip the push
    }
    else if (pBuf[ii] == CTRL_POP)
    {
      if (bKeepProtectedCodes)
      {
        if (--Pushes < 0) Pushes = 0;
      }
      else // Strip the pop
        continue;
    }
    else if (!Pushes && (pBuf[ii] == CTRL_F || pBuf[ii] == CTRL_S))
    {
      if (utils->CountFontSequence(pBuf, ii, iSize)) ii += 2;
      continue;
    }
    else if (!Pushes && pBuf[ii] == CTRL_K)
    {
      // strip
      ii += utils->CountColorSequence(pBuf, ii, iSize);
      continue;
    }
    else if (!Pushes && (pBuf[ii] == CTRL_B || pBuf[ii] == CTRL_U ||
                              pBuf[ii] == CTRL_R || pBuf[ii] == CTRL_O))
      continue;

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripFontSize(wchar_t* pBuf, int &iSize)
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_PUSH)
      Pushes++;
    else if (pBuf[ii] == CTRL_POP)
    {
      if (--Pushes < 0)
        Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_S)
    {
      if (utils->CountFontSequence(pBuf, ii, iSize))
        ii += 2;

      continue;
    }

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripFontType(wchar_t* pBuf, int &iSize)
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_PUSH)
      Pushes++;
    else if (pBuf[ii] == CTRL_POP)
    {
      if (--Pushes < 0)
        Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_F)
    {
      if (utils->CountFontSequence(pBuf, ii, iSize))
        ii += 2;

      continue;
    }

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripExistingColorCodes(wchar_t* pBuf,
                                                              int &iSize)
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_PUSH)
      Pushes++;
    else if (pBuf[ii] == CTRL_POP)
    {
      if (--Pushes < 0)
        Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_K)
    {
      // strip
      ii += utils->CountColorSequence(pBuf, ii, iSize);
      continue;
    }

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripSingleCode(wchar_t* pBuf,
                                              int &iSize, wchar_t Code)
{
  int idx = 0;
  int Pushes = 0;

  bool bProcessPushPop;

  if (Code == CTRL_PUSH || Code == CTRL_POP)
    bProcessPushPop = false;
  else
    bProcessPushPop = true;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (bProcessPushPop)
    {
      if (pBuf[ii] == CTRL_PUSH)
        Pushes++;
      else if (pBuf[ii] == CTRL_POP)
      {
        if (--Pushes < 0)
          Pushes = 0;
      }
      else if (!Pushes && pBuf[ii] == Code)
        continue;
    }
    else if (pBuf[ii] == Code)
      continue;

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::StripFGandUnderlineCodes(wchar_t* pBuf, int &iSize)
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_PUSH)
      Pushes++;
    else if (pBuf[ii] == CTRL_POP)
    {
      if (--Pushes < 0)
        Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_K)
    {
      // strip
      int fg = NO_COLOR;
      int bg = NO_COLOR;
      ii += utils->CountColorSequence(pBuf, ii, iSize, fg, bg);

      // Re-write background code
      if (bg != NO_COLOR)
        utils->WriteSingle(bg, WideString(pBuf), false);

      continue;
    }
    else if (!Pushes && pBuf[ii] == CTRL_U)
      continue;

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
// Here, S has our selected-region including leading and trailing format
// codes.
//
// We also have InitialState and TrailingState to work with...
WideString __fastcall TProcessEffect::ApplyPushPop(WideString S)
{
  if (S.IsEmpty())
    return "";

  // Strip if text has pre-existing codes
  bool bHasPush = utils->ContainsChar(S, CTRL_PUSH);
  bool bHasPop = utils->ContainsChar(S, CTRL_POP);

  if (bHasPush || bHasPop)
  {
    if (bHasPush)
      S = utils->StripChar(S, CTRL_PUSH);

    if (bHasPop)
      S = utils->StripChar(S, CTRL_POP);

    return S;
  }

  TStringsW* sl = new TStringsW();

  sl->Text = S;

  int slCount = sl->Count;

  if (!slCount)
    return "";

  // first do the push on the first string

  // Copy InitialState state but clear the style flags
  PUSHSTRUCT psTemp = InitialState;
  psTemp.ClearStyle();

  // Add font/color state-codes the protected-zone needs to be independant
  // after the push.
  WideString wState = WideString(CTRL_PUSH) +
                              utils->PrintStateString(psTemp, true);

  S = sl->GetString(0); // get first string

  // Add style flags before or after push as dictated by state at the
  // selection-start and the style control chars that immediately
  // preceed the first selection char...
  //
  // InitialState is the state at the first char (including spaces) in the
  // selection.
  //
  // If style is on at the selected text and we have that style char in
  // our leading codes then we need to write the style char after the
  // push; otherwise, before it.
  // Get the state and length of local-leading-codes only (this is not the same
  // as the global InitialState which takes into account text before the
  // selected-zone even begins, back to the beginning of the line)
  int idx;
  PUSHSTRUCT ps = utils->GetLeadingState(S, idx);

  if (idx >= 0)
  {
    if (ps.bBold)
    {
      if (InitialState.bBold)
        wState += CTRL_B;
      else
        wState = utils->InsertW(wState, CTRL_B, 1);
    }
    if (ps.bUnderline)
    {
      if (InitialState.bUnderline)
        wState += CTRL_U;
      else
        wState = utils->InsertW(wState, CTRL_U, 1);
    }
    if (ps.bItalics)
    {
      if (InitialState.bItalics)
        wState += CTRL_R;
      else
        wState = utils->InsertW(wState, CTRL_R, 1);
    }

    // Replace the old leading state with the new one we've generated
    S = utils->DeleteW(S, 1, idx); // delete idx chars
    S = utils->InsertW(S, wState, 1);

    if (slCount > 1)
      S += CTRL_POP; // tack on this line's pop if more than one line

    // write the string back
    sl->SetString(S, 0);
  }

  // do push/pop for each of the in-between strings (if any)...

  if (slCount > 2)
  {
    for (int ii = 1; ii < slCount-1; ii++)
    {
      S = sl->GetString(ii);
      S = utils->InsertW(S, CTRL_PUSH, 1); // Leading code
      S += CTRL_POP; // Trailing code
      sl->SetString(S, ii);
    }
  }

  // now do the pop for the last string (which might be the same as the
  // first string!)

  S = sl->GetString(slCount-1); // get last string

  int len = utils->GetRealLength(S);

  if (len > 0)
  {
    // Copy the state at the first char following the protected-zone
    // but clear the style flags
    PUSHSTRUCT psTemp = this->TrailingState;
    psTemp.ClearStyle();

    // Add any font or color codes after the push
    WideString wState = WideString(CTRL_POP) +
                                  utils->PrintStateString(psTemp, true);

    // Add style flags before or after pop as dictated by state at the
    // selection-start and the actual style control chars that immediately
    // follow the last selection char...
    //
    // TrailingState is the state at the end of the selection buffer,
    // including trailing codes following the last selected char.
    //
    // If a style is on at the end of the selection buffer and we have that
    // style char in our trailing codes then we need to write the style char
    // after the pop; otherwise, before it.
    int iStart, iEnd;
    // This gets the location and state of just the trailing codes
    PUSHSTRUCT ps = utils->GetTrailingState(S, iStart, iEnd);

    if (ps.bItalics)
    {
      if (TrailingState.bItalics)
        wState += CTRL_R;
      else
        wState = utils->InsertW(wState, CTRL_R, 1);
    }
    if (ps.bUnderline)
    {
      if (TrailingState.bUnderline)
        wState += CTRL_U;
      else
        wState = utils->InsertW(wState, CTRL_U, 1);
    }
    if (ps.bBold)
    {
      if (TrailingState.bBold)
        wState += CTRL_B;
      else
        wState = utils->InsertW(wState, CTRL_B, 1);
    }

    // Replace the old trailing state with the new one we've generated
    S = utils->DeleteW(S, iStart+1, iEnd-iStart+1); // delete trailing codes
    S = utils->InsertW(S, wState, iStart+1);

    if (slCount > 1)
      S = utils->InsertW(S, CTRL_PUSH, 1); // prepend push if more than one line

    // write the string back
    sl->SetString(S, slCount-1);
  }

  S = sl->Text;

  return S;
}
//---------------------------------------------------------------------------
// S has our selected text that we want to apply a style-change to, such
// as bold, underline or italics. The string also has leading and trailing
// codes around the selection. We can use InitialState to see the state
// at the first char of the selection relative to the entire document. And
// TrailingState is the state at the first char after the selection.
WideString __fastcall TProcessEffect::ApplySingleCode(WideString S, wchar_t c)
{
  if (S.IsEmpty())
    return "";

  TStringsW* sl = new TStringsW();

  sl->Text = S;

  int ii = 0;
  while (ProcessStyle(sl, ii++, c));

  S = sl->Text;

  delete sl;

  return S;
}
//---------------------------------------------------------------------------
// Strategy: We will need the InitialState and TrailingState global
// PUSHSTRUCTs. If the first char of our selection had the style (in c) ON,
// turn it off for the entire selection. If the style was OFF, turn it on
// for the whole selection.
//
// To do this... we need to get the "local" states in both the leading codes
// and trailing codes then strip the code from the entire selection.
//
// For the first line of text (in a string-list), toggle the style-flag (if a
// code was there, remove it, if not, add it). If more than one line is
// selected, terminate the first line with a style code if the style flag in
// InitialState is set (we are turning the style ON in the entire selection).
// Strip off the old leading codes and write the new state codes.
//
// For the last line of text, if there is more than one line of text, prepend
// a style char to the front of the string if InitialState is set. All strings
// should have been stripped of the char first. At the end of the last line
// toggle the original (local) trailing state flag and re-write the state
// flags.
//
// For > 2 lines, add a code to the beginning and end of each of those strings
// if the style was off in InitialState (we are turning the style ON for the
// entire selection). Otherwise, just strip them.
bool __fastcall TProcessEffect::ProcessStyle(TStringsW* sl, int ii, wchar_t c)
{
  int slCount = sl->Count;

  if (!slCount || ii < 0 || ii >= slCount)
    return false;

  WideString s;

  // Get string
  s = sl->GetString(ii);
  int realLen = utils->GetRealLength(s);

  if (realLen == 0)
    return true; // no real chars!

  // Get the state and length of leading-codes only
  int iLeadingEnd;
  PUSHSTRUCT psLeading = utils->GetLeadingState(s, iLeadingEnd);

  // This gets the location and state of just the trailing codes
  int iTrailingStart, iTrailingEnd;
  PUSHSTRUCT psTrailing = utils->GetTrailingState(s, iTrailingStart,
                                                                iTrailingEnd);
  if (iLeadingEnd >= 0 && iTrailingEnd >= 0)
  {
    // set flags based on which style we are setting
    // (bold, underline or italics)
    bool l, t, ls, ts;
    if (c == CTRL_B)
    {
      l = psLeading.bBold;
      ls = LeadingState.bBold;
      ts = TrailingState.bBold;
    }
    else if (c == CTRL_U)
    {
      l = psLeading.bUnderline;
      ls = LeadingState.bUnderline;
      ts = TrailingState.bUnderline;
    }
    else if (c == CTRL_R)
    {
      l = psLeading.bItalics;
      ls = LeadingState.bItalics;
      ts = TrailingState.bItalics;
    }

    // do the logic... took some doing to figure all of this out :-)

    // leading...
    if (ii == 0)
      // leading state for the first line is just the inverse of the state
      // of the line's leading codes (not relative to the entire document)
      l = !l;
    else
      // leading state at each subsequent line is opposite that of our first
      // selection char (if first char was bold we are turning bold off in
      // the entire selection...)
      l = !ls;

    // trailing...
    if (slCount == 1 || (slCount > 0 && ii == slCount-1))
      // trailing state in the last line of the string-list is "not"
      // exclusive or of the document-relative leading and trailing states.
      t = !(ls ^ ts);
    else
      // trailing state at each subsequent line is opposite that of our first
      // selection char (if first char was bold we are turning bold off in
      // the entire selection...)
      t = !ls;

    // restore flags
    if (c == CTRL_B)
    {
      psLeading.bBold = l;
      psTrailing.bBold = t;
    }
    else if (c == CTRL_U)
    {
      psLeading.bUnderline = l;
      psTrailing.bUnderline = t;
    }
    else if (c == CTRL_R)
    {
      psLeading.bItalics = l;
      psTrailing.bItalics = t;
    }

    // delete old trailing state first (or the index will no longer be right!)
    s = utils->DeleteW(s, iTrailingStart+1, iTrailingEnd-iTrailingStart);

    // delete old leading state
    s = utils->DeleteW(s, 1, iLeadingEnd);

    // strip code to clear out possible conflicts
    s = utils->StripChar(s, c);

    // Replace the old leading state with the new one we've generated
    s = utils->InsertW(s, utils->PrintStateString(psLeading, true), 1);

    // Replace the old trailing state with the new one we've generated
    s += utils->PrintStateString(psTrailing, false);

    // write the string back
    sl->SetString(s, ii);
  }

  return true;
}
//---------------------------------------------------------------------------
int __fastcall TProcessEffect::Rfg(void)
{
  return utils->ConvertColor(IRCRANDOM, bFullSpectFg);
}
//---------------------------------------------------------------------------
int __fastcall TProcessEffect::Rbg(void)
{
  return utils->ConvertColor(IRCRANDOM, bFullSpectBg);
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::WriteColorsReplace(WideString &S,
                                                            int fg, int bg)
{
  int newFg = fg;
  int newBg = bg;

  if (dts->bFgChecked && dts->bBgChecked)
  {
    if (dts->FgFromColor == utils->YcToRgb(fg))
      newFg = utils->RgbToYc(dts->FgToColor);
    if (dts->BgFromColor == utils->YcToRgb(bg))
      newBg = utils->RgbToYc(dts->BgToColor);
  }
  else if (dts->bFgChecked)
  {
    if (dts->FgFromColor == utils->YcToRgb(fg))
      newFg = utils->RgbToYc(dts->FgToColor);
  }
  else if (dts->bBgChecked)
  {
    if (dts->BgFromColor == utils->YcToRgb(bg))
      newBg = utils->RgbToYc(dts->BgToColor);
  }

  // Add the color-sequence to the current line
  utils->WriteColors(newFg, newBg, S);

  // Need this so if we process a Pop, it writes our new
  // color after the pop!
  if (PrevFG != newFg)
    PrevFG = newFg;
  if (PrevBG != newBg)
    PrevBG = newBg;
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::WriteColorsAlt(WideString &S, int Mode,
                                                          int &fg, int &bg)
{
  // Mode is Fg, Bg or Both!
  if (Mode == 0)
    utils->WriteSingle(fg, S, true); // Fg
  else if (Mode == 1)
    utils->WriteSingle(bg, S, false); // Bg
  else
    utils->WriteColors(fg, bg, S); // Both

  // Swap colors
  int temp = fg;
  fg = bg;
  bg = temp;
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::WriteColorsInc(WideString &S, int Mode)
// ep1 and ep2 are fg/bg 1-based color indices into a palette
// so subtract 1 before the lookup!
{
  int tFg, tBg;

  if (dts->RGB_Palette) // Only non-NULL if dts->IsYahTrinPal()!
  {
    if (++ep1 > dts->RGB_PaletteSize)
      ep1 = 1;

    if (++ep2 > dts->RGB_PaletteSize)
      ep2 = 1;

    tFg = -dts->RGB_Palette[ep1-1];
    tBg = -dts->RGB_Palette[ep2-1];
  }
  else
  {
    if (++ep1 > dts->PaletteSize)
      ep1 = 1;

    if (++ep2 > dts->PaletteSize)
      ep2 = 1;

    // Try to avoid visual conflicts
    int tries = 3*dts->PaletteSize;

    if (Mode == 0)
    {
      while (--tries && IsConflict(ep1, NO_COLOR))
        ep1 = random(dts->PaletteSize)+1;
    }
    else if (Mode == 1)
    {
      while (--tries && IsConflict(ep2, NO_COLOR))
        ep2 = random(dts->PaletteSize)+1;
    }
    else
    {
      while (--tries && IsConflict(ep1, ep2))
      {
        ep1 = random(dts->PaletteSize)+1;
        ep2 = random(dts->PaletteSize)+1;
      }
    }

    tFg = ep1;
    tBg = ep2;
  }

  // Mode is Fg, Bg or Both!
  if (Mode == 0)
    utils->WriteSingle(tFg, S, true); // Fg
  else if (Mode == 1)
    utils->WriteSingle(tBg, S, false); // Bg
  else
    utils->WriteColors(tFg, tBg, S); // Both

  PrevFG = tFg;
  PrevBG = tBg;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IsConflict(int fg, int bg)
// Returns true if color-clash
{
  bool bFgNoColor = (fg == NO_COLOR || fg == IRCNOCOLOR);
  bool bBgNoColor = (bg == NO_COLOR || bg == IRCNOCOLOR);

  if (bFgNoColor && bBgNoColor)
    return false;

  if (!bFgNoColor && bBgNoColor)
  {
    if (fg == PrevFG || fg == LeadingState.fg || fg == LeadingState.bg ||
                        IrcColorClash(fg, LeadingState.bg))
      return true;
  }
  else if (bFgNoColor && !bBgNoColor)
  {
    if (bg == PrevBG || bg == LeadingState.fg || bg == LeadingState.bg ||
                                  IrcColorClash(bg, LeadingState.fg))
      return true;
  }
  else
  {
    if (PrevFG == fg || PrevBG == bg || fg == LeadingState.fg ||
                bg == LeadingState.bg || bg == fg || IrcColorClash(bg, fg))
      return true;
  }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IrcColorClash(int c1, int c2)
// Returns true if Yellow/White color-clash
// or IRCTEAL/IRCGREEN MAROON/PURPLE clash)
{
  if (c1 == NO_COLOR || c2 == NO_COLOR)
    return false;

  if ((c1 == IRCWHITE && c2 == IRCYELLOW) || (c2 == IRCWHITE && c1 == IRCYELLOW))
    return true;

  if ((c1 == IRCTEAL && c2 == IRCGREEN) || (c2 == IRCTEAL && c1 == IRCGREEN))
    return true;

  if ((c1 == IRCMAROON && c2 == IRCPURPLE) || (c2 == IRCMAROON && c1 == IRCPURPLE))
    return true;

  if ((c1 == IRCMAROON && c2 == IRCBLACK) || (c2 == IRCMAROON && c1 == IRCBLACK))
    return true;

  return false;
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::WriteColorsRand(WideString &S, int Mode, int fg, int bg)
{
  int nfg, nbg;
  int tries = 128; // timeout counter for resolving conflicts

  if (Mode == 0)
  {
    nfg = Rfg();

    while (--tries && (nfg == fg || nfg == bg || nfg == PrevFG))
      nfg = Rfg();

    utils->WriteSingle(nfg, S, true);

    PrevFG = nfg;
  }
  else if (Mode == 1)
  {
    nbg = Rbg();

    while (--tries && (nbg == fg || nbg == bg || nbg == PrevBG))
      nbg = Rbg();

    utils->WriteSingle(nbg, S, false);

    PrevBG = nbg;
  }
  else
  {
    nfg = Rfg();
    nbg = Rbg();

    while (--tries && (nfg == PrevFG || nfg == bg || nbg == PrevBG || nbg == fg || nbg == bg || nbg == nfg))
    {
      nfg = Rfg();
      nbg = Rbg();
    }

    utils->WriteColors(nfg, nbg, S);

    PrevFG = nfg;
    PrevBG = nbg;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::LimitBC(BlendColor &bc)
{
  if (bc.red > 255) bc.red = 255;
  if (bc.green > 255) bc.green = 255;
  if (bc.blue > 255) bc.blue = 255;

  if (bc.red < 0) bc.red = 0;
  if (bc.green < 0) bc.green = 0;
  if (bc.blue < 0) bc.blue = 0;
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::AddInitialCodes(int mode, PUSHSTRUCT &State, WideString &TempStr)
{
  if (State.fg == NO_COLOR)
  {
    State.fg = utils->ConvertColor(dts->Foreground, bFullSpectFg);

    if (IsFgColorEffect(mode))
      utils->WriteSingle(State.fg, TempStr, true);
  }

  if (State.bg == NO_COLOR)
  {
    State.bg = utils->ConvertColor(dts->Background, bFullSpectBg);

    if (IsBgColorEffect(mode))
      utils->WriteSingle(State.bg, TempStr, false);
  }

  // Write initial font type and size if this client allows it
  if (dts->IsYahTrinPal())
  {
    if (State.fontType == -1)
    {
      State.fontType = dts->cType;

      if (IsFontTypeEffect())
        TempStr += utils->FontTypeToString(dts->cType);
    }

    if (State.fontSize == -1)
    {
      State.fontSize = dts->cSize;

      if (IsFontSizeEffect())
        TempStr += utils->FontSizeToString(dts->cSize);
    }
  }
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IsFgColorEffect(int mode)
{
  bool bFgMode;

  if (mode == -1)
    bFgMode = true;
  else
    bFgMode = (mode == EM_FG || mode == EM_FGBG) ? true : false;

  return (effect == E_INC_COLORS && bFgMode) || (effect == E_ALT_COLORS && bFgMode) ||
    (effect == E_RAND_COLORS && bFgMode) || (effect == E_MORPH_FGBG && bFgMode) ||
    (effect == E_REPLACE_COLOR && bFgMode) ||
        (effect == E_ALT_CHAR && ep1 != NO_COLOR) || effect == E_FG_BLEND;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IsBgColorEffect(int mode)
{
  bool bBgMode;

  if (mode == -1)
    bBgMode = true;
  else
    bBgMode = (mode == EM_BG || mode == EM_FGBG) ? true : false;

  return (effect == E_INC_COLORS && bBgMode) || (effect == E_ALT_COLORS && bBgMode) ||
    (effect == E_RAND_COLORS && bBgMode) || (effect == E_MORPH_FGBG && bBgMode) ||
    (effect == E_REPLACE_COLOR && bBgMode) || effect == E_BG_BLEND;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IsFontTypeEffect(void)
{
  return effect == E_FONT_TYPE;
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::IsFontSizeEffect(void)
{
  return effect == E_FONT_SIZE || (effect >= E_FONT_MOUNTAIN && effect <= E_FONT_RANDOM);
}
//---------------------------------------------------------------------------
bool __fastcall TProcessEffect::CheckForOverlappingPushPop(wchar_t* pBuf, int iSize)
{
  int PushPop = 0;

  if (iSize)
  {
    for (int ii = 0; ii < iSize; ii++)
    {
      if (pBuf[ii] == CTRL_PUSH)
        PushPop++;
      else if (pBuf[ii] == CTRL_POP)
        PushPop--;
    }
  }

  if (PushPop != 0)
    return true;

  return false; // Add push at beginning and pop at end
}
//---------------------------------------------------------------------------
void __fastcall TProcessEffect::DeleteSpaceBgColors(TList* l)
{
  if (l == NULL) return;

  for (int ii = 0; ii < l->Count; ii++)
    try {delete l->Items[ii];} catch(...) {}

  try {delete l;} catch(...) {};
}
//---------------------------------------------------------------------------
TList* __fastcall TProcessEffect::RecordSpaceBgColors(wchar_t* pBuf, int iSize)
// This creates a list with the background color of every space char
// in pBuf. So the list's size will be the # of leading spaces...
//
// Don't forget to delete the returned list!
{
  try
  {
    TList* l = new TList();

    int iRet, len, fg, bg;
    int PresentBg = NO_COLOR;
    int PreviousBg = NO_COLOR;
    bool bFirstSpace = false;

    for(int ii = 0; ii < iSize; ii++)
    {
      if (pBuf[ii] == CTRL_K)
      {
        fg = NO_COLOR;
        bg = NO_COLOR;

        len = utils->CountColorSequence(pBuf, ii, iSize, fg, bg);

        // Background color?
        if (bg != NO_COLOR)
          PresentBg = bg;

        ii += len;

        continue;
      }

      iRet = utils->SkipCode(pBuf, ii, iSize);

      if (iRet == S_NULL)
        break;

      if (iRet == S_CRLF)
      {
        PresentBg = NO_COLOR;
        bFirstSpace = false;
        continue;
      }

      if (iRet)
        continue;

      // Add a space-sequence's color and index to our list
      if (pBuf[ii] == ' ')
      {
        if (!bFirstSpace)
        {
          if (PresentBg != NO_COLOR)
          {
            l->Add(new SPACEITEM(ii, PresentBg));
            PreviousBg = PresentBg;
          }
          else if (LeadingState.bg != NO_COLOR)
          {
            l->Add(new SPACEITEM(ii, LeadingState.bg));
            PreviousBg = LeadingState.bg;
          }
          else
          {
            l->Add(new SPACEITEM(ii, dts->Abg));
            PreviousBg = dts->Abg;
          }

          bFirstSpace = true;
        }
        else // subsequent consecutive spaces...
        {
          if (PresentBg != NO_COLOR && PresentBg != PreviousBg)
          {
            l->Add(new SPACEITEM(ii, PresentBg));
            PreviousBg = PresentBg;
          }
        }
      }
      else
        bFirstSpace = false;
    }

    return l;
  }
  catch(...) {return NULL;}
}
//---------------------------------------------------------------------------

